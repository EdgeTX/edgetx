
POT_CONFIG = {
    "boxer": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" },
        "P3": { "default": "MULTIPOS_SWITCH" }
    },
    "tpro": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" }
    },
    "t12": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITHOUT_DETENT" }
    },
    "tx12": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" }
    },
    "tx12mk2": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" }
    },
    "t16": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "t18": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "tx16s": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x7": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITH_DETENT" }
    },
    "x7access": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITH_DETENT" }
    },
    "x9d": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x9dp": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x9dp2019": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x9e": {
        "P1": { "default": "WITH_DETENT" },
        "P2": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" },
        "SL3": { "default": "SLIDER_WITH_DETENT" },
        "SL4": { "default": "SLIDER_WITH_DETENT" }
    },
    "x10": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x10express": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" }
    },
    "x12s": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "MULTIPOS_SWITCH" },
        "P3": { "default": "WITH_DETENT" },
        "SL1": { "default": "SLIDER_WITH_DETENT" },
        "SL2": { "default": "SLIDER_WITH_DETENT" },
        "SL3": { "default": "SLIDER_WITH_DETENT" },
        "SL4": { "default": "SLIDER_WITH_DETENT" }
    },
    "xlite": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITHOUT_DETENT" }
    },
    "xlites": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITHOUT_DETENT" }
    },
    "x9lite": {
        "P1": { "default": "WITH_DETENT" },
    },
    "x9lites": {
        "P1": { "default": "WITH_DETENT" },
    },
    "zorro": {
        "P1": { "default": "WITHOUT_DETENT" },
        "P2": { "default": "WITHOUT_DETENT" }
    },
}

def pot_cfg_by_target(target, name):

    sw = POT_CONFIG.get(target)
    if sw:
        return sw.get(name)

    return None
