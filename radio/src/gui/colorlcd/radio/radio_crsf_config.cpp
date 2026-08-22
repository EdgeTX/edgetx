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

#include "radio_crsf_config.h"

#if defined(CRSF_CONFIG_MENU)

#include "choice.h"
#include "dialog.h"
#include "edgetx.h"
#include "keyboard_base.h"
#include "numberedit.h"
#include "static.h"
#include "textedit.h"

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

// ELRS 4.1 renamed some fields with an appended state ("Band/Enabled"):
// display them without the suffix
static std::string displayName(const CrsfConfigField& f)
{
  size_t len = strlen(f.name);
  if (len > 8 && strcasecmp(f.name + len - 8, "/Enabled") == 0) len -= 8;
  return std::string(f.name, len);
}

static std::string formatFloatValue(int32_t value, uint8_t prec,
                                    const char* unit)
{
  char buf[24];
  crsfConfigFormatNumber(value, prec, unit, buf, sizeof(buf));
  return std::string(buf);
}

static std::string uidString()
{
  uint8_t uid[6];
  if (!crsfConfigGetUid(uid)) return std::string("...");
  char buf[32];
  snprintf(buf, sizeof(buf), "%u,%u,%u,%u,%u,%u", uid[0], uid[1], uid[2],
           uid[3], uid[4], uid[5]);
  return std::string(buf);
}

static const CrsfConfigDevice* findDevice(uint8_t address)
{
  for (uint8_t i = 0; i < crsfConfigDeviceCount(); i++) {
    auto d = crsfConfigGetDevice(i);
    if (d && d->address == address) return d;
  }
  return nullptr;
}

//-----------------------------------------------------------------------------

class CrsfCommandDialog : public BaseDialog
{
 public:
  CrsfCommandDialog(const char* title, std::function<void()> onClose) :
      BaseDialog(title, false), onClose(std::move(onClose))
  {
    new DynamicText(form, rect_t{}, [=] {
      if (finished) return finalMsg;
      CrsfConfigField f;
      crsfConfigGetField(crsfConfigCommandField(), f);
      if (f.loaded && f.svalue[0]) return std::string(f.svalue);
      return std::string(STR_WAITING_FOR_MODULE);
    });

    auto box = new Window(form, rect_t{});
    box->padAll(PAD_TINY);
    box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_MEDIUM, LV_SIZE_CONTENT);

    okButton = new TextButton(box, rect_t{}, STR_OK, [=]() -> uint8_t {
      crsfConfigCommandConfirm();
      return 0;
    });

    new TextButton(box, rect_t{}, STR_STOP, [=]() -> uint8_t {
      onCancel();
      return 0;
    });

    okButton->hide();
  }

  void checkEvents() override
  {
    // while a modal is open it is the only window receiving
    // checkEvents, so the protocol must be driven from here
    crsfConfigTick();

    if (!crsfConfigCommandActive()) {
      if (!finished) {
        // command completed: keep showing its final message (if any)
        // until the user closes the dialog
        CrsfConfigField f;
        crsfConfigGetField(crsfConfigCommandField(), f);
        if (f.loaded && f.svalue[0]) {
          finished = true;
          finalMsg = f.svalue;
          okButton->hide();
        } else {
          close();
          return;
        }
      }
    } else if (crsfConfigCommandStatus() == CRSF_CMD_CONFIRMATION_NEEDED) {
      okButton->show();
    } else {
      okButton->hide();
    }

    BaseDialog::checkEvents();
  }

 protected:
  TextButton* okButton;
  bool finished = false;
  std::string finalMsg;
  std::function<void()> onClose;

  void close()
  {
    if (onClose) onClose();
    deleteLater();
  }

  void onCancel() override
  {
    // back stops the command
    if (!finished) crsfConfigCommandCancel();
    close();
  }

  void onEvent(event_t event) override
  {
#if defined(HARDWARE_KEYS)
    if (event == EVT_KEY_LONG(KEY_EXIT)) {
      // holding back leaves the command (WiFi, BLE joystick, ...)
      // running in the background
      killEvents(event);
      if (!finished) crsfConfigCommandDetach();
      close();
      return;
    }
#endif
    BaseDialog::onEvent(event);
  }
};

// the topmost modal is the only window receiving checkEvents, so a
// warning shown over the config page has to keep the protocol running
class CrsfWarnDialog : public ConfirmDialog
{
 public:
  using ConfirmDialog::ConfirmDialog;

  void checkEvents() override
  {
    crsfConfigTick();
    ConfirmDialog::checkEvents();
  }
};

//-----------------------------------------------------------------------------

RadioCrsfBindPage::RadioCrsfBindPage() : Page(ICON_RADIO_TOOLS)
{
  header->setTitle(STR_MODULE_BIND);
  auto txDev = findDevice(MODULE_ADDRESS);
  if (txDev) header->setTitle2(txDev->name);

  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);
  body->padAll(PAD_SMALL);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  bool canSet = crsfConfigElrsCanSetBindInfo(txDev);

  if (canSet) {
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_CRSF_BIND_METHOD);
    new Choice(line, rect_t{}, STR_CRSF_BIND_METHODS, 0, 1,
               [=] { return (int)method; },
               [=](int32_t v) {
                 method = v;
                 updateVisibility();
               });
  } else {
    // no bind phrase support on this firmware: classic bind only
    method = 1;
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_CRSF_NEEDS_41);
  }

  // --- bind phrase (ELRS >= 4.1) ---
  phraseLine = body->newLine(grid);
  new StaticText(phraseLine, rect_t{}, STR_CRSF_BIND_PHRASE);
  new TextEdit(phraseLine, rect_t{}, phrase, CRSF_BIND_PHRASE_MAXLEN);

  applyLine = body->newLine(grid);
  new StaticText(applyLine, rect_t{}, STR_CRSF_APPLY_TO);
  new Choice(applyLine, rect_t{}, STR_CRSF_BIND_APPLY_MODES, 0, 1,
             [=] { return (int)applyMode; },
             [=](int32_t v) { applyMode = v; });

  setLine = body->newLine(grid);
  new StaticText(setLine, rect_t{}, "");
  new TextButton(setLine, rect_t{}, STR_CRSF_APPLY, [=]() -> uint8_t {
    if (crsfConfigApplyBindInfo(phrase, applyMode == 1)) {
      statusMsg = (applyMode == 1 && phrase[0]) ? STR_WAITING_FOR_RX
                                                : STR_CRSF_REBOOTING;
      awaitingRefresh = true;
    }
    return 0;
  });

  uidLine = body->newLine(grid);
  new StaticText(uidLine, rect_t{}, STR_CRSF_BIND_UID);
  new DynamicText(uidLine, rect_t{}, [] { return uidString(); });

  // --- classic bind: the short ELRS bind burst is re-triggered by the
  // protocol layer until a receiver connects ---
  classicTxLine = body->newLine(grid);
  new StaticText(classicTxLine, rect_t{}, STR_MODULE);
  bindBtn = new TextButton(classicTxLine, rect_t{}, STR_MODULE_BIND,
                           [=]() -> uint8_t {
    if (crsfConfigBindLoopActive()) {
      crsfConfigBindLoopStop();
      statusMsg.clear();
    } else {
      crsfConfigBindLoopStart();
      statusMsg = STR_MODULE_BINDING;
    }
    return 0;
  });

  classicRxLine = body->newLine(grid);
  new StaticText(classicRxLine, rect_t{}, STR_RECEIVER);
  new TextButton(classicRxLine, rect_t{}, STR_CRSF_RX_BIND, [=]() -> uint8_t {
    if (crsfConfigSendBindCommand(RECEIVER_ADDRESS))
      statusMsg = STR_WAITING_FOR_RX;
    return 0;
  });

  auto line = body->newLine(grid);
  new DynamicText(line, rect_t{}, [=] { return statusMsg; });

  if (canSet) crsfConfigRequestUid(MODULE_ADDRESS);
  updateVisibility();

  // navigation jumps (quick menu) must close the page for real
  forceCloseMsg.subscribe(Messaging::QUICK_MENU_ITEM_SELECT,
                          [=](uint32_t) { deleteLater(); });
}

void RadioCrsfBindPage::updateVisibility()
{
  bool phraseMode = (method == 0);
  phraseLine->show(phraseMode);
  applyLine->show(phraseMode);
  setLine->show(phraseMode);
  uidLine->show(phraseMode);
  classicTxLine->show(!phraseMode);
  classicRxLine->show(!phraseMode);
}

void RadioCrsfBindPage::checkEvents()
{
  crsfConfigTick();

  if (!crsfConfigActive()) {
    Page::onCancel();
    return;
  }

  // the module is back after its post-set reboot: drop the status text
  uint8_t uid[6];
  if (awaitingRefresh && crsfConfigGetUid(uid)) {
    awaitingRefresh = false;
    statusMsg.clear();
  }

  // mirror the protocol-side bind loop state
  bool loopActive = crsfConfigBindLoopActive();
  if (loopActive != loopWasActive) {
    loopWasActive = loopActive;
    bindBtn->setText(loopActive ? STR_STOP : STR_MODULE_BIND);
    if (!loopActive && crsfConfigBindLoopBound()) statusMsg = STR_CONNECTED;
  }

  Page::checkEvents();
}

void RadioCrsfBindPage::onCancel()
{
  if (crsfConfigBindLoopActive()) {
    // first back press only stops the binding loop
    crsfConfigBindLoopStop();
    statusMsg.clear();
    return;
  }
  Page::onCancel();
}

#if defined(HARDWARE_KEYS)
void RadioCrsfBindPage::doKeyShortcut(event_t event)
{
  // only an actual navigation away stops the binding loop: opening the
  // quick menu alone leaves the page (and the loop) in place
  QMPage pg = g_eeGeneral.getKeyShortcut(event);
  if (pg != QM_NONE && pg != QM_OPEN_QUICK_MENU) crsfConfigBindLoopStop();
  Page::doKeyShortcut(event);
}
#endif

//-----------------------------------------------------------------------------

RadioCrsfConfigPage::RadioCrsfConfigPage(uint8_t moduleIdx) :
    Page(ICON_RADIO_TOOLS), moduleIdx(moduleIdx)
{
  header->setTitle(STR_CRSF_CONFIG);

  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);
  body->padAll(PAD_SMALL);

  crsfConfigStart(moduleIdx);
  rebuild();

  // navigation jumps (quick menu) must close the page for real
  forceCloseMsg.subscribe(Messaging::QUICK_MENU_ITEM_SELECT, [=](uint32_t) {
    folderStack.clear();
    deleteLater();
  });
}

RadioCrsfConfigPage::~RadioCrsfConfigPage() { crsfConfigStop(); }

void RadioCrsfConfigPage::onCancel()
{
  if (!folderStack.empty()) {
    folder = folderStack.back();
    folderStack.pop_back();
    crsfConfigReloadFolder(folder);
    // back at the root: rescan the bus for devices (a receiver may
    // have connected in the meantime)
    if (folder == 0) crsfConfigRefreshDevices();
    needsRebuild = true;
    return;
  }
  Page::onCancel();
}

#if defined(HARDWARE_KEYS)
void RadioCrsfConfigPage::doKeyShortcut(event_t event)
{
  // a page shortcut closes this page through onCancel(): skip the
  // folder navigation there; opening the quick menu alone must not
  // disturb the folder position
  QMPage pg = g_eeGeneral.getKeyShortcut(event);
  if (pg != QM_NONE && pg != QM_OPEN_QUICK_MENU) folderStack.clear();
  Page::doKeyShortcut(event);
}
#endif

void RadioCrsfConfigPage::updateHeader()
{
  std::string status;

  auto dev = crsfConfigCurrentDevice();
  header->setTitle(dev ? dev->name : STR_CRSF_CONFIG);

  // transient states (ARMED, warnings) flash instead of showing solid
  bool flash = (get_tmr10ms() % 100) < 50;

  if (crsfConfigState() == CRSF_STATE_ELRS_V1) {
    status = STR_CRSF_ELRS_V1;
  } else if (!dev) {
    status = STR_WAITING_FOR_MODULE;
  } else {
    uint8_t count = crsfConfigFieldCount();
    uint8_t loaded = crsfConfigLoadedFieldCount();
    char buf[32];
    if (count > 0 && loaded < count) {
      snprintf(buf, sizeof(buf), "%s %u/%u", STR_LOADING, loaded, count);
      status = buf;
    } else if (dev->isELRS_TX) {
      const CrsfElrsStatus& es = crsfConfigElrsStatus();
      if ((es.flags & CRSF_ELRS_FLAG_WARNING_MASK) && es.flagInfo[0] &&
          flash) {
        // e.g. "Model Mismatch", alternating with the normal status
        status = es.flagInfo;
      } else if (es.goodPkts > 0) {
        // telemetry ratio : packet rate, like the ELRS Lua shows
        snprintf(buf, sizeof(buf), "%u:%uHz - ", es.badPkts, es.goodPkts);
        status = buf;
        status += (es.flags & CRSF_ELRS_FLAG_CONNECTED) ? STR_CONNECTED
                                                        : STR_NOT_CONNECTED;
        if ((es.flags & CRSF_ELRS_FLAG_ARMED) && flash) {
          status += " - ";
          status += STR_CRSF_ARMED;
        }
      }
    }
  }

  if (status != lastStatus) {
    lastStatus = status;
    header->setTitle2(status);
  }
}

void RadioCrsfConfigPage::checkEvents()
{
  crsfConfigTick();

  if (!crsfConfigActive()) {
    // module protocol changed under our feet
    Page::onCancel();
    return;
  }

  updateHeader();

  const CrsfElrsStatus& es = crsfConfigElrsStatus();
  if ((es.flags & CRSF_ELRS_FLAG_CRITICAL_MASK) && !critDialogOpen) {
    critDialogOpen = true;
    new CrsfWarnDialog(
        STR_WARNING, es.flagInfo[0] ? es.flagInfo : STR_WARNING,
        [=]() {
          crsfConfigAckCriticalFlags();
          critDialogOpen = false;
        },
        [=]() { critDialogOpen = false; });
  }

  // rebuilds are deferred to here: destroying widgets from inside
  // their own event callbacks is not safe, and neither is destroying
  // a field that is being edited or has the keyboard open
  if (needsRebuild || crsfConfigGeneration() != lastGen) {
    if (!Keyboard::keyboardWindow() &&
        !lv_group_get_editing(lv_group_get_default())) {
      needsRebuild = false;
      rebuild();
    }
  }

  Page::checkEvents();
}

void RadioCrsfConfigPage::rebuild()
{
  lastGen = crsfConfigGeneration();
  strEditCount = 0;
  body->clear();

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  if (crsfConfigState() == CRSF_STATE_ELRS_V1) {
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_CRSF_ELRS_V1);
    return;
  }

  auto dev = crsfConfigCurrentDevice();
  if (!dev) {
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_WAITING_FOR_MODULE);
    return;
  }

  // other devices on the bus: selectable at the root of the tree
  if (folder == 0 && crsfConfigDeviceCount() > 1) {
    for (uint8_t i = 0; i < crsfConfigDeviceCount(); i++) {
      auto d = crsfConfigGetDevice(i);
      if (!d || d->address == dev->address) continue;
      uint8_t addr = d->address;
      auto line = body->newLine(grid);
      new StaticText(line, rect_t{}, STR_MODULE);
      new TextButton(line, rect_t{}, d->name, [=]() -> uint8_t {
        crsfConfigSelectDevice(addr);
        folder = 0;
        folderStack.clear();
        needsRebuild = true;
        return 0;
      });
    }
  }

  // the ELRS TX module's own "Bind" command is replaced, in place, by
  // the full native bind page (bind phrase / UID and classic bind)
  uint8_t bindFieldId = crsfConfigElrsBindFieldId();

  uint8_t count = crsfConfigFieldCount();
  for (uint8_t id = 1; id <= count; id++) {
    CrsfConfigField f;
    if (!crsfConfigGetField(id, f) || !f.loaded) continue;
    if (f.hidden || f.parent != folder) continue;

    if (id == bindFieldId) {
      auto line = body->newLine(grid);
      new TextButton(line, rect_t{}, STR_MODULE_BIND, [=]() -> uint8_t {
        new RadioCrsfBindPage();
        return 0;
      });
      continue;
    }

    auto line = body->newLine(grid);

    switch (f.type) {
      case CRSF_FIELD_UINT8:
      case CRSF_FIELD_INT8:
      case CRSF_FIELD_UINT16:
      case CRSF_FIELD_INT16:
      case CRSF_FIELD_FLOAT: {
        new StaticText(line, rect_t{}, displayName(f));
        auto edit = new NumberEdit(
            line, rect_t{0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, f.min,
            f.max,
            [=] {
              CrsfConfigField v;
              crsfConfigGetField(id, v);
              return (int)v.value;
            },
            [=](int32_t value) { crsfConfigWriteValue(id, value); });
        if (f.type == CRSF_FIELD_FLOAT) {
          // fixed point value with device-supplied decimals and step
          if (f.step > 1) edit->setStep(f.step);
          uint8_t prec = f.prec;
          std::string unit(f.unit);
          edit->setDisplayHandler([=](int value) {
            return formatFloatValue(value, prec, unit.c_str());
          });
        } else if (f.unit[0]) {
          edit->setSuffix(f.unit);
        }
        break;
      }

      case CRSF_FIELD_TEXT_SELECTION: {
        new StaticText(line, rect_t{}, displayName(f));
        std::vector<std::string> values;
        uint8_t nOptions = crsfConfigOptionCount(f.options);
        char buf[32];
        for (uint8_t i = 0; i < nOptions; i++) {
          crsfConfigGetOption(f.options, i, buf, sizeof(buf));
          // e.g. ELRS "Max Power" options are bare numbers with a mW unit
          values.emplace_back(std::string(buf) + f.unit);
        }
        auto choice = new Choice(
            line, rect_t{}, values, 0, nOptions > 0 ? nOptions - 1 : 0,
            [=] {
              CrsfConfigField v;
              crsfConfigGetField(id, v);
              return (int)v.value;
            },
            [=](int32_t value) { crsfConfigWriteValue(id, value); });
        // skips empty placeholders and cross-field restrictions
        // (e.g. ELRS fan threshold vs Max Power)
        choice->setAvailableHandler([=](int value) {
          return crsfConfigSelectionAllowed(id, value);
        });
        break;
      }

      case CRSF_FIELD_STRING: {
        new StaticText(line, rect_t{}, displayName(f));
        if (f.maxStrLen > 0 && strEditCount < MAX_STR_EDITS) {
          char* buf = strEditBuf[strEditCount++];
          uint8_t cap = min<uint8_t>(f.maxStrLen, CRSF_BIND_PHRASE_MAXLEN);
          strncpy(buf, f.svalue, cap);
          buf[cap] = '\0';
          new TextEdit(line, rect_t{}, buf, cap,
                       [=]() { crsfConfigWriteString(id, buf); });
        } else {
          new DynamicText(line, rect_t{}, [=] {
            CrsfConfigField v;
            crsfConfigGetField(id, v);
            return std::string(v.svalue);
          });
        }
        break;
      }

      case CRSF_FIELD_INFO: {
        bool isVersion = crsfConfigElrsIsVersionField(f);
        new StaticText(line, rect_t{},
                       isVersion ? std::string(STR_CURRENT_VERSION)
                                 : displayName(f));
        new DynamicText(line, rect_t{}, [=] {
          CrsfConfigField v;
          crsfConfigGetField(id, v);
          if (!isVersion) return std::string(v.svalue);
          std::string s(v.name);
          if (v.svalue[0]) {
            s += " (";
            s += v.svalue;
            s += ")";
          }
          return s;
        });
        break;
      }

      case CRSF_FIELD_FOLDER: {
        std::string label = displayName(f);
        label += " >";
        new TextButton(line, rect_t{}, label, [=]() -> uint8_t {
          folderStack.push_back(folder);
          folder = id;
          crsfConfigReloadFolder(id);
          needsRebuild = true;
          return 0;
        });
        break;
      }

      case CRSF_FIELD_COMMAND: {
        new TextButton(line, rect_t{}, f.name, [=]() -> uint8_t {
          crsfConfigCommandStart(id);
          CrsfConfigField cmd;
          crsfConfigGetField(id, cmd);
          if (!cmdDialog) {
            cmdDialog = new CrsfCommandDialog(cmd.name,
                                              [=] { cmdDialog = nullptr; });
          }
          return 0;
        });
        break;
      }

      default:
        break;
    }
  }

  uint8_t loaded = crsfConfigLoadedFieldCount();
  if (count == 0 || loaded < count) {
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_LOADING);
  }
}

#endif  // defined(CRSF_CONFIG_MENU)
