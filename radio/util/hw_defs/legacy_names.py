LEGACY_NAMES = [
    {
        "targets": {"x10","x10express","t16","tx16s","t18"},
        "inputs": {
            "P1": {
                "yaml": "S1",
                "lua": "s1",
                "description": "Potentiometer S1"
            },
            "P2": {
                "yaml": "6POS",
                "lua": "6pos",
                "description": "Multipos Switch"
            },
            "P3": {
                "yaml": "S2",
                "lua": "s2",
                "description": "Potentiometer S2"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "description": "Right slider"
            },
            "EXT1": {
                "yaml": "EXT1",
                "lua": "ext1",
                "description": "Ext 1"
            },
            "EXT2": {
                "yaml": "EXT2",
                "lua": "ext2",
                "description": "Ext 2"
            },
            "EXT3": {
                "yaml": "EXT3",
                "lua": "ext3",
                "description": "Ext 3"
            },
            "EXT4": {
                "yaml": "EXT4",
                "lua": "ext4",
                "description": "Ext 4"
            }
        }
    },
    {
        "targets": {"x12s"},
        "inputs": {
            "P1": {
                "yaml": "S1",
                "lua": "s1",
                "description": "Potentiometer S1"
            },
            "P2": {
                "yaml": "6POS",
                "lua": "6pos",
                "description": "Multipos Switch"
            },
            "P3": {
                "yaml": "S2",
                "lua": "s2",
                "description": "Potentiometer S2"
            },
            "SL1": {
                "yaml": "LS",
                "lua": "ls",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "RS",
                "lua": "rs",
                "description": "Right slider"
            },
            "SL3": {
                "yaml": "S3",
                "lua": "s3",
                "description": "Slider S3"
            },
            "SL4": {
                "yaml": "S4",
                "lua": "s4",
                "description": "Slider S4"
            },
            "JSx": {
                "yaml": "MOUSE1",
                "lua": "jsx",
                "description": "Joystick X"
            },
            "JSy": {
                "yaml": "MOUSE2",
                "lua": "jsy",
                "description": "Joystick Y"
            }
        }
    },
    {
        "targets": {"x9e"},
        "inputs": {
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "description": "Potentiometer 3"
            },
            "P4": {
                "yaml": "POT4",
                "lua": "s4",
                "description": "Potentiometer 4 (X9E only)"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "ls",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "rs",
                "description": "Right slider"
            },
            "SL3": {
                "yaml": "SLIDER3",
                "lua": "lcs",
                "description": "Left center slider (X9E only)"
            },
            "SL4": {
                "yaml": "SLIDER4",
                "lua": "rcs",
                "description": "Right center slider (X9E only)"
            }            
        }
    },
    {
        "targets": {"boxer"},
        "inputs": {
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "description": "Potentiometer 3"
            }
        }
    },
    {
        "targets": {
            "commando8",
            "lr3pro",
            "nv14",
            "t8", "t12",
            "tlite", "tpro",
            "tx12", "tx12mk2",
            "x7", "x7access",
            "xlite", "xlites",
            "zorro"
        },
        "inputs": {
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "description": "Potentiometer 2"
            }
        }
    },
    {
        "targets": {"x9lite","x9lites"},
        "inputs": {
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "description": "Potentiometer 1"
            }
        }
    },
    {
        "targets": {"x9d","x9d+","x9d+2019"},
        "inputs": {
            "P1": {
                "yaml": "POT1",
                "lua": "s1",
                "description": "Potentiometer 1"
            },
            "P2": {
                "yaml": "POT2",
                "lua": "s2",
                "description": "Potentiometer 2"
            },
            "P3": {
                "yaml": "POT3",
                "lua": "s3",
                "description": "Potentiometer 3"
            },
            "SL1": {
                "yaml": "SLIDER1",
                "lua": "ls",
                "description": "Left slider"
            },
            "SL2": {
                "yaml": "SLIDER2",
                "lua": "rs",
                "description": "Right slider"
            }
        }
    }
]

def inputs_by_target(target):

    for d in LEGACY_NAMES:
        if target in d['targets']:
            return d['inputs']

    return None
