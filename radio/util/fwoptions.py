#!/usr/bin/env python

languages = (
    "en",
    "fr",
    "se",
    "it",
    "cz",
    "de",
    "pt",
    "es",
    "pl",
    "nl"
)

tts_languages = {
    "en",
    "fr",
    "it",
    "cz",
    "de",
    "pt"
}

options_taranis_x9d = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "haptic": ("HAPTIC", "YES", "NO"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "noras": ("RAS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_taranis_x9dp = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "noras": ("RAS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}
options_taranis_x7 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
}

options_taranis_x9lite = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_taranis_xlite = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_taranis_xlites = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_taranis_x9e = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "sqt5font": ("FONT", "SQT5", None),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "shutdownconfirm": ("SHUTDOWN_CONFIRMATION", "YES", "NO"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "horussticks": ("STICKS", "HORUS", "STANDARD"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_horus_x12s = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "pcbdev": ("PCBREV", "10", None),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "internalaccess": [("INTERNAL_MODULE_PXX1", "NO", None), ("INTERNAL_MODULE_PXX2", "YES", None)],
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_horus_x10 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "internalaccess": [("INTERNAL_MODULE_PXX1", "NO", None), ("INTERNAL_MODULE_PXX2", "YES", None)],
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_horus_x10express = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "luac": ("LUA_COMPILER", "YES", "NO"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "eu": ("MODULE_PROTOCOL_D8", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
}

options_betafpv_lr3pro = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
}

options_jumper_tlite = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
}

options_jumper_t12 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "internalmulti": ("INTERNAL_MODULE_MULTI", "YES", "NO"),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_jumper_t16 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "internalmulti": ("INTERNAL_MODULE_MULTI", "YES", "NO"),
    "bluetooth": ("BLUETOOTH", "YES", "NO"),
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_jumper_t18 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "bluetooth": ("BLUETOOTH", "YES", "NO"),
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_radiomaster_tx12 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO")
}

options_radiomaster_tx12mk2 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_zorro = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_boxer = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_gx12 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_pocket = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_mt12 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_radiomaster_t8 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "bindkey": ("BIND_KEY", "NO", "YES"),
}

options_radiomaster_tx16s = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "bluetooth": ("BLUETOOTH", "YES", "NO"),
    "internalgps": ("INTERNAL_GPS", "YES", "NO"),
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_fatfish_f16 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "bluetooth": ("BLUETOOTH", "YES", "NO"),
    "internalgps": ("INTERNAL_GPS", "YES", "NO"),
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_helloradiosky_v12 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_helloradiosky_v14 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "afhds3": ("AFHDS3", "YES", "NO"),
    "internalelrs": ("INTERNAL_MODULE_ELRS", "YES", "NO"),
}

options_helloradiosky_v16 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
    "flexr9m": ("MODULE_PROTOCOL_FLEX", "YES", None),
    "bluetooth": ("BLUETOOTH", "YES", "NO"),
    "internalgps": ("INTERNAL_GPS", "YES", "NO"),
    "externalaccessmod": ("HARDWARE_EXTERNAL_ACCESS_MOD", "YES", "NO"),
}

options_commando8_t8 = {
    "noheli": ("HELI", "NO", "YES"),
    "lua": ("LUA", "YES", "NO_MODEL_SCRIPTS"),
    "nogvars": ("GVARS", "NO", "YES"),
    "faimode": ("FAI", "YES", None),
    "faichoice": ("FAI", "CHOICE", None),
    "nooverridech": ("OVERRIDE_CHANNEL_FUNCTION", "NO", "YES"),
}
