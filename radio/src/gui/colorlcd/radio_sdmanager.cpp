/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "io/frsky_firmware_update.h"
#include "io/multi_firmware_update.h"
#include "io/bootloader_flash.h"
#include "standalone_lua.h"

RadioSdManagerPage::RadioSdManagerPage() :
  PageTab(SD_IS_HC() ? STR_SDHC_CARD : STR_SD_CARD, ICON_RADIO_SD_MANAGER)
{
}

// TODO elsewhere
extern bool compare_nocase(const std::string &first, const std::string &second);


char * getFullPath(const std::string &filename)
{
  static char full_path[FF_MAX_LFN + 1]; // TODO optimize that!
  f_getcwd(full_path, FF_MAX_LFN);
  strcat(full_path, "/");
  strcat(full_path, filename.c_str());
  return full_path;
}

class FilePreview : public Window
{
  public:
    FilePreview(Window *parent, const rect_t &rect) :
            Window(parent, rect, NO_SCROLLBAR)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FilePreview";
    }
#endif

    void setFile(const char *filename)
    {
      delete bitmap;
      const char *ext = getFileExtension(filename);
      if (ext && isExtensionMatching(ext, BITMAPS_EXT)) {
        bitmap = BitmapBuffer::loadBitmap(filename);
      } else {
        bitmap = nullptr;
      }
      invalidate();
    }

    void paint(BitmapBuffer *dc) override
    {
      coord_t y = parent->getScrollPositionY() + 2;
      coord_t h = MENU_BODY_HEIGHT - 4;
      lcd->drawSolidFilledRect(0, y, width(), h, DISABLE_COLOR);
      if (bitmap) {
        coord_t bitmapHeight = min<coord_t>(h, bitmap->height());
        coord_t bitmapWidth = min<coord_t>(width(), bitmap->width());
        dc->drawScaledBitmap(bitmap, (width() - bitmapWidth) / 2, y + (h - bitmapHeight) / 2, bitmapWidth,
                             bitmapHeight);
      }
    }

  protected:
    BitmapBuffer *bitmap = nullptr;
};

template <class T>
class FlashDialog: public FullScreenDialog
{
  public:
    explicit FlashDialog(const T & device):
      FullScreenDialog(WARNING_TYPE_INFO, "Flash device"),
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
      device.flashFirmware(filename, [=](const char * title, const char * message, int count, int total) -> void {
          setMessage(message);
          progress.setValue(total > 0 ? count * 100 / total : 0);
          MainWindow::instance()->run(false);
      });
      deleteLater();
    }

  protected:
    T device;
    Progress progress;
};

void RadioSdManagerPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  FILINFO fno;
  DIR dir;
  std::list<std::string> files;
  std::list<std::string> directories;

  auto preview = new FilePreview(window, {LCD_W / 2 + 6, 0, LCD_W / 2 - 16, window->height()});

  FRESULT res = f_opendir(&dir, "."); // Open the directory
  if (res == FR_OK) {
    // read all entries
    bool firstTime = true;
    for (;;) {
      res = sdReadDir(&dir, &fno, firstTime);

      if (res != FR_OK || fno.fname[0] == 0)
        break; // Break on error or end of dir
      if (strlen(fno.fname) > SD_SCREEN_FILE_LENGTH)
        continue;
      if (fno.fname[0] == '.' && fno.fname[1] != '.')
        continue; // Ignore hidden files under UNIX, but not ..

      if (fno.fattrib & AM_DIR) {
        directories.push_back(fno.fname);
      } else {
        files.push_back(fno.fname);
      }
    }

    // sort directories and files
    directories.sort(compare_nocase);
    files.sort(compare_nocase);

    for (auto name: directories) {
      new TextButton(window, grid.getLabelSlot(), name, [=]() -> uint8_t {
          f_chdir(name.data());
          window->clear();
          build(window);
          return 0;
      }, 0);
      grid.nextLine();
    }

    for (auto name: files) {
      auto button = new TextButton(window, grid.getLabelSlot(), name, [=]() -> uint8_t {
          auto menu = new Menu(window);
          const char *ext = getFileExtension(name.data());
          if (ext) {
            if (!strcasecmp(ext, SOUNDS_EXT)) {
              menu->addLine(STR_PLAY_FILE, [=]() {
                  audioQueue.stopAll();
                  audioQueue.playFile(getFullPath(name), 0, ID_PLAY_FROM_SD_MANAGER);
              });
            }
#if defined(MULTIMODULE) && !defined(DISABLE_MULTI_UPDATE)
            if (!READ_ONLY() && !strcasecmp(ext, MULTI_FIRMWARE_EXT)) {
              MultiFirmwareInformation information;
              if (information.readMultiFirmwareInformation(name.data()) == nullptr) {
#if defined(INTERNAL_MODULE_MULTI)
                menu->addLine(STR_FLASH_INTERNAL_MULTI, [=]() {
                    MultiDeviceFirmwareUpdate deviceFirmwareUpdate(INTERNAL_MODULE, MULTI_TYPE_MULTIMODULE);
                    auto dialog = new FlashDialog<MultiDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                    dialog->flash(getFullPath(name));
                });
#endif
                menu->addLine(STR_FLASH_EXTERNAL_MULTI, [=]() {
                    MultiDeviceFirmwareUpdate deviceFirmwareUpdate(EXTERNAL_MODULE, MULTI_TYPE_MULTIMODULE);
                    auto dialog = new FlashDialog<MultiDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                    dialog->flash(getFullPath(name));
                });
              }
            }
#endif
            else if (!READ_ONLY() && !strcasecmp(ext, ELRS_FIRMWARE_EXT)) {
              menu->addLine(STR_FLASH_EXTERNAL_ELRS, [=]() {
                  MultiDeviceFirmwareUpdate deviceFirmwareUpdate(EXTERNAL_MODULE, MULTI_TYPE_ELRS);
                  auto dialog = new FlashDialog<MultiDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                  dialog->flash(getFullPath(name));
              });
            }
            else if (isExtensionMatching(ext, BITMAPS_EXT)) {
              // TODO
            }
            else if (!strcasecmp(ext, TEXT_EXT)) {
              menu->addLine(STR_VIEW_TEXT, [=]() {
                  // TODO
              });
            }
            if (!READ_ONLY() && !strcasecmp(ext, FIRMWARE_EXT)) {
              if (isBootloader(name.data())) {
                menu->addLine(STR_FLASH_BOOTLOADER, [=]() {
                    BootloaderFirmwareUpdate bootloaderFirmwareUpdate;
                    auto dialog = new FlashDialog<BootloaderFirmwareUpdate>(bootloaderFirmwareUpdate);
                    dialog->flash(getFullPath(name));
                });
              }
            }
            else if (!READ_ONLY() && !strcasecmp(ext, SPORT_FIRMWARE_EXT)) {
              if (HAS_SPORT_UPDATE_CONNECTOR()) {
                menu->addLine(STR_FLASH_EXTERNAL_DEVICE, [=]() {
                    FrskyDeviceFirmwareUpdate deviceFirmwareUpdate(SPORT_MODULE);
                    auto dialog = new FlashDialog<FrskyDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                    dialog->flash(getFullPath(name));
                });
              }
              menu->addLine(STR_FLASH_INTERNAL_MODULE, [=]() {
                  FrskyDeviceFirmwareUpdate deviceFirmwareUpdate(INTERNAL_MODULE);
                  auto dialog = new FlashDialog<FrskyDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                  dialog->flash(getFullPath(name));
              });
              menu->addLine(STR_FLASH_EXTERNAL_MODULE, [=]() {
                  FrskyDeviceFirmwareUpdate deviceFirmwareUpdate(EXTERNAL_MODULE);
                  auto dialog = new FlashDialog<FrskyDeviceFirmwareUpdate>(deviceFirmwareUpdate);
                  dialog->flash(getFullPath(name));
              });
            }
#if defined(LUA)
            else if (isExtensionMatching(ext, SCRIPTS_EXT)) {
              menu->addLine(STR_EXECUTE_FILE, [=]() {
                luaExec(getFullPath(name));
                //TODO: check 'luaState'
                new StandaloneLuaWindow(window);
              });
            }
#endif
          }
          if (!READ_ONLY()) {
            menu->addLine(STR_COPY_FILE, [=]() {
                clipboard.type = CLIPBOARD_TYPE_SD_FILE;
                f_getcwd(clipboard.data.sd.directory, CLIPBOARD_PATH_LEN);
                strncpy(clipboard.data.sd.filename, name.c_str(), CLIPBOARD_PATH_LEN - 1);
            });
            if (clipboard.type == CLIPBOARD_TYPE_SD_FILE) {
              menu->addLine(STR_PASTE, [=]() {
                  // TODO
              });
            }
            menu->addLine(STR_RENAME_FILE, [=]() {
                // TODO
            });
            menu->addLine(STR_DELETE_FILE, [=]() {
                f_unlink(getFullPath(name));
                // coord_t scrollPosition = window->getScrollPositionY();
                window->clear();
                build(window);
                // window->setScrollPositionY(scrollPosition);
            });
          }
          return 0;
      }, 0);
      button->setFocusHandler([=](bool active) {
        if (active) {
          preview->setFile(getFullPath(name));
        }
      });
      grid.nextLine();
    }
  }

  window->setInnerHeight(grid.getWindowHeight());
  preview->setHeight(max(window->height(), grid.getWindowHeight()));
}

#if 0
bool menuRadioSdManagerInfo(event_t event)
{
  SIMPLE_SUBMENU(STR_SD_INFO_TITLE, ICON_RADIO_SD_MANAGER, 1);

  lcdDrawText(MENUS_MARGIN_LEFT, 2*FH, STR_SD_TYPE);
  lcdDrawText(100, 2*FH, SD_IS_HC() ? STR_SDHC_CARD : STR_SD_CARD);

  lcdDrawText(MENUS_MARGIN_LEFT, 3*FH, STR_SD_SIZE);
  lcdDrawNumber(100, 3*FH, sdGetSize(), LEFT, 0, nullptr, "M");

  lcdDrawText(MENUS_MARGIN_LEFT, 4*FH, STR_SD_SECTORS);
#if defined(SD_GET_FREE_BLOCKNR)
  lcdDrawNumber(100, 4*FH, SD_GET_FREE_BLOCKNR()/1000, LEFT, 0, nullptr, "/");
  lcdDrawNumber(150, 4*FH, sdGetNoSectors()/1000, LEFT);
#else
  lcdDrawNumber(100, 4*FH, sdGetNoSectors()/1000, LEFT, 0, NULL, "k");
#endif

  lcdDrawText(MENUS_MARGIN_LEFT, 5*FH, STR_SD_SPEED);
  lcdDrawNumber(100, 5*FH, SD_GET_SPEED()/1000, LEFT, 0, NULL, "kb/s");

  return true;
}
#endif

#if 0
void onSdManagerMenu(const char * result)
{
  TCHAR lfn[FF_MAX_LFN+1];

  // TODO possible buffer overflows here!

  uint8_t index = menuVerticalPosition-menuVerticalOffset;
  char *line = reusableBuffer.sdmanager.lines[index];

  if (result == STR_SD_INFO) {
    pushMenu(menuRadioSdManagerInfo);
  }
  else if (result == STR_SD_FORMAT) {
    POPUP_CONFIRMATION(STR_CONFIRM_FORMAT);
  }
  else if (result == STR_PASTE) {
    f_getcwd(lfn, FF_MAX_LFN);
    // if destination is dir, copy into that dir
    if (IS_DIRECTORY(line)) {
      strcat(lfn, PSTR("/"));
      strcat(lfn, line);
    }
    if (strcmp(clipboard.data.sd.directory, lfn)) {  // prevent copying to the same directory
      POPUP_WARNING(sdCopyFile(clipboard.data.sd.filename, clipboard.data.sd.directory, clipboard.data.sd.filename, lfn));
      REFRESH_FILES();
    }
  }
  else if (result == STR_RENAME_FILE) {
    memcpy(reusableBuffer.sdmanager.originalName, line, sizeof(reusableBuffer.sdmanager.originalName));
    uint8_t fnlen = 0, extlen = 0;
    getFileExtension(line, 0, LEN_FILE_EXTENSION_MAX, &fnlen, &extlen);
    // write spaces to allow extending the length of a filename
    memset(line + fnlen - extlen, ' ', SD_SCREEN_FILE_LENGTH - fnlen + extlen);
    line[SD_SCREEN_FILE_LENGTH-extlen] = '\0';
    s_editMode = EDIT_MODIFY_STRING;
    editNameCursorPos = 0;
  }
  else if (result == STR_ASSIGN_BITMAP) {
    memcpy(g_model.header.bitmap, line, sizeof(g_model.header.bitmap));
    storageDirty(EE_MODEL);
  }
  else if (result == STR_ASSIGN_SPLASH) {
    f_getcwd(lfn, FF_MAX_LFN);
    sdCopyFile(line, lfn, SPLASH_FILE, BITMAPS_PATH);
  }
  else if (result == STR_VIEW_TEXT) {
    getSelectionFullPath(lfn);
    pushMenuTextView(lfn);
  }
}
#endif
