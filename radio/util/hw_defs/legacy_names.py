"""Legacy names used for each analog input in previous EdgeTx versions."""

LEGACY_NAMES = [
    {
        "targets": {"x10", "x10express", "t16", "tx16s", "t18", "f16", "v16"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "S1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer S1"
            },
            "P2": {
                "yaml": "6POS",
                "lua": "6pos",
                "label": "6POS",
                "short_label": "3",
                "description": "Multipos Switch"
            },
            "P3": {
                "yaml": "S2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer S2"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider"
            },
            "EXT1": {
                "yaml": "EXT1",
                "lua": "ext1",
                "label": "EXT1",
                "short_label": "E1",
                "description": "Ext 1"
            },
            "EXT2": {
                "yaml": "EXT2",
                "lua": "ext2",
                "label": "EXT2",
                "short_label": "E2",
                "description": "Ext 2"
            },
            "EXT3": {
                "yaml": "EXT3",
                "lua": "ext3",
                "label": "EXT3",
                "short_label": "E3",
                "description": "Ext 3"
            },
            "EXT4": {
                "yaml": "EXT4",
                "lua": "ext4",
                "label": "EXT4",
                "short_label": "E4",
                "description": "Ext 4"
            }
        }
    },
    {
        "targets": {"t15"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "S1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer S1"
            },
            "P2": {
                "yaml": "S2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer S2"
            }
        }
    },
    {
        "targets": {"x12s"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "S1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer S1"
            },
            "P2": {
                "yaml": "6POS",
                "lua": "6pos",
                "label": "6POS",
                "short_label": "3",
                "description": "Multipos Switch"
            },
            "P3": {
                "yaml": "S2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer S2"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider"
            },
            "SL3": {
                "yaml": "S3",
                "lua": "s3",
                "label": "L1",
                "short_label": "1",
                "description": "Slider S3"
            },
            "SL4": {
                "yaml": "S4",
                "lua": "s4",
                "label": "L2",
                "short_label": "2",
                "description": "Slider S4"
            },
            "JSx": {
                "yaml": "MOUSE1",
                "lua": "jsx",
                "label": "JSx",
                "short_label": "X",
                "description": "Joystick X"
            },
            "JSy": {
                "yaml": "MOUSE2",
                "lua": "jsy",
                "label": "JSy",
                "short_label": "Y",
                "description": "Joystick Y"
            }
        }
    },
    {
        "targets": {"x9e"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "F1",
                "short_label": "F1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "F2",
                "short_label": "F2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "F3",
                "short_label": "F3",
                "description": "Potentiometer 3"
            },
            "P4": {
                "yaml": "POT4",
                "lua": "s4",
                "label": "F4",
                "short_label": "F4",
                "description": "Potentiometer 4"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "lcs",
                "label": "S1",
                "short_label": "S1",
                "description": "Left center slider"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "rcs",
                "label": "S2",
                "short_label": "S2",
                "description": "Right center slider"
            },
            "SL3": {
                "yaml": "SLIDER3",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL4": {
                "yaml": "SLIDER4",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider"
            },
        }
    },
    {
        "targets": {"boxer"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "6 pos"
            }
        }
    },
    {
        "targets": {"gx12"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "p1",
                "label": "P1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "p2",
                "label": "P2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "s1",
                "label": "S1",
                "short_label": "S1",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "s2",
                "label": "S2",
                "short_label": "S2",
                "description": "Right slider"
            }
        }
    },
    {
        "targets": {"mt12"},
        "inputs": {
            "ST": {
                "yaml": "ST",
                "lua": "ste",
                "description": "Steering"
            },
            "TH": {
                "yaml": "TH",
                "lua": "thr",
                "description": "Throttle"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "Analog 3"
            },
            "P4": {
                "yaml": "POT4",
                "lua": "s4",
                "label": "S4",
                "short_label": "4",
                "description": "Analog 4"
            }
        }
    },
    {
        "targets": {"t14", "t12max", "tpros"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
        }
    },
    {
        "targets": {
            "commando8",
            "lr3pro",
            "nv14", "el18",
            "t8", "t12",
            "tlite", "tlitef4", "tpro", "tprov2", "tpros", "bumblebee",
            "tx12", "tx12mk2",
            "x7", "x7access",
            "xlite", "xlites",
            "zorro"
        },
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            }
        }
    },
    {
        "targets": {
            "pl18"
        },
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "Potentiometer 3"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider"
            },
        }
    },
    {
        "targets": {
            "pl18ev"
        },
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "Potentiometer 3"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider"
            },
            "EXT1": {
                "yaml": "EXT1",
                "lua": "ext1",
                "label": "EXT1",
                "short_label": "E1",
                "description": "Ext 1"
            },
            "EXT2": {
                "yaml": "EXT2",
                "lua": "ext2",
                "label": "EXT2",
                "short_label": "E2",
                "description": "Ext 2"
            },
            "EXT3": {
                "yaml": "EXT3",
                "lua": "ext3",
                "label": "EXT3",
                "short_label": "E3",
                "description": "Ext 3"
            },
            "EXT4": {
                "yaml": "EXT4",
                "lua": "ext4",
                "label": "EXT4",
                "short_label": "E4",
                "description": "Ext 4"
            }
        }
    },
    {
        "targets": {"v12", "v14"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "6 pos"
            }
        }
    },
    {
        "targets": {"t20","t20v2"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "Potentiometer 3"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "s4",
                "label": "S4",
                "short_label": "4",
                "description": "Potentiometer 4"
            },
            "SL3": {
                "yaml": "SLIDER3",
                "lua": "sl",
                "label": "SL",
                "short_label": "L",
                "description": "Right finger slider"
            },
            "SL4": {
                "yaml": "SLIDER4",
                "lua": "sr",
                "label": "SR",
                "short_label": "R",
                "description": "Right finger slider"
            }
        }
    },
    {
        "targets": {"pocket"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            }
        }
    },
    {
        "targets": {"x9lite", "x9lites"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            }
        }
    },
    {
        "targets": {"x9d", "x9d+", "x9d+2019"},
        "inputs": {
            "LH": {
                "yaml": "Rud",
                "lua": "rud",
                "description": "Rudder"
            },
            "LV": {
                "yaml": "Ele",
                "lua": "ele",
                "description": "Elevator"
            },
            "RV": {
                "yaml": "Thr",
                "lua": "thr",
                "description": "Throttle"
            },
            "RH": {
                "yaml": "Ail",
                "lua": "ail",
                "description": "Aileron"
            },
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "label": "S3",
                "short_label": "3",
                "description": "Potentiometer 3"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "ls",
                "label": "LS",
                "short_label": "L",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "rs",
                "label": "RS",
                "short_label": "R",
                "description": "Right slider",
            },
        },
    },
    {
        "targets": {"h747", "h7rs"},
        "inputs": {
            "LH": {"yaml": "Rud", "lua": "rud", "description": "Rudder"},
            "LV": {"yaml": "Ele", "lua": "ele", "description": "Elevator"},
            "RV": {"yaml": "Thr", "lua": "thr", "description": "Throttle"},
            "RH": {"yaml": "Ail", "lua": "ail", "description": "Aileron"},
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "label": "S1",
                "short_label": "1",
                "description": "Potentiometer 1",
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "label": "S2",
                "short_label": "2",
                "description": "Potentiometer 2",
            },
        },
    },
]


def inputs_by_target(target):
    """Retrieve legacy definitions by target.

    Parameters
    ----------
    target: str
        Name of the target

    Returns
    -------
    dict
        a dictionary with the basic attributes needed to generate
        compatibility mappings
    """
    for d in LEGACY_NAMES:
        if target in d['targets']:
            return d['inputs']

    return None
