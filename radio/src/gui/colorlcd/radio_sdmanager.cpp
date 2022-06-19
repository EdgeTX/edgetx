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

#include "radio_sdmanager.h"
#include "opentx.h"
#include "libopenui.h"
#include "gui/colorlcd/LvglWrapper.h"
#include "io/frsky_firmware_update.h"
#include "io/multi_firmware_update.h"
#include "io/bootloader_flash.h"
#include "standalone_lua.h"
#include "sdcard.h"
#include "view_text.h"
#include "file_preview.h"
#include "file_browser.h"

constexpr int WARN_FILE_LENGTH = 40 * 1024;

#define CELL_CTRL_DIR  LV_TABLE_CELL_CTRL_CUSTOM_1
#define CELL_CTRL_FILE LV_TABLE_CELL_CTRL_CUSTOM_2

class FileNameEditWindow : public Page
{
  public:
  FileNameEditWindow(const std::string iName) :
      Page(ICON_RADIO_SD_MANAGER), name(std::move(iName))
  {
    buildHeader(&header);
    buildBody(&body);
  };

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FileNameEditWindow"; }
#endif
  protected:
  const std::string name;

  void buildHeader(Window *window)
  {
    new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_RENAME_FILE, 0, COLOR_THEME_PRIMARY2);
  }

  void buildBody(Window *window)
  {
    GridLayout grid(window);
    grid.spacer(8);
    uint8_t nameLength;
    uint8_t extLength;
    char extension[LEN_FILE_EXTENSION_MAX + 1];
    memset(extension, 0, sizeof(extension));
    const char *ext =
        getFileExtension(name.c_str(), 0, 0, &nameLength, &extLength);

    if (extLength > LEN_FILE_EXTENSION_MAX) extLength = LEN_FILE_EXTENSION_MAX;
    if (ext) strncpy(extension, ext, extLength);

    const uint8_t maxNameLength = SD_SCREEN_FILE_LENGTH - extLength;
    nameLength -= extLength;
    if (nameLength > maxNameLength) nameLength = maxNameLength;
    memset(reusableBuffer.sdManager.originalName, 0, SD_SCREEN_FILE_LENGTH);

    strncpy(reusableBuffer.sdManager.originalName, name.c_str(), nameLength);
    reusableBuffer.sdManager.originalName[nameLength] = '\0';

    auto newFileName = new TextEdit(
        window, grid.getSlot(), reusableBuffer.sdManager.originalName,
        SD_SCREEN_FILE_LENGTH - extLength, LcdFlags(0));
    newFileName->setChangeHandler([=]() {
      char *newValue = reusableBuffer.sdManager.originalName;
      size_t totalSize = strlen(newValue);
      char changedName[SD_SCREEN_FILE_LENGTH + 1];
      memset(changedName, 0, sizeof(changedName));
      strncpy(changedName, newValue, totalSize);
      changedName[totalSize] = '\0';
      if (extLength) {
        strncpy(changedName + totalSize, extension, extLength);
      }
      changedName[totalSize + extLength] = '\0';
      f_rename((const TCHAR *)name.c_str(), (const TCHAR *)changedName);
    });
  };
};

RadioSdManagerPage::RadioSdManagerPage() :
  PageTab(SD_IS_HC() ? STR_SDHC_CARD : STR_SD_CARD, ICON_RADIO_SD_MANAGER)
{
}

template <class T>
class FlashDialog: public FullScreenDialog
{
  public:
    explicit FlashDialog(const T & device):
      FullScreenDialog(WARNING_TYPE_INFO, STR_FLASH_DEVICE),
      device(device),
      progress(this, {LCD_W / 2 - 50, LCD_H / 2, 100, 15})
    {
    }

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_deleted)
        return;

      progress.deleteLater(true, false);
      FullScreenDialog::deleteLater(detach, trash);
    }

    void flash(const char * filename)
    {
      TRACE("flashing '%s'", filename);
      device.flashFirmware(
          filename,
          [=](const char *title, const char *message, int count,
              int total) -> void {
            setMessage(message);
            progress.setValue(total > 0 ? count * 100 / total : 0);
            lv_refr_now(nullptr);
          });
      deleteLater();
    }

  protected:
    T device;
    Progress progress;
};

#if LCD_W > LCD_H // landscape
static const lv_coord_t col_dsc[] = {LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#else // portrait
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#endif

void RadioSdManagerPage::build(FormWindow * window)
{
  FlexGridLayout grid(col_dsc, row_dsc, 0);
  window->padAll(0);
  
  FormGroup* form = new FormGroup(window, rect_t{});
  form->setWidth(window->width());
  form->setHeight(window->height());
  grid.apply(form);
  
  browser = new FileBrowser(form, rect_t{}, ROOT_PATH);
  grid.add(browser);
  grid.nextCell();

  auto obj = browser->getLvObj();
  lv_obj_set_style_grid_cell_x_align(obj, LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_grid_cell_y_align(obj, LV_GRID_ALIGN_STRETCH, 0);

  // Adjust file browser width
  browser->adjustWidth();
  
  preview = new FilePreview(form, rect_t{});
  grid.add(preview);
  grid.nextCell();

  obj = preview->getLvObj();
  lv_obj_set_style_pad_all(obj, lv_dpx(8), 0);
  lv_obj_set_style_grid_cell_x_align(obj, LV_GRID_ALIGN_STRETCH, 0);
  lv_obj_set_style_grid_cell_y_align(obj, LV_GRID_ALIGN_STRETCH, 0);

  browser->setFileAction([=](const char* path, const char* name, const char* fullpath) {
      fileAction(path, name, fullpath);
  });
  browser->setFileSelected([=](const char* path, const char* name, const char* fullpath) {
      preview->setFile(fullpath);
  });
  browser->refresh();
}

void RadioSdManagerPage::fileAction(const char* path, const char* name,
                                    const char* fullpath)
{
  auto window = Layer::back();
  auto menu = new Menu(window);
  const char* ext = getFileExtension(name);
  if (ext) {
    if (!strcasecmp(ext, SOUNDS_EXT)) {
      menu->addLine(STR_PLAY_FILE, [=]() {
        audioQueue.stopAll();
        audioQueue.playFile(fullpath, 0, ID_PLAY_FROM_SD_MANAGER);
      });
    }
#if defined(MULTIMODULE) && !defined(DISABLE_MULTI_UPDATE)
    if (!READ_ONLY() && !strcasecmp(ext, MULTI_FIRMWARE_EXT)) {
      MultiFirmwareInformation information;
      if (information.readMultiFirmwareInformation(fullpath) == nullptr) {
#if defined(INTERNAL_MODULE_MULTI)
        menu->addLine(STR_FLASH_INTERNAL_MULTI, [=]() {
          MultiFirmwareUpdate(fullpath, INTERNAL_MODULE,
                              MULTI_TYPE_MULTIMODULE);
        });
#endif
        menu->addLine(STR_FLASH_EXTERNAL_MULTI, [=]() {
          MultiFirmwareUpdate(fullpath, EXTERNAL_MODULE,
                              MULTI_TYPE_MULTIMODULE);
        });
      }
    }
#endif
    else if (!READ_ONLY() && !strcasecmp(ext, ELRS_FIRMWARE_EXT)) {
      menu->addLine(STR_FLASH_EXTERNAL_ELRS, [=]() {
        MultiFirmwareUpdate(fullpath, EXTERNAL_MODULE, MULTI_TYPE_ELRS);
      });
    } else if (!strcasecmp(BITMAPS_PATH, path) &&
               isExtensionMatching(ext, BITMAPS_EXT)) {
      menu->addLine(STR_ASSIGN_BITMAP, [=]() {
        memcpy(g_model.header.bitmap, name, sizeof(g_model.header.bitmap));
        storageDirty(EE_MODEL);
      });
    } else if (!strcasecmp(ext, TEXT_EXT) || !strcasecmp(ext, LOGS_EXT)) {
      menu->addLine(STR_VIEW_TEXT, [=]() {
        FIL file;
        if (FR_OK == f_open(&file, fullpath, FA_OPEN_EXISTING | FA_READ)) {
          const int fileLength = file.obj.objsize;
          f_close(&file);

          if (fileLength > WARN_FILE_LENGTH) {
            char buf[64];
            sprintf(buf, " %s %dkB. %s", STR_FILE_SIZE, fileLength / 1024,
                    STR_FILE_OPEN);
            new ConfirmDialog(window, STR_WARNING, buf,
                              [=] { new ViewTextWindow(path, name); });
          } else {
            new ViewTextWindow(path, name);
          }
        }
      });
    }
    if (!READ_ONLY() && !strcasecmp(ext, FIRMWARE_EXT)) {
      if (isBootloader(fullpath)) {
        menu->addLine(STR_FLASH_BOOTLOADER,
                      [=]() { BootloaderUpdate(fullpath); });
      }
    } else if (!READ_ONLY() && !strcasecmp(ext, SPORT_FIRMWARE_EXT)) {
      if (HAS_SPORT_UPDATE_CONNECTOR()) {
        menu->addLine(STR_FLASH_EXTERNAL_DEVICE,
                      [=]() { FrSkyFirmwareUpdate(fullpath, SPORT_MODULE); });
      }
      menu->addLine(STR_FLASH_INTERNAL_MODULE,
                    [=]() { FrSkyFirmwareUpdate(fullpath, INTERNAL_MODULE); });
      menu->addLine(STR_FLASH_EXTERNAL_MODULE,
                    [=]() { FrSkyFirmwareUpdate(fullpath, EXTERNAL_MODULE); });
    } else if (!READ_ONLY() && !strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
      FrSkyFirmwareInformation information;
      if (readFrSkyFirmwareInformation(fullpath, information) ==
          nullptr) {
#if defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)
        menu->addLine(STR_FLASH_INTERNAL_MODULE, [=]() {
          FrSkyFirmwareUpdate(fullpath, INTERNAL_MODULE);
        });
#endif
        if (information.productFamily == FIRMWARE_FAMILY_EXTERNAL_MODULE) {
          menu->addLine(STR_FLASH_EXTERNAL_MODULE, [=]() {
            FrSkyFirmwareUpdate(fullpath, EXTERNAL_MODULE);
          });
        }
        if (information.productFamily == FIRMWARE_FAMILY_RECEIVER ||
            information.productFamily == FIRMWARE_FAMILY_SENSOR) {
          if (HAS_SPORT_UPDATE_CONNECTOR()) {
            menu->addLine(STR_FLASH_EXTERNAL_DEVICE, [=]() {
              FrSkyFirmwareUpdate(fullpath, SPORT_MODULE);
            });
          } else {
            menu->addLine(STR_FLASH_EXTERNAL_MODULE, [=]() {
              FrSkyFirmwareUpdate(fullpath, EXTERNAL_MODULE);
            });
          }
        }
#if _NYI_  // Not yet implemented
#if defined(PXX2)
        if (information.productFamily == FIRMWARE_FAMILY_RECEIVER) {
          if (isReceiverOTAEnabledFromModule(INTERNAL_MODULE,
                                             information.productId))
            menu->addLine(STR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA, [=]() {
              FrSkyFirmwareUpdate(fullpath, INTERNAL_MODULE_OTA);
            });
          if (isReceiverOTAEnabledFromModule(EXTERNAL_MODULE,
                                             information.productId))
            menu->addLine(STR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA, [=]() {
              FrSkyFirmwareUpdate(fullpath, EXTERNAL_MODULE_OTA);
            });
        }
        if (information.productFamily == FIRMWARE_FAMILY_FLIGHT_CONTROLLER) {
          menu->addLine(
              STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA, [=]() {
                FrSkyFirmwareUpdate(
                    fullpath,
                    STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA);
              });
          menu->addLine(
              STR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA, [=]() {
                FrSkyFirmwareUpdate(
                    fullpath,
                    STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA);
              });
        }
#endif
#if defined(BLUETOOTH)
        if (information.productFamily == FIRMWARE_FAMILY_BLUETOOTH_CHIP) {
          menu->addLine(STR_FLASH_BLUETOOTH_MODULE, [=]() {
            FrSkyFirmwareUpdate(fullpath, STR_FLASH_BLUETOOTH_MODULE);
          });
        }
#endif
#if defined(HARDWARE_POWER_MANAGEMENT_UNIT)
        if (information.productFamily ==
            FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT) {
          menu->addLine(STR_FLASH_POWER_MANAGEMENT_UNIT, [=]() {
            FrSkyFirmwareUpdate(fullpath, STR_FLASH_POWER_MANAGEMENT_UNIT);
          });
        }
#endif
#endif  // _NYI_
      }
    }
#if defined(LUA)
    else if (isExtensionMatching(ext, SCRIPTS_EXT)) {
      menu->addLine(STR_EXECUTE_FILE, [=]() {
        luaExec(fullpath);
        StandaloneLuaWindow::instance()->attach();
      });
    }
#endif
  }
  if (!READ_ONLY()) {
    menu->addLine(STR_COPY_FILE, [=]() {
      clipboard.type = CLIPBOARD_TYPE_SD_FILE;
      f_getcwd(clipboard.data.sd.directory, CLIPBOARD_PATH_LEN);
      strncpy(clipboard.data.sd.filename, name, CLIPBOARD_PATH_LEN - 1);
    });
    if (clipboard.type == CLIPBOARD_TYPE_SD_FILE) {
      menu->addLine(STR_PASTE, [=]() {
        static char lfn[FF_MAX_LFN + 1];  // TODO optimize that!
        f_getcwd((TCHAR*)lfn, FF_MAX_LFN);
        // prevent copying to the same directory with the same name
        char* destNamePtr = clipboard.data.sd.filename;
        if (!strcmp(clipboard.data.sd.directory, lfn)) {
          char destFileName[2 * CLIPBOARD_PATH_LEN + 1];
          destNamePtr =
              strAppend(destFileName, FILE_COPY_PREFIX, CLIPBOARD_PATH_LEN);
          destNamePtr = strAppend(destNamePtr, clipboard.data.sd.filename,
                                  CLIPBOARD_PATH_LEN);
          destNamePtr = destFileName;
        }
        sdCopyFile(clipboard.data.sd.filename, clipboard.data.sd.directory,
                   destNamePtr, lfn);
        clipboard.type = CLIPBOARD_TYPE_NONE;

        browser->refresh();
      });
    }
    menu->addLine(STR_RENAME_FILE, [=]() {
      auto few = new FileNameEditWindow(name);
      few->setCloseHandler([=]() { browser->refresh(); });
    });
    menu->addLine(STR_DELETE_FILE, [=]() {
      f_unlink(fullpath);
      browser->refresh();
    });
  }
}

void RadioSdManagerPage::BootloaderUpdate(const char* fn)
{
  BootloaderFirmwareUpdate bootloaderFirmwareUpdate;
  auto dialog =
      new FlashDialog<BootloaderFirmwareUpdate>(bootloaderFirmwareUpdate);
  dialog->flash(fn);
}

void RadioSdManagerPage::FrSkyFirmwareUpdate(const char* fn,
                                             ModuleIndex module)
{
  FrskyDeviceFirmwareUpdate deviceFirmwareUpdate(module);
  auto dialog =
      new FlashDialog<FrskyDeviceFirmwareUpdate>(deviceFirmwareUpdate);
  dialog->flash(fn);
}

void RadioSdManagerPage::MultiFirmwareUpdate(const char* fn,
                                             ModuleIndex module,
                                             MultiModuleType type)
{
  MultiDeviceFirmwareUpdate deviceFirmwareUpdate(module, type);
  auto dialog =
      new FlashDialog<MultiDeviceFirmwareUpdate>(deviceFirmwareUpdate);
  dialog->flash(fn);
}
