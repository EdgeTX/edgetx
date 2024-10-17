
#include <stdio.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <locale>
#include <string.h>

#define CFN_ONLY
#define SKIP
#include "../radio/src/dataconstants.h"

#define LCD_W 480
#define TR(a,b) b
#define TR3(a, b, c) c

#if defined(LNG_CN)
#include "../radio/src/translations/cn.h"
#define LOC "zh_CN.UTF-8"
#elif defined(LNG_CZ)
#include "../radio/src/translations/cz.h"
#define LOC "cs_CZ.UTF-8"
#elif defined(LNG_DA)
#include "../radio/src/translations/da.h"
#define LOC "da_DK.UTF-8"
#elif defined(LNG_DE)
#include "../radio/src/translations/de.h"
#define LOC "de_DE.UTF-8"
#elif defined(LNG_EN)
#include "../radio/src/translations/en.h"
#define LOC "en_US.UTF-8"
#elif defined(LNG_ES)
#include "../radio/src/translations/es.h"
#define LOC "es_ES.UTF-8"
#elif defined(LNG_FI)
#include "../radio/src/translations/fi.h"
#define LOC "fi_FI.UTF-8"
#elif defined(LNG_FR)
#include "../radio/src/translations/fr.h"
#define LOC "fr_FR.UTF-8"
#elif defined(LNG_HE)
#include "../radio/src/translations/he.h"
#define LOC "he_IL.UTF-8"
#elif defined(LNG_IT)
#include "../radio/src/translations/it.h"
#define LOC "it_IT.UTF-8"
#elif defined(LNG_JP)
#include "../radio/src/translations/jp.h"
#define LOC "ja_JP.UTF-8"
#elif defined(LNG_NL)
#include "../radio/src/translations/nl.h"
#define LOC "nl_NL.UTF-8"
#elif defined(LNG_PL)
#include "../radio/src/translations/pl.h"
#define LOC "pl_PL.UTF-8"
#elif defined(LNG_PT)
#include "../radio/src/translations/pt.h"
#define LOC "pt_PT.UTF-8"
#elif defined(LNG_RU)
#include "../radio/src/translations/ru.h"
#define LOC "ru_RU.UTF-8"
#elif defined(LNG_SE)
#include "../radio/src/translations/se.h"
#define LOC "sv_SE.UTF-8"
#elif defined(LNG_TW)
#include "../radio/src/translations/tw.h"
#define LOC "zh_TW.UTF-8"
#elif defined(LNG_UA)
#include "../radio/src/translations/ua.h"
#define LOC "uk_UA.UTF-8"
#else
#error "Unknown language"
#endif

struct cfn {
  std::string str;
  std::string nam;
  Functions func;
  std::string cond;
};

struct LocaleComparator {
    std::locale locale;

    LocaleComparator(const std::locale& loc) : locale(loc) {}

    bool operator()(const struct cfn& lhs, const struct cfn& rhs) const {
        return std::use_facet< std::collate<char> >(locale).compare(
            lhs.str.data(), lhs.str.data() + lhs.str.size(),
            rhs.str.data(), rhs.str.data() + rhs.str.size()) < 0;
    }
};

int main()
{
#if defined(LOC)
  std::vector<struct cfn> list = {
    { TR_SF_SAFETY, "FUNC_OVERRIDE_CHANNEL", FUNC_OVERRIDE_CHANNEL, "" },
    { TR_SF_TRAINER, "FUNC_TRAINER", FUNC_TRAINER, "" },
    { TR_SF_INST_TRIM, "FUNC_INSTANT_TRIM", FUNC_INSTANT_TRIM, "" },
    { TR_SF_RESET, "FUNC_RESET", FUNC_RESET, "" },
    { TR_SF_SET_TIMER, "FUNC_SET_TIMER", FUNC_SET_TIMER, "" },
    { TR_ADJUST_GVAR, "FUNC_ADJUST_GVAR", FUNC_ADJUST_GVAR, "" },
    { TR_SF_VOLUME, "FUNC_VOLUME", FUNC_VOLUME, "" },
    { TR_SF_FAILSAFE, "FUNC_SET_FAILSAFE", FUNC_SET_FAILSAFE, "" },
    { TR_SF_RANGE_CHECK, "FUNC_RANGECHECK", FUNC_RANGECHECK, "" },
    { TR_SF_MOD_BIND, "FUNC_BIND", FUNC_BIND, "" },
    { TR_SOUND, "FUNC_PLAY_SOUND", FUNC_PLAY_SOUND, "" },
    { TR_PLAY_TRACK, "FUNC_PLAY_TRACK", FUNC_PLAY_TRACK, "" },
    { TR_PLAY_VALUE, "FUNC_PLAY_VALUE", FUNC_PLAY_VALUE, "" },
    { TR_SF_PLAY_SCRIPT, "FUNC_PLAY_SCRIPT", FUNC_PLAY_SCRIPT, "" },
    { TR_SF_BG_MUSIC, "FUNC_BACKGND_MUSIC", FUNC_BACKGND_MUSIC, "" },
    { TR_SF_BG_MUSIC_PAUSE, "FUNC_BACKGND_MUSIC_PAUSE", FUNC_BACKGND_MUSIC_PAUSE, "" },
    { TR_SF_VARIO, "FUNC_VARIO", FUNC_VARIO, "" },
    { TR_SF_HAPTIC, "FUNC_HAPTIC", FUNC_HAPTIC, "" },
    { TR_SF_LOGS, "FUNC_LOGS", FUNC_LOGS, "" },
    { TR_BRIGHTNESS, "FUNC_BACKLIGHT", FUNC_BACKLIGHT, "defined(OLED_SCREEN)" },
    { TR_SF_BACKLIGHT, "FUNC_BACKLIGHT", FUNC_BACKLIGHT, "!defined(OLED_SCREEN)" },
    { TR_SF_SCREENSHOT, "FUNC_SCREENSHOT", FUNC_SCREENSHOT, "" },	
    { TR_SF_RACING_MODE, "FUNC_RACING_MODE", FUNC_RACING_MODE, "" },
    { TR_SF_DISABLE_TOUCH, "FUNC_DISABLE_TOUCH", FUNC_DISABLE_TOUCH, "defined(COLORLCD)" },
    { TR_SF_SET_SCREEN, "FUNC_SET_SCREEN", FUNC_SET_SCREEN, "defined(COLORLCD)" },
    { TR_SF_SET_TELEMSCREEN, "FUNC_SET_SCREEN", FUNC_SET_SCREEN, "!defined(COLORLCD)" },
    { TR_SF_DISABLE_AUDIO_AMP, "FUNC_DISABLE_AUDIO_AMP", FUNC_DISABLE_AUDIO_AMP, "" },
    { TR_SF_RGBLEDS, "FUNC_RGB_LED", FUNC_RGB_LED, "" },
    { TR_SF_LCD_TO_VIDEO, "FUNC_LCD_TO_VIDEO", FUNC_LCD_TO_VIDEO, "defined(VIDEO_SWITCH)" },
    { TR_SF_PUSH_CUST_SWITCH, "FUNC_PUSH_CUST_SWITCH", FUNC_PUSH_CUST_SWITCH, "defined(FUNCTION_SWITCHES)" },
    { TR_SF_TEST, "FUNC_TEST", FUNC_TEST, "defined(DEBUG)" },
  };

  std::locale locale(LOC);
  std::sort(list.begin(), list.end(), LocaleComparator(locale));

  for (int i = 0; i < list.size(); i += 1) {
    bool addEndif = false;
    if (!list[i].cond.empty()) {
      printf("#if %s\n", list[i].cond.c_str());
      addEndif = true;
    }
    printf("  /* %s */ %s,\n", list[i].str.c_str(), list[i].nam.c_str());
    if (addEndif)
      printf("#endif\n");
  }

  return 0;
#else
static_assert(false,"No valid language defined!");
#endif
}
