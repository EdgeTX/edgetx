#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "trainer_driver.h"

#include "hal.h"
#include "edgetx.h"

static void initSpeakerEnable()
{
  gpio_init(AUDIO_SPEAKER_ENABLE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

static void enableSpeaker()
{
  gpio_set(AUDIO_SPEAKER_ENABLE_GPIO);
}

static void disableSpeaker()
{
  gpio_clear(AUDIO_SPEAKER_ENABLE_GPIO);
}

static void initHeadphoneTrainerSwitch()
{
  gpio_init(HEADPHONE_TRAINER_SWITCH_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

static void enableHeadphone()
{
  gpio_clear(HEADPHONE_TRAINER_SWITCH_GPIO);
}

static void enableTrainer()
{
  gpio_set(HEADPHONE_TRAINER_SWITCH_GPIO);
}

static void initJackDetect(void)
{
  gpio_init(JACK_DETECT_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
}

static bool isJackPlugged()
{
  // debounce
  static bool debounced_state = 0;
  static bool last_state = 0;

  if (gpio_read(JACK_DETECT_GPIO)) {
    if (!last_state) {
      debounced_state = false;
    }
    last_state = false;
  }
  else {
    if (last_state) {
      debounced_state = true;
    }
    last_state = true;
  }
  return debounced_state;
}

enum JackState
{
  SPEAKER_ACTIVE,
  HEADPHONE_ACTIVE,
  TRAINER_ACTIVE,
};

uint8_t jackState = SPEAKER_ACTIVE;

const char STR_JACK_HEADPHONE[] = "Headphone";
const char STR_JACK_TRAINER[] = "Trainer";

void onJackConnectMenu(const char* result)
{
  if (result == STR_JACK_HEADPHONE) {
    jackState = HEADPHONE_ACTIVE;
    disableSpeaker();
    enableHeadphone();
  } else if (result == STR_JACK_TRAINER) {
    jackState = TRAINER_ACTIVE;
    enableTrainer();
  }
}

void handleJackConnection()
{
  if (jackState == SPEAKER_ACTIVE && isJackPlugged()) {
    if (g_eeGeneral.jackMode == JACK_HEADPHONE_MODE) {
      jackState = HEADPHONE_ACTIVE;
      disableSpeaker();
      enableHeadphone();
    } else if (g_eeGeneral.jackMode == JACK_TRAINER_MODE) {
      jackState = TRAINER_ACTIVE;
      enableTrainer();
    } else if (popupMenuItemsCount == 0) {
      POPUP_MENU_START(onJackConnectMenu, 2, STR_JACK_HEADPHONE, STR_JACK_TRAINER);
    }
  } else if (jackState == SPEAKER_ACTIVE && !isJackPlugged() &&
             popupMenuItemsCount > 0 && popupMenuHandler == onJackConnectMenu) {
    popupMenuItemsCount = 0;
  } else if (jackState != SPEAKER_ACTIVE && !isJackPlugged()) {
    jackState = SPEAKER_ACTIVE;
    enableSpeaker();
  }
}

// defined __WEAK in boards/generic_stm32/trainer_driver.cpp
void board_trainer_init()
{
  initJackDetect();
  initSpeakerEnable();
  enableSpeaker();
  initHeadphoneTrainerSwitch();

  trainer_init();
}

// defined __WEAK in boards/generic_stm32/trainer_driver.cpp
bool is_trainer_dsc_connected()
{
  return jackState == TRAINER_ACTIVE;  
}

