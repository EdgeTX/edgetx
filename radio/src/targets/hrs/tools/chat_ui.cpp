#include "edgetx.h"
#include "chat_ui.h"

#if defined(COLORLCD) &&  defined(MODULE_XIAOZHI_CHAT)

#include "aux3_chat.h"
#include "board.h"

#include "audio.h"
#include "definitions.h"
#include "etx_lv_theme.h"
#include "mainwindow.h"
#include "sdcard.h"
#include "window.h"

#include <cstring>
#include <string>

namespace {

constexpr coord_t ICON_SIZE = 48;
constexpr coord_t ICON_MARGIN = 18;
constexpr lv_opa_t ICON_BG_OPA = LV_OPA_COVER;
constexpr lv_opa_t HALO_OPA = LV_OPA_70;
constexpr uint8_t WIFI_SCAN_WAIT_ALERT_THRESHOLD = 5;
constexpr uint8_t WIFI_SCAN_WAIT_PLAY_ID = ID_PLAY_FROM_SD_MANAGER;

enum class ChatDirective : uint8_t {
  NONE,
  CLEAR,
  ASSISTANT,
  USER,
  SYSTEM,
  /** After `Application: >>` — user speech */
  APP_USER,
  /** After `Application: <<` — assistant reply */
  APP_ASSISTANT,
};

enum class ChatVisualState : uint8_t {
  REST,
  LISTENING,
  ANSWERING,
};

static inline bool isFullwidthColonUtf8(const char* p)
{
  return (uint8_t)p[0] == 0xEF && (uint8_t)p[1] == 0xBC && (uint8_t)p[2] == 0x9A;
}

static bool asciiPrefixMatch(const char* p, const char* prefix)
{
  while (*prefix) {
    char c = *p++;
    char d = *prefix++;
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c + ('a' - 'A'));
    }
    if (d >= 'A' && d <= 'Z') {
      d = static_cast<char>(d + ('a' - 'A'));
    }
    if (c != d) {
      return false;
    }
  }
  return true;
}

static const char* skipSpace(const char* p)
{
  while (*p == ' ' || *p == '\t') {
    ++p;
  }
  return p;
}

static bool extractLeadingAsciiToken(const char* text, char* out, size_t outLen)
{
  if (!text || !out || outLen == 0) {
    return false;
  }

  const char* p = skipSpace(text);
  size_t n = 0;
  while (*p) {
    const char c = *p;
    const bool isAlphaNum =
        (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9');
    if (!isAlphaNum && c != '_' && c != '-') {
      break;
    }

    if (n + 1 < outLen) {
      out[n++] = c;
    }
    ++p;
  }

  out[n] = '\0';
  return n > 0;
}

static bool strEqualAsciiCaseFold(const char* a, const char* b)
{
  while (*a && *b) {
    char ca = *a++;
    char cb = *b++;
    if (ca >= 'A' && ca <= 'Z') {
      ca = static_cast<char>(ca + ('a' - 'A'));
    }
    if (cb >= 'A' && cb <= 'Z') {
      cb = static_cast<char>(cb + ('a' - 'A'));
    }
    if (ca != cb) {
      return false;
    }
  }
  return *a == '\0' && *b == '\0';
}

static bool asciiContains(const char* text, const char* needle)
{
  if (!text || !needle || !needle[0]) {
    return false;
  }

  const size_t textLen = strlen(text);
  const size_t needleLen = strlen(needle);
  if (textLen < needleLen) {
    return false;
  }

  for (size_t i = 0; i + needleLen <= textLen; ++i) {
    if (asciiPrefixMatch(text + i, needle)) {
      return true;
    }
  }

  return false;
}

static void normalizeLineInPlace(char* line)
{
  if (!line || !line[0]) {
    return;
  }

  if ((uint8_t)line[0] == 0xEF && (uint8_t)line[1] == 0xBB && (uint8_t)line[2] == 0xBF) {
    memmove(line, line + 3, strlen(line + 3) + 1);
  }

  size_t n = strlen(line);
  while (n > 0 && (line[n - 1] == '\r' || line[n - 1] == '\n')) {
    line[--n] = '\0';
  }
}

static bool extractStatePayload(const char* lineIn, char* out, size_t outLen)
{
  if (!lineIn || !out || outLen == 0) {
    return false;
  }

  char buf[V15_CHAT_UART_LINE_BUFFER_SIZE];
  strncpy(buf, lineIn, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';
  normalizeLineInPlace(buf);

  const char* line = skipSpace(buf);
  const size_t len = strlen(line);
  if (len < 6) {
    return false;
  }

  for (size_t i = 0; i + 5 <= len; ++i) {
    if (!asciiPrefixMatch(line + i, "state")) {
      continue;
    }

    const char* p = line + i + 5;
    p = skipSpace(p);

    if (*p == ':') {
      ++p;
    }
    else if (isFullwidthColonUtf8(p)) {
      p += 3;
    }
    else {
      continue;
    }

    p = skipSpace(p);
    strncpy(out, p, outLen - 1);
    out[outLen - 1] = '\0';
    return out[0] != '\0';
  }

  return false;
}

static bool detectVisualState(const char* payload, ChatDirective d,
                              ChatVisualState* stateOut)
{
  if (!stateOut) {
    return false;
  }

  if (d == ChatDirective::CLEAR) {
    *stateOut = ChatVisualState::REST;
    return true;
  }

  const char* p = skipSpace(payload ? payload : "");
  char stateWord[24];
  if (extractLeadingAsciiToken(p, stateWord, sizeof(stateWord))) {
    if (strEqualAsciiCaseFold(stateWord, "listening") ||
        strEqualAsciiCaseFold(stateWord, "listen")) {
      *stateOut = ChatVisualState::LISTENING;
      return true;
    }

    if (strEqualAsciiCaseFold(stateWord, "speaking") ||
        strEqualAsciiCaseFold(stateWord, "speak")) {
      *stateOut = ChatVisualState::ANSWERING;
      return true;
    }

    // idle / sleep / standby / connecting / starting → hide icon
    if (strEqualAsciiCaseFold(stateWord, "idle") ||
        strEqualAsciiCaseFold(stateWord, "sleep") ||
        strEqualAsciiCaseFold(stateWord, "sleeping") ||
        strEqualAsciiCaseFold(stateWord, "sleepy") ||
        strEqualAsciiCaseFold(stateWord, "standby") ||
        strEqualAsciiCaseFold(stateWord, "connecting") ||
        strEqualAsciiCaseFold(stateWord, "starting") ||
        strEqualAsciiCaseFold(stateWord, "configuring") ||
        strEqualAsciiCaseFold(stateWord, "unknown")) {
      *stateOut = ChatVisualState::REST;
      return true;
    }
  }

  if (asciiContains(p, "idle") || asciiContains(p, "sleep") ||
      asciiContains(p, "standby")) {
    *stateOut = ChatVisualState::REST;
    return true;
  }

  if (asciiContains(p, "listening") || asciiContains(p, "listen")) {
    *stateOut = ChatVisualState::LISTENING;
    return true;
  }

  if (asciiContains(p, "speaking") || asciiContains(p, "speak")) {
    *stateOut = ChatVisualState::ANSWERING;
    return true;
  }

  return false;
}

static bool playNoWifiAlertFile()
{
  char path[AUDIO_FILENAME_MAXLEN + 1];
  char* str = getAudioPath(path);
  strcpy(str, SYSTEM_SUBDIR "/nowifi.wav");

  STOP_PLAY(WIFI_SCAN_WAIT_PLAY_ID);
  PLAY_FILE(path, 0, WIFI_SCAN_WAIT_PLAY_ID);
  return IS_PLAYING(WIFI_SCAN_WAIT_PLAY_ID);
}
class XiaozhiChatPanel : public Window
{
 public:
  XiaozhiChatPanel() :
      Window(MainWindow::instance(),
             rect_t{LCD_W - ICON_SIZE - ICON_MARGIN,
                    LCD_H - ICON_SIZE - ICON_MARGIN - 18,
                    ICON_SIZE,
                    ICON_SIZE})
  {
    setWindowFlag(OPAQUE | IS_BUBBLE_POPUP);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_all(lvobj, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(lvobj, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(lvobj, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(lvobj, 0, LV_PART_MAIN);

    halo = lv_obj_create(lvobj);
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, ICON_SIZE, ICON_SIZE);
    lv_obj_center(halo);
    lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(halo, 2, LV_PART_MAIN);
    lv_obj_add_flag(halo, LV_OBJ_FLAG_EVENT_BUBBLE);

    spinner = lv_arc_create(lvobj);
    lv_arc_set_rotation(spinner, 270);
    lv_arc_set_bg_angles(spinner, 0, 360);
    lv_arc_set_angles(spinner, 18, 118);
    lv_obj_remove_style(spinner, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(spinner, ICON_SIZE, ICON_SIZE);
    lv_obj_center(spinner);
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(spinner, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);

    badge = lv_obj_create(lvobj);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 10, 10);
    lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, -1, 1);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 2, LV_PART_MAIN);
    lv_obj_add_flag(badge, LV_OBJ_FLAG_EVENT_BUBBLE);

    core = lv_obj_create(lvobj);
    lv_obj_remove_style_all(core);
    lv_obj_set_size(core, ICON_SIZE - 14, ICON_SIZE - 14);
    lv_obj_center(core);
    lv_obj_set_style_radius(core, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_width(core, 3, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(core, 18, LV_PART_MAIN);
    lv_obj_add_flag(core, LV_OBJ_FLAG_EVENT_BUBBLE);

    stateLabel = etx_label_create(core, FONT_XL_INDEX);
    lv_obj_align(stateLabel, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_color(stateLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_opa(stateLabel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_flag(stateLabel, LV_OBJ_FLAG_EVENT_BUBBLE);

    for (uint8_t i = 0; i < 3; ++i) {
      waveBars[i] = lv_obj_create(core);
      lv_obj_remove_style_all(waveBars[i]);
      lv_obj_set_style_radius(waveBars[i], LV_RADIUS_CIRCLE, LV_PART_MAIN);
      lv_obj_set_style_bg_color(waveBars[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(waveBars[i], LV_OPA_70, LV_PART_MAIN);
      lv_obj_add_flag(waveBars[i], LV_OBJ_FLAG_EVENT_BUBBLE);
    }
    lv_obj_set_size(waveBars[0], 4, 9);
    lv_obj_set_size(waveBars[1], 4, 14);
    lv_obj_set_size(waveBars[2], 4, 9);
    lv_obj_align(waveBars[0], LV_ALIGN_BOTTOM_MID, -8, -5);
    lv_obj_align(waveBars[1], LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_align(waveBars[2], LV_ALIGN_BOTTOM_MID, 8, -5);

    lv_obj_add_event_cb(lvobj, iconDragCb, LV_EVENT_PRESSING, this);

    setVisualState(ChatVisualState::REST, true);
    lv_obj_move_foreground(lvobj);
    lv_obj_invalidate(lvobj);
  }

  void clearLine()
  {
    setVisualState(ChatVisualState::REST, true);
  }

  void applyDirective(ChatDirective d, const char* payload, const char* body)
  {
    if (d == ChatDirective::CLEAR) {
      clearLine();
      return;
    }

    ChatVisualState nextState = visualState;
    if (detectVisualState(payload, d, &nextState)) {
      setVisualState(nextState, true);
      return;
    }

    if (d == ChatDirective::NONE) {
      return;
    }

    if (body && body[0]) {
      if (d == ChatDirective::USER || d == ChatDirective::APP_USER) {
        setVisualState(ChatVisualState::LISTENING, true);
      }
      else {
        setVisualState(ChatVisualState::ANSWERING, true);
      }
    }
  }

  void checkIdleTimeout()
  {
    if (visualState == ChatVisualState::REST) {
      return;
    }

    // Backup when STATE: idle is missed (e.g. ESP already light-sleeping).
    // Answering ends quickly; listening can idle longer before auto-standby.
    const tmr10ms_t limit =
        (visualState == ChatVisualState::ANSWERING) ? 800 : 4500;  // 8s / 45s
    if ((tmr10ms_t)(g_tmr10ms - lastActivityTick) > limit) {
      setVisualState(ChatVisualState::REST, true);
    }
  }

  ChatVisualState getVisualState() const
  {
    return visualState;
  }

 protected:
  static void zoomAnimCb(void* obj, int32_t value)
  {
    lv_obj_set_style_transform_zoom(static_cast<lv_obj_t*>(obj),
                                    static_cast<lv_coord_t>(value),
                                    LV_PART_MAIN);
  }

  static void opaAnimCb(void* obj, int32_t value)
  {
    lv_obj_set_style_bg_opa(static_cast<lv_obj_t*>(obj),
                            static_cast<lv_opa_t>(value),
                            LV_PART_MAIN);
  }

  static void angleAnimCb(void* obj, int32_t value)
  {
    lv_obj_set_style_transform_angle(static_cast<lv_obj_t*>(obj),
                                     static_cast<lv_coord_t>(value),
                                     LV_PART_MAIN);
  }

  static void startPulseAnim(lv_obj_t* obj, lv_coord_t fromZoom, lv_coord_t toZoom,
                             lv_opa_t fromOpa, lv_opa_t toOpa, uint32_t time)
  {
    lv_anim_del(obj, zoomAnimCb);
    lv_anim_del(obj, opaAnimCb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, zoomAnimCb);
    lv_anim_set_values(&a, fromZoom, toZoom);
    lv_anim_set_time(&a, time);
    lv_anim_set_playback_time(&a, time);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, opaAnimCb);
    lv_anim_set_values(&a, fromOpa, toOpa);
    lv_anim_set_time(&a, time);
    lv_anim_set_playback_time(&a, time);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }

  static void startSpinAnim(lv_obj_t* obj, int32_t fromAngle, int32_t toAngle,
                            uint32_t time)
  {
    lv_anim_del(obj, angleAnimCb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, angleAnimCb);
    lv_anim_set_values(&a, fromAngle, toAngle);
    lv_anim_set_time(&a, time);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
  }

  void setVisualState(ChatVisualState state, bool refreshTick = false)
  {
    if (refreshTick || state != ChatVisualState::REST) {
      lastActivityTick = g_tmr10ms;
    }

    if (state == visualState && !refreshTick) {
      return;
    }

    visualState = state;

    // Use Window show/hide so EdgeTX visibility tracking stays consistent.
    if (visualState == ChatVisualState::REST) {
      hide();
    }
    else {
      show();
      lv_obj_move_foreground(lvobj);
    }

    lv_color_t coreColor = lv_color_hex(0x213454);
    lv_color_t accentColor = lv_color_hex(0x9DF1FF);
    const char* labelText = " ";
    bool showSpinner = false;
    bool animateHalo = false;
    bool showBadge = false;
    bool animateBadge = false;
    bool showWave = false;
    bool animateWave = false;
    uint32_t pulseTime = 1600;
    uint32_t spinTime = 0;
    lv_coord_t haloToZoom = 280;
    lv_opa_t haloToOpa = LV_OPA_50;
    lv_coord_t haloBorderWidth = 2;
    lv_coord_t spinnerWidth = 4;
    lv_opa_t badgeOpa = LV_OPA_TRANSP;
    uint16_t arcStart = 18;
    uint16_t arcEnd = 118;
    lv_coord_t waveHeights[3] = {5, 8, 5};
    lv_opa_t waveOpa = LV_OPA_70;

    switch (visualState) {
      case ChatVisualState::REST:
        coreColor = lv_color_hex(0x1C2C48);
        accentColor = lv_color_hex(0x8DEBFF);
        break;

      case ChatVisualState::LISTENING:
        coreColor = lv_color_hex(0x1428A0);
        accentColor = lv_color_hex(0x00F6FF);
        labelText = " ";
        showSpinner = true;
        animateHalo = false;
        showBadge = true;
        pulseTime = 1700;
        spinTime = 820;
        haloToZoom = 256;
        haloToOpa = LV_OPA_COVER;
        haloBorderWidth = 5;
        spinnerWidth = 8;
        badgeOpa = LV_OPA_COVER;
        arcStart = 10;
        arcEnd = 138;
        break;

      case ChatVisualState::ANSWERING:
        coreColor = lv_color_hex(0xB83800);
        accentColor = lv_color_hex(0xFFD54A);
        labelText = " ";
        showSpinner = true;
        animateHalo = false;
        showBadge = true;
        pulseTime = 1700;
        spinTime = 760;
        haloToZoom = 256;
        haloToOpa = LV_OPA_COVER;
        haloBorderWidth = 5;
        spinnerWidth = 8;
        badgeOpa = LV_OPA_COVER;
        arcStart = 24;
        arcEnd = 220;
        break;
    }

    lv_obj_set_style_bg_color(halo, accentColor, LV_PART_MAIN);
    lv_obj_set_style_border_color(halo, accentColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(halo, haloBorderWidth, LV_PART_MAIN);
    lv_obj_set_style_border_opa(halo, showSpinner ? LV_OPA_COVER : LV_OPA_0,
                                LV_PART_MAIN);
    lv_obj_set_style_bg_opa(halo, showSpinner ? LV_OPA_20 : LV_OPA_0, LV_PART_MAIN);

    lv_obj_set_style_bg_color(core, coreColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(core, ICON_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_border_color(core, accentColor, LV_PART_MAIN);
    lv_obj_set_style_border_width(core, showSpinner ? 4 : 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(core, accentColor, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(core, showSpinner ? 22 : 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(core, showSpinner ? LV_OPA_70 : LV_OPA_20,
                                LV_PART_MAIN);

    lv_obj_set_style_bg_color(badge, accentColor, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, badgeOpa, LV_PART_MAIN);
    lv_obj_set_style_border_color(badge, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_shadow_color(badge, accentColor, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(badge, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(badge, LV_OPA_40, LV_PART_MAIN);
    if (showBadge) {
      lv_obj_clear_flag(badge, LV_OBJ_FLAG_HIDDEN);
    }
    else {
      lv_obj_add_flag(badge, LV_OBJ_FLAG_HIDDEN);
    }

    lv_arc_set_angles(spinner, arcStart, arcEnd);
    lv_obj_set_style_arc_width(spinner, spinnerWidth, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, spinnerWidth, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, accentColor, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(spinner, showSpinner ? LV_OPA_90 : LV_OPA_0,
                             LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(spinner, showSpinner ? LV_OPA_COVER : LV_OPA_0,
                             LV_PART_INDICATOR);

    if (showSpinner) {
      lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN);
      startSpinAnim(spinner, 0, 3600, spinTime);
    }
    else {
      lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
      lv_anim_del(spinner, angleAnimCb);
      lv_obj_set_style_transform_angle(spinner, 0, LV_PART_MAIN);
    }

    lv_label_set_text(stateLabel, labelText);

    for (uint8_t i = 0; i < 3; ++i) {
      lv_obj_set_size(waveBars[i], 4, waveHeights[i]);
      lv_obj_set_style_bg_color(waveBars[i], accentColor, LV_PART_MAIN);
      lv_obj_set_style_bg_opa(waveBars[i], waveOpa, LV_PART_MAIN);
      if (showWave) {
        lv_obj_clear_flag(waveBars[i], LV_OBJ_FLAG_HIDDEN);
      }
      else {
        lv_obj_add_flag(waveBars[i], LV_OBJ_FLAG_HIDDEN);
      }
    }

    if (animateHalo) {
      startPulseAnim(halo, 256, haloToZoom, LV_OPA_20, haloToOpa, pulseTime);
    }
    else {
      lv_anim_del(halo, zoomAnimCb);
      lv_anim_del(halo, opaAnimCb);
      lv_obj_set_style_transform_zoom(halo, showSpinner ? haloToZoom : 256,
                                      LV_PART_MAIN);
      lv_obj_set_style_bg_opa(halo, showSpinner ? LV_OPA_20 : LV_OPA_0, LV_PART_MAIN);
    }

    if (animateBadge) {
      startPulseAnim(badge, 256, 320, badgeOpa, LV_OPA_COVER, pulseTime / 2);
    }
    else {
      lv_anim_del(badge, zoomAnimCb);
      lv_anim_del(badge, opaAnimCb);
      lv_obj_set_style_transform_zoom(badge, 256, LV_PART_MAIN);
      lv_obj_set_style_bg_opa(badge, badgeOpa, LV_PART_MAIN);
    }

    if (animateWave) {
      startPulseAnim(waveBars[0], 256, 300, waveOpa, LV_OPA_COVER, pulseTime);
      startPulseAnim(waveBars[1], 256, 320, waveOpa, LV_OPA_COVER,
                     pulseTime + 100);
      startPulseAnim(waveBars[2], 256, 300, waveOpa, LV_OPA_COVER,
                     pulseTime - 80);
    }
    else {
      for (uint8_t i = 0; i < 3; ++i) {
        lv_anim_del(waveBars[i], zoomAnimCb);
        lv_anim_del(waveBars[i], opaAnimCb);
        lv_obj_set_style_transform_zoom(waveBars[i], 256, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(waveBars[i], waveOpa, LV_PART_MAIN);
      }
    }

    lv_anim_del(core, zoomAnimCb);
    lv_anim_del(core, opaAnimCb);
    lv_obj_set_style_transform_zoom(core, 256, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(core, ICON_BG_OPA, LV_PART_MAIN);
    lv_obj_invalidate(lvobj);
  }

  static void iconDragCb(lv_event_t* e)
  {
    if (lv_event_get_code(e) != LV_EVENT_PRESSING) {
      return;
    }

    auto* self = static_cast<XiaozhiChatPanel*>(lv_event_get_user_data(e));
    if (!self) {
      return;
    }

    lv_indev_t* indev = lv_indev_get_act();
    if (!indev) {
      return;
    }

    lv_point_t vect{};
    lv_indev_get_vect(indev, &vect);
    if (vect.x == 0 && vect.y == 0) {
      return;
    }

    lv_obj_t* root = self->getLvObj();
    lv_coord_t nx = lv_obj_get_x(root) + vect.x;
    lv_coord_t ny = lv_obj_get_y(root) + vect.y;

    const lv_coord_t w = lv_obj_get_width(root);
    const lv_coord_t h = lv_obj_get_height(root);
    if (nx < 0) {
      nx = 0;
    }
    if (ny < 0) {
      ny = 0;
    }
    if (nx + w > LCD_W) {
      nx = LCD_W - w;
    }
    if (ny + h > LCD_H) {
      ny = LCD_H - h;
    }

    self->setPos(nx, ny);
  }

  lv_obj_t* halo = nullptr;
  lv_obj_t* spinner = nullptr;
  lv_obj_t* badge = nullptr;
  lv_obj_t* core = nullptr;
  lv_obj_t* stateLabel = nullptr;
  lv_obj_t* waveBars[3] = {nullptr, nullptr, nullptr};
  ChatVisualState visualState = ChatVisualState::REST;
  tmr10ms_t lastActivityTick = 0;
};

static XiaozhiChatPanel* s_chatPanel = nullptr;
static uint8_t s_wifiScanWaitConsecutiveCount = 0;
// Defer panel create a few menus ticks after AI On so Pairing→On RX flush
// settles before heavy LVGL + UART poll start (avoids WDT on return to main).
static uint8_t s_chatCreateDelayTicks = 0;
static constexpr uint8_t CHAT_CREATE_DELAY_TICKS = 8;

}  // namespace

void v15ChatUiTask(void)
{
  if (s_chatPanel && s_chatPanel->deleted()) {
    s_chatPanel = nullptr;
  }

  const uint8_t mode = v15AiModeGet();
  // Only AI On (1) owns the floating chat UI + UART line parser.
  // Pairing (2) / Boot (3): UART is owned by ext_senser_driver; creating the
  // chat panel while returning to the main menu races log floods and can WDT
  // into EMERGENCY MODE. Off (0): no panel.
  if (mode != 1) {
    if (s_chatPanel) {
      s_chatPanel->closeWindow();
      s_chatPanel = nullptr;
    }
    s_wifiScanWaitConsecutiveCount = 0;
    s_chatCreateDelayTicks = 0;
    return;
  }

  if (!s_chatPanel) {
    if (s_chatCreateDelayTicks == 0) {
      if (v15ChatUartIsReady()) {
        v15ChatUartClearRxBuffer();
      }
      s_chatCreateDelayTicks = CHAT_CREATE_DELAY_TICKS;
      return;
    }
    if (s_chatCreateDelayTicks > 1) {
      --s_chatCreateDelayTicks;
      return;
    }
    s_chatCreateDelayTicks = 0;
    if (MainWindow::instance()) {
      s_chatPanel = new XiaozhiChatPanel();
      if (s_chatPanel) {
        s_chatPanel->onClosing([]() { s_chatPanel = nullptr; });
      }
    }
  }

  if (s_chatPanel) {
    s_chatPanel->checkIdleTimeout();
  }

  v15ChatUartPollLine();

  if (v15ChatUartTakeWifiScanWaitAlert()) {
    if (s_wifiScanWaitConsecutiveCount < 255) {
      ++s_wifiScanWaitConsecutiveCount;
    }
    if (s_wifiScanWaitConsecutiveCount >= WIFI_SCAN_WAIT_ALERT_THRESHOLD) {
      playNoWifiAlertFile();
      s_wifiScanWaitConsecutiveCount = 0;
    }
  }

  if (!v15ChatUartHasNewLine()) {
    return;
  }

  char line[V15_CHAT_UART_LINE_BUFFER_SIZE];
  if (!v15ChatUartFetchLatestLine(line, sizeof(line), true)) {
    return;
  }

  // A real device STATE: line means WiFi-scan spam is not the stuck-icon cause.
  s_wifiScanWaitConsecutiveCount = 0;

  char payload[V15_CHAT_UART_LINE_BUFFER_SIZE];
  if (!extractStatePayload(line, payload, sizeof(payload)) || !s_chatPanel) {
    return;
  }

  s_chatPanel->applyDirective(ChatDirective::NONE, payload, nullptr);
}

uint8_t v15ChatUiGetVisualState(void)
{
  if (s_chatPanel) {
    return static_cast<uint8_t>(s_chatPanel->getVisualState());
  }
  return V15_CHAT_UI_STATE_REST;
}
#endif
