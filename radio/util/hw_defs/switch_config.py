"""Default switches configurations and display positions."""

SWITCH_CONFIG = {
    "boxer": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SG": {"default": "2POS",   "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]},
        "SF": {"default": "TOGGLE", "display": [1, 2]},
        "SH": {"default": "2POS", "display": [1, 3]},
    },
    "gx12": {
        # left side
        "SA": {"default": "TOGGLE", "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SG": {"default": "TOGGLE", "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 1]},
        "SD": {"default": "TOGGLE", "display": [1, 0]},
        "SF": {"default": "3POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]},
    },
    "commando8": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "el18": {
        "SA": { "default": "2POS"   },
        "SB": { "default": "3POS"   },
        "SC": { "default": "3POS"   },
        "SD": { "default": "2POS"   },
        "SE": { "default": "2POS"   },
        "SF": { "default": "3POS"   },
        "SG": { "default": "3POS"   },
        "SH": { "default": "TOGGLE" }
    },
    "pl18": {
        "SA": { "default": "2POS"   },
        "SB": { "default": "3POS"   },
        "SC": { "default": "2POS"   },
        "SD": { "default": "3POS"   },
        "SE": { "default": "3POS"   },
        "SF": { "default": "2POS"   },
        "SG": { "default": "3POS"   },
        "SH": { "default": "TOGGLE" }
    },
    "pl18ev": {
        "SA": { "default": "2POS"   },
        "SB": { "default": "3POS"   },
        "SC": { "default": "2POS"   },
        "SD": { "default": "3POS"   },
        "SE": { "default": "3POS"   },
        "SF": { "default": "2POS"   },
        "SG": { "default": "3POS"   },
        "SH": { "default": "3POS"   },
        "SI": { "default": "3POS"   },
        "SJ": { "default": "3POS"   }
    },
    "lr3pro": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "nv14": {
        "SA": {"default": "2POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "TOGGLE"},
        "SD": {"default": "2POS"},
        "SE": {"default": "TOGGLE"},
        "SF": {"default": "3POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "v12": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "TOGGLE", "display": [1, 2]},
    },
    "v14": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "TOGGLE", "display": [1, 2]},
    },
    "v16": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "mt12": {
        # left side
        "SA": { "default": "3POS",    "display": [ 0, 0 ] },
        "SB": { "default": "TOGGLE",  "display": [ 0, 1 ] },
        # right side
        "SC": { "default": "TOGGLE",  "display": [ 1, 0 ] },
        "SD": { "default": "TOGGLE",  "display": [ 1, 1 ] },
    },
    "t8": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "t12": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SG": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SH": {"default": "2POS",   "display": [1, 2]}
    },
    "t12max": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]}
    },
    "t14": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "TOGGLE", "display": [1, 2]}
    },
    "t15": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "2POS"},
        "SF": {"default": "2POS"},
    },
    "t16": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "t18": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "tlite": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "2POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "tlitef4": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "2POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "tpro": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "TOGGLE", "display": [0, 1]},
        "SE": {"default": "NONE",   "display": [0, 2]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "NONE",   "display": [1, 2]},
    },
    "tprov2": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "TOGGLE", "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        "SG": {"default": "NONE",   "display": [0, 3]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
        "SH": {"default": "NONE",   "display": [1, 3]},
    },
    "tpros": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "TOGGLE", "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
    },
    "bumblebee": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "TOGGLE", "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        "SG": {"default": "TOGGLE", "display": [0, 3]},
        # right side
        "SB": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]},
    },
    "t20": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        "SG": {"default": "TOGGLE", "display": [0, 3]},
        "SI": {"default": "TOGGLE", "display": [0, 4]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]},
        "SJ": {"default": "TOGGLE", "display": [1, 4]}
    },
    "t20v2": {
        # left side
        "SA": {"default": "2POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        "SG": {"default": "TOGGLE", "display": [0, 3]},
        "SI": {"default": "TOGGLE", "display": [0, 4]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]},
        "SJ": {"default": "TOGGLE", "display": [1, 4]}
    },
    "tx12": {
        # left side
        "SA": {"default": "TOGGLE", "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SG": {"default": "NONE",   "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "3POS",   "display": [1, 2]},
        "SH": {"default": "NONE",   "display": [1, 3]}
    },
    "tx16s": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "f16": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "tx12mk2": {
        # left side
        "SA": {"default": "TOGGLE", "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "3POS",   "display": [1, 2]},
    },
    "x7": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SF": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SH": {"default": "TOGGLE", "display": [1, 2]}
    },
    "x7access": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SF": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SH": {"default": "TOGGLE", "display": [1, 2]}
    },
    "x9d": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SF": {"default": "2POS",   "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SG": {"default": "3POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]}
    },
    "x9d+": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SF": {"default": "2POS",   "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SG": {"default": "3POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]}
    },
    "x9d+2019": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SF": {"default": "2POS",   "display": [0, 3]},
        "SI": {"default": "TOGGLE", "display": [0, 4]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SG": {"default": "3POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]}
    },
    "x9e": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "3POS",   "display": [0, 2]},
        "SF": {"default": "2POS",   "display": [0, 3]},
        # extra
        "SI": {"default": "NONE",   "display": [0, 4]},
        "SK": {"default": "NONE",   "display": [0, 5]},
        "SM": {"default": "NONE",   "display": [0, 6]},
        "SO": {"default": "NONE",   "display": [0, 7]},
        "SQ": {"default": "NONE",   "display": [0, 8]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "3POS",   "display": [1, 1]},
        "SH": {"default": "3POS",   "display": [1, 2]},
        "SG": {"default": "TOGGLE", "display": [1, 3]},
        # extra
        "SJ": {"default": "NONE",   "display": [1, 4]},
        "SL": {"default": "NONE",   "display": [1, 5]},
        "SN": {"default": "NONE",   "display": [1, 6]},
        "SP": {"default": "NONE",   "display": [1, 7]},
        "SR": {"default": "NONE",   "display": [1, 8]}
    },
    "xlite": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]}
    },
    "xlites": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SC": {"default": "3POS",   "display": [0, 1]},
        "SE": {"default": "TOGGLE", "display": [0, 2]},
        # right side
        "SB": {"default": "2POS",   "display": [1, 0]},
        "SD": {"default": "2POS",   "display": [1, 1]},
        "SF": {"default": "TOGGLE", "display": [1, 2]}
    },
    "x9lite": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SD": {"default": "2POS",   "display": [0, 2]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SE": {"default": "TOGGLE", "display": [1, 1]}
    },
    "x9lites": {
        # left side
        "SA": {"default": "3POS",   "display": [0, 0]},
        "SB": {"default": "3POS",   "display": [0, 1]},
        "SD": {"default": "2POS",   "display": [0, 2]},
        "SF": {"default": "TOGGLE", "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 1]},
        "SE": {"default": "TOGGLE", "display": [1, 2]},
        "SG": {"default": "TOGGLE", "display": [1, 3]}
    },
    "x10": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"},
        "SI": {"default": "NONE"},
        "SJ": {"default": "NONE"}
    },
    "x12s": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"},
        "SI": {"default": "NONE"},
        "SJ": {"default": "NONE"}
    },
    "x10express": {
        "SA": {"default": "3POS"},
        "SB": {"default": "3POS"},
        "SC": {"default": "3POS"},
        "SD": {"default": "3POS"},
        "SE": {"default": "3POS"},
        "SF": {"default": "2POS"},
        "SG": {"default": "3POS"},
        "SH": {"default": "TOGGLE"}
    },
    "zorro": {
        # left side
        "SB": {"default": "3POS",   "display": [0, 0]},
        "SA": {"default": "TOGGLE", "display": [0, 1]},
        "SE": {"default": "2POS",   "display": [0, 2]},
        "SG": {"default": "TOGGLE", "display": [0, 3]},
        # right side
        "SC": {"default": "3POS",   "display": [1, 0]},
        "SD": {"default": "TOGGLE", "display": [1, 1]},
        "SF": {"default": "2POS",   "display": [1, 2]},
        "SH": {"default": "TOGGLE", "display": [1, 3]},
    },
    "pocket": {
        # left side
        "SA": { "default": "2POS", "display": [ 0, 0 ] },
        "SB": { "default": "3POS",   "display": [ 0, 1 ] },
        "SE": { "default": "TOGGLE",   "display": [ 0, 2 ] },
        # right side
        "SD": { "default": "2POS",   "display": [ 1, 0 ] },
        "SC": { "default": "3POS", "display": [ 1, 1 ] },
    },
}


def switch_cfg_by_target(target, name):
    """Retrieve default switches config.

    Parameters
    ----------
    target: str
        Name of the target
    name: str
        Canonical switch name (Sx where 'x' is a capital letter)

    Returns
    -------
    dict
        a dictionary with one key for each pot or slider that mapped
        to another dictionary with a "default" attribute containing
        the default switches configuration. On 'stdlcd' targets, a
        'display' 2 dimensional array defining the switch's position
        on screen.
    """
    sw = SWITCH_CONFIG.get(target)
    if sw:
        return sw.get(name)

    return None
