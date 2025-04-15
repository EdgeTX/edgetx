
# Trims:
#  TRIMS_GPIO_REG_[trim]
#  TRIMS_GPIO_PIN_[trim]
#
# where 'trim' is one of:
#
#   (-)    (+)
#  'LHL', 'LHR',
#  'LVD', 'LVU',
#  'RVD', 'RVU',
#  'RHL', 'RHR',
#  'LSD', 'LSU', T4
#  'RSD', 'RSU', T5

TRIMS = [
    # Air
    { "name": "T1", "dec": "LHL", "inc": "LHR", "input": "LH" },
    { "name": "T2", "dec": "LVD", "inc": "LVU", "input": "LV" },
    { "name": "T3", "dec": "RVD", "inc": "RVU", "input": "RV" },
    { "name": "T4", "dec": "RHL", "inc": "RHR", "input": "RH" },
    { "name": "T5", "dec": "LSD", "inc": "LSU" },
    { "name": "T6", "dec": "RSD", "inc": "RSU" },
    { "name": "T7", "dec": "T7L", "inc": "T7R" },
    { "name": "T8", "dec": "T8D", "inc": "T8U" },
    # Surface
    { "name": "T1", "dec": "T1L", "inc": "T1R", "input": "ST" },
    { "name": "T2", "dec": "T2L", "inc": "T2R", "input": "TH" },
    { "name": "T3", "dec": "T3L", "inc": "T3R"},
    { "name": "T4", "dec": "T4L", "inc": "T4R"},
    { "name": "T5", "dec": "T5L", "inc": "T5R" },
    { "name": "T6", "dec": "T6D", "inc": "T6U" },
]

KEYS = [
    { "name": "MENU", "key": "KEY_MENU", "label": "MENU" },
    { "name": "EXIT", "key": "KEY_EXIT", "label": "EXIT" },
    { "name": "ENTER", "key": "KEY_ENTER", "label": "Enter" },

    { "name": "PAGEUP", "key": "KEY_PAGEUP", "label": "PAGE<" },
    { "name": "PAGEDN", "key": "KEY_PAGEDN", "label": "PAGE>" },

    { "name": "UP", "key": "KEY_UP", "label": "Up" },
    { "name": "DOWN", "key": "KEY_DOWN", "label": "Down" },

    { "name": "LEFT", "key": "KEY_LEFT", "label": "Left" },
    { "name": "RIGHT", "key": "KEY_RIGHT", "label": "Right" },

    { "name": "PLUS", "key": "KEY_PLUS", "label": "(+)" },
    { "name": "MINUS", "key": "KEY_MINUS", "label": "(-)" },

    { "name": "MDL", "key": "KEY_MODEL", "label": "MDL" },
    { "name": "TELE", "key": "KEY_TELE", "label": "TELE" },
    { "name": "SYS", "key": "KEY_SYS", "label": "SYS" },

    { "name": "SHIFT", "key": "KEY_SHIFT", "label": "Shift" }
]

KEY_LABELS = [
    {
        "targets": {"boxer", "f16", "mt12", "gx12", "pocket", "tx12", "tx12mk2", "tx16s", "v16", "zorro"},
        "keys": {
            "EXIT": { "label": "RTN" }
        }
    },
    {
        "targets": {"bumblebee", "t12max", "t14", "t15", "t20", "t20v2", "tpro", "tpros", "tprov2"},
        "keys": {
            "PAGEDN": { "label": "</>" },
        }
    },
    {
        "targets": {"x7", "x7access", "x9e"},
        "keys": {
            "PAGEDN": { "label": "PAGE" },
        }
    },
    {
        "targets": {"t16", "t18"},
        "keys": {
            "EXIT": { "label": "RTN" },
            "PAGEDN": {"label": "PAGE" }
        }
    },
    {
        "targets": {"x9d", "x9d+", "x9d+2019"},
        "keys": {
            "ENTER": { "label": "ENT" },
            "PAGEDN": { "label": "PAGE" }
        }
    },
    {
        "targets": {"x10", "x10express"},
        "keys": {
            "EXIT": { "label": "RTN" },
            "PAGEDN": { "label": "PgUp/Dn" }
        }
    },
    {
        "targets": {"x12s"},
        "keys": {
            "EXIT": { "label": "RTN" },
            "PAGEUP": { "label": "PgUp" },
            "PAGEDN": { "label": "PgDn" }
        }
    },
    {
        "targets": {"t8"},
        "keys": {
            "EXIT": { "label": "RTN" },
            "ENTER": { "label": "ENT" }
        }
    },
    {
        "targets": {"tlite"},
        "keys": {
            "EXIT": { "label": "RTN" },
            "ENTER": { "label": "ENT" },
            "LEFT": { "label": "Left/SYS" },
            "RIGHT": { "label": "Right/MDL" }
        }
    },
        {
        "targets": {"commando8"},
        "keys": {
            "EXIT": { "label": "BCK" },
            "PAGEUP": { "label": "Left" },
            "PAGEDN": { "label": "Right" },
            "PLUS": { "label": "Up" },
            "MINUS": { "label": "Down" },
            "MENU": { "label": "Menu" }
        }
    },
    {
        "targets": {"st16"},
        "keys": {
            "PAGEDN": { "label": "MENU" }
        }
    },
]

class Key:

    def __init__(self, gpio, pin):
        self.gpio = gpio
        self.pin = pin
        self.active_low = True

class Trim:

    def __init__(self, name, dec, inc):
        self.name = name
        if dec and dec.gpio and inc and inc.gpio:
            self.dec = dec
            self.inc = inc
            self.active_low = True

def get_trim_switch(hw_defs, tag):

    gpio = f'TRIMS_GPIO_REG_{tag}'
    pin  = f'TRIMS_GPIO_PIN_{tag}'

    if (gpio in hw_defs) and (pin in hw_defs):
        key = Key(hw_defs[gpio], hw_defs[pin])
        if 'TRIMS_GPIO_ACTIVE_HIGH' in hw_defs:
            key.active_low = False
        return key

    return None


def parse_trims(hw_defs):

    trims = []
    for t in TRIMS:

        name = t['name']
        dec = get_trim_switch(hw_defs, t['dec'])
        inc = get_trim_switch(hw_defs, t['inc'])

        if dec and inc:
            trims.append(Trim(name, dec, inc))

        elif t.get('input'):
            input = t['input']
            if f'TRIMS_VIRTUAL_{input}' in hw_defs:
                trims.append(Trim(name, None, None))

    return trims


def key_label(target, name):
    for d in KEY_LABELS:
        if target in d['targets']:
            keys = d.get('keys')
            key = keys.get(name)
            if key:
                return key['label']

    return None


def parse_keys(target, hw_defs):

    keys = []

    for k in KEYS:

        name = k['name']
        gpio = f'KEYS_GPIO_REG_{name}'
        pin  = f'KEYS_GPIO_PIN_{name}'

        if (gpio in hw_defs) and (pin in hw_defs):
            key = Key(hw_defs[gpio], hw_defs[pin])
            key.key = k['key']
            key.name = name
            label = key_label(target, name)
            if label:
                key.label = label
            else:
                key.label = k['label']
            if 'KEYS_GPIO_ACTIVE_HIGH' in hw_defs:
                key.active_low = False
            keys.append(key)

    return keys
