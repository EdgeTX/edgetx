/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "fx.h"
#include "FXExpression.h"
#include "FXPNGImage.h"
#include <unistd.h>
#include "fxkeys.h"
#include "opentx.h"
#include <time.h>
#include <ctype.h>
#include "targets/simu/simulcd.h"

#if defined(SIMU_AUDIO)
  #include <SDL.h>
  #undef main
#endif

#if LCD_W > 212
  #define LCD_ZOOM 1
#else
  #define LCD_ZOOM 2
#endif

#define W2 LCD_W*LCD_ZOOM
#define H2 LCD_H*LCD_ZOOM

#if defined(HARDWARE_TOUCH)
  #define TAP_TIME 25

  tmr10ms_t now = 0;
  tmr10ms_t downTime = 0;
  tmr10ms_t tapTime = 0;
  short tapCount = 0;
#endif

class OpenTxSim: public FXMainWindow
{
  FXDECLARE(OpenTxSim)

  public:
    OpenTxSim(){};
    OpenTxSim(FXApp* a);
    ~OpenTxSim();
    void updateKeysAndSwitches(bool start=false);
    long onKeypress(FXObject*, FXSelector, void*);
    long onMouseDown(FXObject*,FXSelector,void*);
    long onMouseUp(FXObject*,FXSelector,void*);
    long onMouseMove(FXObject*,FXSelector,void*);
    long onTimeout(FXObject*, FXSelector, void*);
    void createBitmap(int index, uint16_t *data, int x, int y, int w, int h);
    void makeSnapshot(const FXDrawable* drawable);
    void doEvents();
    void refreshDisplay();
    void setPixel(int x, int y, FXColor color);

  private:
    FXImage * bmp;
    FXImageFrame * bmf;

  public:
    FXSlider * sliders[NUM_STICKS];
    FXKnob * knobs[NUM_POTS+NUM_SLIDERS];
};

// Message Map
FXDEFMAP(OpenTxSim) OpenTxSimMap[] = {
  // Message_Type   _______ID____Message_Handler_______
  FXMAPFUNC(SEL_TIMEOUT,   2,    OpenTxSim::onTimeout),
  FXMAPFUNC(SEL_KEYPRESS,  0,    OpenTxSim::onKeypress),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS, 0, OpenTxSim::onMouseDown),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE, 0, OpenTxSim::onMouseUp),
  FXMAPFUNC(SEL_MOTION,    0,    OpenTxSim::onMouseMove),
};

FXIMPLEMENT(OpenTxSim, FXMainWindow, OpenTxSimMap, ARRAYNUMBER(OpenTxSimMap))

OpenTxSim::OpenTxSim(FXApp* a):
  FXMainWindow(a, "OpenTX Simu", nullptr, nullptr, DECOR_ALL, 20, 90, 0, 0)
{
  bmp = new FXPPMImage(getApp(), nullptr, IMAGE_OWNED|IMAGE_KEEP|IMAGE_SHMI|IMAGE_SHMP, W2, H2);

#if defined(SIMU_AUDIO)
  SDL_Init(SDL_INIT_AUDIO);
#endif

  FXHorizontalFrame * hf11 = new FXHorizontalFrame(this, LAYOUT_CENTER_X);
  FXHorizontalFrame * hf1 = new FXHorizontalFrame(this, LAYOUT_FILL_X);

  //rh lv rv lh
  for (int i=0; i<4; i++) {
    switch (i) {
#define L LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_X|LAYOUT_FIX_Y
      case 0:
        sliders[i]=new FXSlider(hf1, nullptr, 0, L|SLIDER_HORIZONTAL, 10, 110, 100, 20);
        break;
      case 1:
        sliders[i]=new FXSlider(hf1, nullptr, 0, L|SLIDER_VERTICAL, 110, 10, 20, 100);
        break;
      case 2:
        sliders[i]=new FXSlider(hf1, nullptr, 0, L|SLIDER_VERTICAL, 130, 10, 20, 100);
        break;
      case 3:
        sliders[i]=new FXSlider(hf1, nullptr, 0, L|SLIDER_HORIZONTAL, 150, 110, 100, 20);
        break;
      default:;
    }
    sliders[i]->setRange(-1024, 1024);
    sliders[i]->setTickDelta(7);
    sliders[i]->setValue(0);
  }

  for (int i = 0; i < NUM_POTS + NUM_SLIDERS; i++) {
    knobs[i]= new FXKnob(hf11, nullptr, 0, KNOB_TICKS|LAYOUT_LEFT);
    knobs[i]->setValue(0);

#if defined(PCBHORUS)
    if (i == 1) {  // 6-pos switch
      knobs[i]->setRange(0, 2048);
      knobs[i]->setIncrement(2048 / 5);
      knobs[i]->setTickDelta(2048 / 5);
      continue;
    }
#endif

    knobs[i]->setRange(-1024, 1024);
  }

  bmf = new FXImageFrame(this, bmp);
  bmf->enable();
  bmf->setTarget(this);

  updateKeysAndSwitches(true);

  getApp()->addTimeout(this, 2, 100);
}

OpenTxSim::~OpenTxSim()
{
  TRACE("OpenTxSim::~OpenTxSim()");

  simuStop();
  stopAudioThread();

#if defined(EEPROM)
  stopEepromThread();
#endif

  delete bmp;
  delete sliders[0];
  delete sliders[1];
  delete sliders[2];
  delete sliders[3];

  for (int i = 0; i < NUM_POTS + NUM_SLIDERS; i++) {
    delete knobs[i];
  }

  delete bmf;

#if defined(SIMU_AUDIO)
  SDL_Quit();
#endif
}

void OpenTxSim::createBitmap(int index, uint16_t *data, int x, int y, int w, int h)
{
  FXPNGImage snapshot(getApp(), nullptr, IMAGE_OWNED, w, h);

  for (int i=0; i<w; i++) {
    for (int j=0; j<h; j++) {
      pixel_t z = data[(y+j) * LCD_W + (x+i)];
      FXColor color = FXRGB(255*((z&0xF00)>>8)/0x0f, 255*((z&0x0F0)>>4)/0x0f, 255*(z&0x00F)/0x0f);
      snapshot.setPixel(i, j, color);
    }
  }

  FXFileStream stream;
  char buf[32];
  sprintf(buf, "%02d.png", index);
  if (stream.open(buf, FXStreamSave)) {
    snapshot.savePixels(stream);
    stream.close();
    TRACE("Bitmap %d (w=%d, h=%d) created", index, w, h);
  }
  else {
    TRACE("Bitmap %d (w=%d, h=%d) error", index, w, h);
  }
}

void OpenTxSim::makeSnapshot(const FXDrawable* drawable)
{
  // Construct and create an FXImage object
  FXPNGImage snapshot(getApp(), nullptr, 0, drawable->getWidth(), drawable->getHeight());
  snapshot.create();

  // Create a window device context and lock it onto the image
  FXDCWindow dc(&snapshot);

  // Draw from the widget to this
  dc.drawArea(drawable, 0, 0, drawable->getWidth(), drawable->getHeight(), 0, 0);

  // Release lock
  dc.end();

  // Grab pixels from server side back to client side
  snapshot.restore();

  // Save recovered pixels to a file
  FXFileStream stream;
  char buf[100];

  do {
    stream.close();
    sprintf(buf, "snapshot_%02d.png", ++g_snapshot_idx);
  } while (stream.open(buf, FXStreamLoad));

  if (stream.open(buf, FXStreamSave)) {
    snapshot.savePixels(stream);
    stream.close();
    printf("Snapshot written: %s\n", buf);
  }
  else {
    printf("Cannot create snapshot %s\n", buf);
  }
}

void OpenTxSim::doEvents()
{
  getApp()->runOneEvent(false);
}

long OpenTxSim::onKeypress(FXObject *, FXSelector, void * v)
{
  auto * evt = (FXEvent *)v;

  // TRACE("keypress %x", evt->code);

  if (evt->code == 's') {
    makeSnapshot(bmf);
  }

  return 0;
}

long OpenTxSim::onMouseDown(FXObject *, FXSelector, void * v)
{
  FXEvent * evt = (FXEvent *)v;
  UNUSED(evt);

  TRACE_WINDOWS("[Mouse Press] %d %d", evt->win_x, evt->win_y);

#if defined(HARDWARE_TOUCH)
  now = get_tmr10ms();
  simTouchState.tapCount = 0;

  simTouchState.event = TE_DOWN;
  simTouchState.startX = simTouchState.x = evt->win_x;
  simTouchState.startY = simTouchState.y = evt->win_y;
  downTime = now;
  simTouchOccured = true;
#endif

  return 0;
}

long OpenTxSim::onMouseUp(FXObject*,FXSelector,void*v)
{
  FXEvent * evt = (FXEvent *)v;
  UNUSED(evt);

  TRACE_WINDOWS("[Mouse Release] %d %d", evt->win_x, evt->win_y);

#if defined(HARDWARE_TOUCH)
  now = get_tmr10ms();

  if (simTouchState.event == TE_DOWN) {
    simTouchState.event = TE_UP;
    simTouchState.x = simTouchState.startX;
    simTouchState.y = simTouchState.startY;
    if (now - downTime <= TAP_TIME) {
      if (now - tapTime > TAP_TIME)
        tapCount = 1;
      else
        tapCount++;
      simTouchState.tapCount = tapCount;
      tapTime = now;
    }
  }
  else {
    simTouchState.event = TE_SLIDE_END;
  }
  simTouchOccured = true;
#endif

  return 0;
}

long OpenTxSim::onMouseMove(FXObject*,FXSelector,void*v)
{
  FXEvent * evt = (FXEvent *)v;
  UNUSED(evt);

  if (evt->state & LEFTBUTTONMASK) {
    TRACE_WINDOWS("[Mouse Move] %d %d", evt->win_x, evt->win_y);

#if defined(HARDWARE_TOUCH)
    simTouchState.deltaX += evt->win_x - simTouchState.x;
    simTouchState.deltaY += evt->win_y - simTouchState.y;
    if (simTouchState.event == TE_SLIDE || abs(simTouchState.deltaX) >= SLIDE_RANGE || abs(simTouchState.deltaY) >= SLIDE_RANGE) {
      simTouchState.event = TE_SLIDE;
      simTouchState.x = evt->win_x;
      simTouchState.y = evt->win_y;
    }
    simTouchOccured = true;
#endif
  }

  return 0;
}

void OpenTxSim::updateKeysAndSwitches(bool start)
{
  static int keys[] = {
#if defined(PCBNV14)
    // no keys
#elif defined(PCBHORUS)
    KEY_Page_Up,   KEY_PGUP,
    KEY_Page_Down, KEY_PGDN,
    KEY_Return,    KEY_ENTER,
    KEY_Up,        KEY_UP,
    KEY_Down,      KEY_DOWN,
    KEY_Right,     KEY_RIGHT,
    KEY_Left,      KEY_LEFT,
#elif defined(PCBXLITE) || defined(RADIO_FAMILY_JUMPER_T12)
  #if defined(KEYS_GPIO_REG_SHIFT)
    KEY_Shift_L,   KEY_SHIFT,
  #endif
    KEY_Return,    KEY_ENTER,
    KEY_BackSpace, KEY_EXIT,
    KEY_Right,     KEY_RIGHT,
    KEY_Left,      KEY_LEFT,
    KEY_Up,        KEY_UP,
    KEY_Down,      KEY_DOWN,
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
    KEY_Page_Up,   KEY_PAGEUP,
    KEY_Page_Down, KEY_PAGEDN,
    KEY_Return,    KEY_ENTER,
    KEY_Up,        KEY_MODEL,
    KEY_Down,      KEY_EXIT,
    KEY_Right,     KEY_TELE,
    KEY_Left,      KEY_SYS,
#elif defined(RADIO_T8)
    KEY_Page_Up,   KEY_PAGEUP,
    KEY_Page_Down, KEY_PAGEDN,
    KEY_Return,    KEY_ENTER,
    KEY_Right,     KEY_MODEL,
    KEY_BackSpace, KEY_EXIT,
    KEY_Left,      KEY_SYS,
    KEY_Up,        KEY_PLUS,
    KEY_Down,      KEY_MINUS,
#elif defined(PCBTARANIS)
    KEY_Page_Up,   KEY_MENU,
  #if defined(KEYS_GPIO_REG_PAGE)
    KEY_Page_Down, KEY_PAGE,
  #endif
    KEY_Return,    KEY_ENTER,
    KEY_BackSpace, KEY_EXIT,
    KEY_Up,        KEY_PLUS,
    KEY_Down,      KEY_MINUS,
#else
    KEY_Return,    KEY_MENU,
    KEY_BackSpace, KEY_EXIT,
    KEY_Right,     KEY_RIGHT,
    KEY_Left,      KEY_LEFT,
    KEY_Up,        KEY_UP,
    KEY_Down,      KEY_DOWN,
#endif
  };

  for (unsigned int i=0; i<DIM(keys); i+=2) {
    simuSetKey(keys[i+1], start ? false : getApp()->getKeyState(keys[i]));
  }

#ifdef __APPLE__
  // gruvin: Can't use Function keys on the Mac -- too many other app conflicts.
  //         The ordering of these keys, Q/W,E/R,T/Y,U/I matches the on screen
  //         order of trim sliders
  static FXuint trimKeys[] = { KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0 };
#else
  static FXuint trimKeys[] = { KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12 };
#endif

  for (unsigned i=0; i<sizeof(trimKeys)/(2*sizeof(FXuint)); i++) {
    simuSetTrim(i, getApp()->getKeyState(trimKeys[i]));
  }

#define SWITCH_KEY(key, swtch, states) \
  static bool state##key = 0; \
  static int8_t state_##swtch = -1; \
  static int8_t inc_##swtch = 4-states; \
  if (getApp()->getKeyState(KEY_##key)) { \
    if (!state##key) { \
      state_##swtch = (state_##swtch+inc_##swtch); \
      if (state_##swtch == -1+((states-1)*inc_##swtch)) inc_##swtch = -4+states; \
      else if (state_##swtch == -1) inc_##swtch = 4-states; \
      state##key = true; \
    } \
  } \
  else { \
    state##key = false; \
  } \
  simuSetSwitch(swtch, state_##swtch) \

  SWITCH_KEY(A, 0, 3);
  SWITCH_KEY(B, 1, 3);
  SWITCH_KEY(C, 2, 3);
  SWITCH_KEY(D, 3, 3);

  #if defined(RADIO_TPRO)
    SWITCH_KEY(1, 4, 2);
    SWITCH_KEY(2, 5, 2);
    SWITCH_KEY(3, 6, 2);
    SWITCH_KEY(4, 7, 2);
    SWITCH_KEY(5, 8, 2);
    SWITCH_KEY(6, 9, 2);
  #elif defined(HARDWARE_SWITCH_G) && defined(HARDWARE_SWITCH_H)
    SWITCH_KEY(E, 4, 3);
    SWITCH_KEY(F, 5, 2);
    SWITCH_KEY(G, 6, 3);
    SWITCH_KEY(H, 7, 2);
  #elif defined(HARDWARE_SWITCH_F) && defined(HARDWARE_SWITCH_H)
    SWITCH_KEY(F, 4, 2);
    SWITCH_KEY(H, 5, 2);
  #endif

  #if defined(PCBX9E)
    SWITCH_KEY(I, 8, 3);
    SWITCH_KEY(J, 9, 3);
    SWITCH_KEY(K, 10, 3);
    SWITCH_KEY(L, 11, 3);
    SWITCH_KEY(M, 12, 3);
    SWITCH_KEY(N, 13, 3);
    SWITCH_KEY(O, 14, 3);
    SWITCH_KEY(P, 15, 3);
    SWITCH_KEY(Q, 16, 3);
    SWITCH_KEY(R, 17, 3);
  #endif
}

long OpenTxSim::onTimeout(FXObject*, FXSelector, void*)
{
  if (hasFocus()) {

    updateKeysAndSwitches();

#if defined(ROTARY_ENCODER_NAVIGATION)
    static bool rotencAction = false;
    if (getApp()->getKeyState(KEY_X) || getApp()->getKeyState(KEY_plus)) {
      if (!rotencAction) ROTARY_ENCODER_NAVIGATION_VALUE += ROTARY_ENCODER_GRANULARITY;
      rotencAction = true;
    }
    else if (getApp()->getKeyState(KEY_W) || getApp()->getKeyState(KEY_minus)) {
      if (!rotencAction) ROTARY_ENCODER_NAVIGATION_VALUE -= ROTARY_ENCODER_GRANULARITY;
      rotencAction = true;
    }
    else {
      rotencAction = false;
    }
#endif
  }

#if !defined(SIMU_BOOTLOADER)
  per10ms();
#else
  void interrupt10ms();
  interrupt10ms();
#endif
  refreshDisplay();
  getApp()->addTimeout(this, 2, 10);
  return 0;
}

#if LCD_W >= 212
  #define BL_COLOR FXRGB(47, 123, 227)
#else
  #define BL_COLOR FXRGB(150, 200, 152)
#endif

void OpenTxSim::setPixel(int x, int y, FXColor color)
{
#if LCD_ZOOM > 1
  for (int i=0; i<LCD_ZOOM; ++i) {
    for (int j=0; j<LCD_ZOOM; ++j) {
      bmp->setPixel(LCD_ZOOM*x+i, LCD_ZOOM*y+j, color);
    }
  }
#else
  bmp->setPixel(x, y, color);
#endif
}

// from lcd driver
void lcdFlushed();

void OpenTxSim::refreshDisplay()
{
  static bool lightEnabled = (bool)isBacklightEnabled();

  if ((bool(isBacklightEnabled()) != lightEnabled) || simuLcdRefresh) {

    if (bool(isBacklightEnabled()) != lightEnabled) {
      lightEnabled = (bool)isBacklightEnabled();
      TRACE("backlight %s", lightEnabled ? "ON" : "OFF");
    }

#if !defined(COLORLCD)
    FXColor offColor = isBacklightEnabled() ? BL_COLOR : FXRGB(200, 200, 200);
    FXColor onColor = FXRGB(0, 0, 0);
#endif
    for (int x = 0; x < LCD_W; x++) {
      for (int y = 0; y < LCD_H; y++) {
#if defined(COLORLCD)
        pixel_t z = simuLcdBuf[y * LCD_W + x];
        FXColor color =
          FXRGB(((z & 0xF800) >> 8) + ((z & 0xE000) >> 13),
                ((z & 0x07E0) >> 3) + ((z & 0x0600) >> 9),
                (((z & 0x001F) << 3) & 0x00F8) + ((z & 0x001C) >> 2));
        setPixel(x, y, color);
#else
#if LCD_DEPTH == 4
        pixel_t *p = &simuLcdBuf[y / 2 * LCD_W + x];
        uint8_t z = (y & 1) ? (*p >> 4) : (*p & 0x0F);
        if (z) {
          FXColor color;
          if (isBacklightEnabled())
            color = FXRGB(47 - (z * 47) / 15, 123 - (z * 123) / 15,
                          227 - (z * 227) / 15);
          else
            color = FXRGB(200 - (z * 200) / 15, 200 - (z * 200) / 15,
                          200 - (z * 200) / 15);
          setPixel(x, y, color);
        }
#else  // LCD_DEPTH == 1
        if (simuLcdBuf[x + (y / 8) * LCD_W] & (1 << (y % 8))) {
          setPixel(x, y, onColor);
        }
#endif
        else {
          setPixel(x, y, offColor);
        }
#endif  // !defined(COLORLCD)
      }
    }
#if defined(COLORLCD)
    if (!lightEnabled) { bmp->fade(0, 50); }
#endif

    bmp->render();
    bmf->setImage(bmp);

    simuLcdRefresh = false;
    lcdFlushed();
  }
}

OpenTxSim * opentxSim;

void doFxEvents()
{
  //puts("doFxEvents");
  opentxSim->getApp()->runOneEvent(false);
  opentxSim->refreshDisplay();
}

int main(int argc, char ** argv)
{
  // Each FOX GUI program needs one, and only one, application object.
  // The application objects coordinates some common stuff shared between
  // all the widgets; for example, it dispatches events, keeps track of
  // all the windows, and so on.
  // We pass the "name" of the application, and its "vendor", the name
  // and vendor are used to search the registry database (which stores
  // persistent information e.g. fonts and colors).
  FXApp application("OpenTX Simu", "OpenTX");

  // Here we initialize the application.  We pass the command line arguments
  // because FOX may sometimes need to filter out some of the arguments.
  // This opens up the display as well, and reads the registry database
  // so that persistent settings are now available.
  application.init(argc, argv);

  // This creates the main window. We pass in the title to be displayed
  // above the window, and possibly some icons for when its iconified.
  // The decorations determine stuff like the borders, close buttons,
  // drag handles, and so on the Window Manager is supposed to give this
  // window.
  //FXMainWindow *main=new FXMainWindow(&application, "Hello", nullptr, nullptr, DECOR_ALL);
  opentxSim = new OpenTxSim(&application);
  application.create();

  // Pretty self-explanatory:- this shows the window, and places it in the
  // middle of the screen.
#ifndef __APPLE__
  opentxSim->show(PLACEMENT_SCREEN);
#else
  opentxSim->show(); // Otherwise the main window gets centred across my two monitors, split down the middle.
#endif


  printf("Model size = %d\n", (int)sizeof(g_model));

  simuInit();

#if defined(EEPROM) || defined(EEPROM_RLC)
  startEepromThread(argc >= 2 ? argv[1] : "eeprom.bin");
#endif

#if !defined(SIMU_BOOTLOADER)
  startAudioThread();
#endif
  simuStart(true/*false*/, argc >= 3 ? argv[2] : 0, argc >= 4 ? argv[3] : 0);

  return application.run();
}

uint16_t anaIn(uint8_t chan)
{
  if (chan < NUM_STICKS)
    return opentxSim->sliders[chan]->getValue();
  else if (chan < NUM_STICKS + NUM_POTS + NUM_SLIDERS)
    return opentxSim->knobs[chan - NUM_STICKS]->getValue();
#if defined(PCBTARANIS)
  else if (chan == TX_RTC_VOLTAGE)
    return 800; // 2.34V
#endif
  else
    return 0;
}

uint16_t getAnalogValue(uint8_t index)
{
  return anaIn(index);
}

void createBitmap(int index, uint16_t *data, int x, int y, int w, int h)
{
  opentxSim->createBitmap(index, data, x, y, w, h);
}
