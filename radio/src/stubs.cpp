#include "edgetx.h"

const char *warningText = NULL;
const char *warningInfoText;
uint8_t     warningInfoLength;
uint8_t     warningType;
uint8_t     warningResult = 0;
uint8_t     warningInfoFlags;
void        (*popupFunc)(uint16_t event) = NULL;
const char *popupMenuItems[POPUP_MENU_MAX_LINES];
uint16_t    popupMenuItemsCount = 0;
uint16_t    popupMenuOffset = 0;
uint8_t     popupMenuOffsetType = 0;
void        (*popupMenuHandler)(const char * result);
uint8_t noHighlightCounter;

void runPopupWarning(uint16_t event)
{
}

void drawFatalErrorScreen(const char * message)
{
}

void showMessageBox(const char * title)
{

}

bool menuFirstCalib(event_t event)
{
  return false;
}

void drawAlertBox(const char * title, const char * text, const char * action)
{

}

uint8_t menuLevel;
int menuVerticalOffset;

void runFatalErrorScreen(char const*)
{

}
