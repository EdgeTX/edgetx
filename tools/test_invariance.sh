#!/bin/bash
# Preuve d'invariance du layout "code en bank 1" (TX16S, EdgeTX#5899) :
# la garantie doit venir du linker script, pas d'un placement chanceux.
#
# Deux builds volontairement perturbes :
#   1. +8K de rodata et une fonction dans un fichier CORE (decale tout
#      le contenu de la bank 1)
#   2. +8K de rodata et une fonction dans un fichier UI, plus
#      inversion de l'ordre de link de deux sources UI (decale la
#      bank 2 et change la survie des comdats)
#
# Chaque build doit passer tools/check_bank1.sh. Les perturbations ne
# sont jamais commitees (restauration git a la fin, arbre propre exige).
#
# Usage: bench/test_invariance.sh

set -e
cd "$(dirname "$0")/.."

if ! git diff --quiet radio/src; then
  echo "ERREUR: modifications non commitees dans radio/src, abandon"; exit 1
fi

cleanup() {
  git checkout -- radio/src/gps.cpp radio/src/gui/colorlcd/mainview/view_main.cpp \
                  radio/src/gui/colorlcd/CMakeLists.txt 2>/dev/null || true
}
trap cleanup EXIT

build_and_check() {
  local label=$1
  echo "=== build perturbe: $label ==="
  docker run --rm -u $(id -u):$(id -g) -v "$(pwd)":/src -w /src \
    ghcr.io/edgetx/edgetx-dev:latest bash -c "
set -e
rm -rf build-invar && mkdir build-invar && cd build-invar
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain/arm-none-eabi.cmake \
      -DPCB=X10 -DPCBREV=TX16S -DCMAKE_BUILD_TYPE=Release .. > cmake.log 2>&1
make firmware -j\$(nproc) > make.log 2>&1 || { tail -20 make.log; exit 1; }
"
  tools/check_bank1.sh build-invar/arm-none-eabi/firmware.elf
  echo "=== $label: OK ==="
}

# --- perturbation 1: core ---
cat >> radio/src/gps.cpp <<'PAD'

// (test d'invariance bank 1 -- jamais commite)
const volatile char _invar_pad_core[8192] = {1};
void _invar_dummy_core() { (void)_invar_pad_core[0]; }
PAD
build_and_check "padding core (+8K bank 1)"
cleanup

# --- perturbation 2: UI + ordre de link ---
cat >> radio/src/gui/colorlcd/mainview/view_main.cpp <<'PAD'

// (test d'invariance bank 1 -- jamais commite)
const volatile char _invar_pad_ui[8192] = {2};
void _invar_dummy_ui() { (void)_invar_pad_ui[0]; }
PAD
python3 - <<'PYEOF'
import re
p = 'radio/src/gui/colorlcd/CMakeLists.txt'
lines = open(p).readlines()
# inverse les deux premieres sources .cpp adjacentes trouvees
for i in range(len(lines) - 1):
    if lines[i].strip().endswith('.cpp') and lines[i+1].strip().endswith('.cpp'):
        lines[i], lines[i+1] = lines[i+1], lines[i]
        print(f"swap: {lines[i].strip()} <-> {lines[i+1].strip()}")
        break
open(p, 'w').writelines(lines)
PYEOF
build_and_check "padding UI (+8K bank 2) + ordre de link inverse"

echo
echo "== invariance: les 2 builds perturbes passent la verification =="
