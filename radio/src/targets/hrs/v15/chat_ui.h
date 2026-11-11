#pragma once

#include <stdint.h>

// V15 UART5 chat: line with `Display:` (or raw `Application: >>`/`<<`) is parsed;
// rolling line shows the directive body (text after tts:/stt:/Application: >>/…).
// `Display: clear` clears the line.

// Enums are always available, regardless of module enable/disable
enum {
  V15_CHAT_UI_STATE_REST = 0,
  V15_CHAT_UI_STATE_LISTENING = 1,
  V15_CHAT_UI_STATE_ANSWERING = 2,
};

#if defined(MODULE_XIAOZHI_CHAT)

void v15ChatUiTask(void);
uint8_t v15ChatUiGetVisualState(void);

#else

inline uint8_t v15ChatUiGetVisualState(void) { return 0; }

#endif
