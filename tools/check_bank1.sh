#!/bin/bash
# Verification post-link du layout "code en bank 1" (TX16S, EdgeTX#5899,
# errata ST ES0166 §2.4.6 : bruit PA12/USB-D+ -> lectures corrompues en
# flash bank 2 pendant une session USB).
#
# Garanties verifiees :
#  1. aucune section executable au-dessus de 0x08100000, sauf .uicode
#     (UI gelee pendant l'USB) ; le depassement de la bank 1 est deja
#     une erreur de link (region FLASH = 1024K)
#  2. le code chaud USB est bien en RAM (.fastcode)
#  3. aucun chemin d'appel DIRECT depuis un contexte actif pendant
#     l'USB (ISR, mixer, audio, timers telemetrie/logs, tete de
#     perMain) vers la bank 2 — hors aretes gardees a l'execution
#     (liste blanche ci-dessous)
#
# Limites connues : les appels indirects (pointeurs de fonction) ne
# sont pas suivis ; la couverture est completee a l'execution par la
# barriere MPU du build de validation (DIAG_BANK2_FENCE).
#
# Usage: tools/check_bank1.sh [chemin/firmware.elf]

set -e
ELF=${1:-"$(dirname "$0")/../build-bank1/arm-none-eabi/firmware.elf"}
[ -f "$ELF" ] || { echo "ELF introuvable: $ELF"; exit 1; }

echo "== 1. sections executables =="
BAD_SECTIONS=$(arm-none-eabi-objdump -h "$ELF" | awk '
  /CODE/ { print prev }
  { if ($2 ~ /^\./) prev = $2" "$4 }' | while read name vma; do
    case "$name" in .uicode) continue ;; esac
    v=$((16#$vma))
    if [ $v -ge $((0x08100000)) ] && [ $v -lt $((0x08200000)) ]; then
      echo "$name@0x$vma"
    fi
  done)
if [ -n "$BAD_SECTIONS" ]; then
  echo "ERREUR: sections code en bank 2: $BAD_SECTIONS"; exit 1
fi
echo "OK (seule .uicode est en bank 2)"

echo "== 2. code USB en RAM =="
ADDR=$(arm-none-eabi-nm "$ELF" | awk '/ T OTG_FS_IRQHandler/ {print $1}')
case "$ADDR" in
  2000*) echo "OK: OTG_FS_IRQHandler @ 0x$ADDR" ;;
  *) echo "ERREUR: OTG_FS_IRQHandler @ 0x$ADDR (attendu 0x2000xxxx)"; exit 1 ;;
esac

echo "== 3. joignabilite bank 2 depuis les contextes actifs pendant l'USB =="
DISAS=$(mktemp)
trap 'rm -f "$DISAS"' EXIT
arm-none-eabi-objdump -d "$ELF" > "$DISAS"
python3 - "$DISAS" <<'PYEOF'
import sys, re, collections
sys.stdin = open(sys.argv[1])

BANK2 = range(0x08100000, 0x08200000)

# Racines : tout ce qui peut s'executer pendant une session USB active.
# - toutes les ISR (suffixe _IRQHandler / Handler du vecteur)
# - taches et timers qui ne sont pas geles par le gel UI
# - la tete de perMain (avant le retour anticipe du gel)
ROOT_PATTERNS = [
    r'.*_IRQHandler$', r'^HardFault_Handler$', r'^SVC_Handler$',
    r'^PendSV_Handler$', r'^SysTick_Handler$',
    r'.*mixerTask.*', r'.*audioTask.*', r'^telemetryTimerCb$',
    r'.*logsTimerCb.*', r'^_Z9logsWritev$', r'^_Z15telemetryWakeupv$',
    r'^_Z13evalFunctions.*',
    # menusTask tourne pendant le gel (seul perMain est tronque) et
    # appelle pwrCheck a chaque cycle de 50 ms
    r'.*menusTask.*', r'^pwrCheck$',
    # timers FreeRTOS (indirects via la tache timer, non vus par le
    # graphe d'appels directs) : chaque callback est une racine
    r'.*_timer_10ms_cb.*', r'.*per10msv?$', r'.*loggingTimerCb.*',
    r'.*spacemouseTimerCb.*', r'.*MultiRfProtocols7timerCb.*',
    r'^_ZL11_refresh_cb.*',  # rgb_leds uniquement
    # tete de perMain : ce qui s'execute AVANT le retour anticipe du
    # gel USB (perMain lui-meme est tronque a l'execution)
    r'^_Z17checkSpeakerVolumev$', r'^_Z16initLoggingTimerv$',
    r'^_Z20checkTrainerSettingsv$', r'^_Z12periodicTickv$',
    r'^_Z14checkBacklightv$', r'^_Z11flightResetv$',
]

# Fonctions gardees a l'execution : ne s'executent jamais pendant une
# session USB active (etat USB teste en entree, ou garde
# _usb_ui_frozen) -> sous-arbre entier elague
PRUNED = {
    '_Z19handleUsbConnectionv',   # demarre/arrete l'USB : UI appelee
                                  # uniquement avant usbStart/apres usbStop
    '_Z11closeUsbMenuv',          # uniquement sur debranchement
    '_Z13POPUP_WARNINGPKcS0_',    # garde _usb_ui_frozen()
    '_Z17POPUP_INFORMATIONPKc',   # garde _usb_ui_frozen()
    '_Z18checkStorageUpdatev',    # garde !usbPlugged dans perMain
    '_Z17usbSessionCleanupv',     # commence par usbStop() : la bank 2
    '_Z19usbSessionForceStopv',   # redevient sure (PA12 au repos)
    '_Z10edgeTxInitv',            # boot uniquement, avant toute session USB
    '_Z7perMainv',                # gel : retour anticipe avant toute UI ;
                                  # sa tete est auditee par racines dediees
    '_Z11edgeTxCloseh',           # extinction (toujours apres
                                  # usbSessionForceStop) ou pre-usbStart (MSC)
}

# Aretes tolerees individuellement : gardees a l'execution, mais dont
# l'appelant doit rester audite pour le reste de son sous-arbre
WHITELIST_EDGES = {
    # pwrCheck coupe la session USB (usbSessionForceStop) avant tout
    # flux d'extinction, et saute l'animation pendant le gel
    ('pwrCheck', '_Z21drawShutdownAnimationmmPKc'),
    ('pwrCheck', '_Z18confirmationDialogPKcS0_bRKSt8functionIFbvEE'),
    # sortie de menusTask = power off, toujours apres usbSessionForceStop
    ('_ZL9menusTaskv', '_Z15drawSleepBitmapv'),
    # initialisation LVGL au demarrage de la tache, avant tout USB
    ('_ZL9menusTaskv', '_ZN11LvglWrapper8instanceEv'),
    # ecran d'urgence au boot (UNEXPECTED_SHUTDOWN), avant tout USB
    ('_ZL9menusTaskv', '_Z20drawFatalErrorScreenPKc'),
}

funcs = {}      # nom -> addr
calls = collections.defaultdict(set)  # caller -> {(callee, addr)}
cur = None
for line in sys.stdin:
    m = re.match(r'^([0-9a-f]+) <(.+)>:', line)
    if m:
        cur = m.group(2); funcs[cur] = int(m.group(1), 16); continue
    if cur is None: continue
    m = re.search(r'\t(?:bl|b\.w|b\.n|b|beq\.w|bne\.w|bcs\.w|bcc\.w|bmi\.w|bpl\.w|bvs\.w|bvc\.w|bhi\.w|bls\.w|bge\.w|blt\.w|bgt\.w|ble\.w|cbz|cbnz)\s+([0-9a-f]+) <([^>+]+)(?:\+0x[0-9a-f]+)?>', line)
    if m:
        calls[cur].add((m.group(2), int(m.group(1), 16)))

# une racine doit vivre en bank 1 : un symbole apparie en bank 2 est
# du code UI (gele pendant l'USB), pas un contexte chaud
roots = [f for f in funcs
         if any(re.match(p, f) for p in ROOT_PATTERNS)
         and funcs[f] not in BANK2]

# BFS sur les appels directs, en restant en bank 1 ; une arete vers la
# bank 2 depuis un appelant non liste = violation
violations = []
seen = set(roots)
parent = {r: None for r in roots}
queue = list(roots)
while queue:
    f = queue.pop()
    if f in PRUNED: continue
    for callee, addr in calls.get(f, ()):
        if addr in BANK2:
            if (f, callee) in WHITELIST_EDGES: continue
            chain = []
            n = f
            while n is not None:
                chain.append(n); n = parent.get(n)
            violations.append((' -> '.join(reversed(chain)), callee))
            continue
        if callee not in seen and callee in funcs:
            seen.add(callee); parent[callee] = f
            queue.append(callee)

if violations:
    print(f"ERREUR: {len(violations)} chemin(s) direct(s) vers la bank 2 :")
    for chain, callee in sorted(set(violations))[:40]:
        print(f"  {chain}  ==>  {callee}")
    sys.exit(1)
print(f"OK ({len(roots)} racines, {len(seen)} fonctions parcourues, "
      f"aucun chemin direct vers la bank 2)")
PYEOF

echo "== verification bank 1 : OK =="
