"""Default configuration of pots and sliders for each target."""

POT_CONFIG = {
    "boxer": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"},
        "P3": {"default": "MULTIPOS_SWITCH"}
    },
    "el18": {
        "P1": { "default": "WITHOUT_DETENT"},
        "P2": { "default": "WITHOUT_DETENT"}
    },
    "nv14": {
        "P1": { "default": "WITHOUT_DETENT"},
        "P2": { "default": "WITHOUT_DETENT"}
    },
    "tpro": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "tprov2": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "t20": {
        "P1":  {"default": "WITH_DETENT"},
        "P2":  {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"},
        "SL3": {"default": "SLIDER_WITH_DETENT"},
        "SL4": {"default": "SLIDER_WITH_DETENT"},
    },
    "t12": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITHOUT_DETENT"}
    },
    "tx12": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "tx12mk2": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "t16": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "t18": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "tx16s": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x7": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "x7access": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITH_DETENT"}
    },
    "x9d": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x9dp": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x9dp2019": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x9e": {
        "P1": {"default": "WITH_DETENT"},
        "P2": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"},
        "SL3": {"default": "SLIDER_WITH_DETENT"},
        "SL4": {"default": "SLIDER_WITH_DETENT"}
    },
    "x10": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x10express": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"}
    },
    "x12s": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "MULTIPOS_SWITCH"},
        "P3": {"default": "WITH_DETENT"},
        "SL1": {"default": "SLIDER_WITH_DETENT"},
        "SL2": {"default": "SLIDER_WITH_DETENT"},
        "SL3": {"default": "SLIDER_WITH_DETENT"},
        "SL4": {"default": "SLIDER_WITH_DETENT"}
    },
    "xlite": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITHOUT_DETENT"}
    },
    "xlites": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITHOUT_DETENT"}
    },
    "x9lite": {
        "P1": {"default": "WITH_DETENT"},
    },
    "x9lites": {
        "P1": {"default": "WITH_DETENT"},
    },
    "zorro": {
        "P1": {"default": "WITHOUT_DETENT"},
        "P2": {"default": "WITHOUT_DETENT"}
    },
}


def pot_cfg_by_target(target, name):
    """Retrieve default pot config.

    Parameters
    ----------
    target: str
        Name of the target
    name: str
        Canonical name of the pot (Px where 'x' is a number)
        or slider (SLx where 'x' is a number)

    Returns
    -------
    dict
        a dictionary with one key for each pot or slider that mapped
        to another dictionary with a single attribute ("default") containing
        the default configuration for a pot or slider
    """
    sw = POT_CONFIG.get(target)
    if sw:
        return sw.get(name)

    return None
