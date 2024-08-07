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

#include <stdio.h>
#include "edgetx.h"
#include "io/frsky_firmware_update.h"
#include "io/multi_firmware_update.h"
#include "io/bootloader_flash.h"
#include "lib_file.h"
#include "hal/storage.h"

#define NODE_TYPE(fname)       fname[SD_SCREEN_FILE_LENGTH+1]
#define IS_DIRECTORY(fname)    ((bool)(!NODE_TYPE(fname)))
#define IS_FILE(fname)         ((bool)(NODE_TYPE(fname)))

inline void REFRESH_FILES()
{
  reusableBuffer.sdManager.offset = 65535;
}

void menuRadioSdManagerInfo(event_t event)
{
  SIMPLE_SUBMENU(STR_SD_INFO_TITLE, 1);

  lcdDrawTextAlignedLeft(2*FH, STR_SD_SIZE);
  lcdDrawNumber(10*FW, 2*FH, sdGetSize(), LEFT);
  lcdDrawChar(lcdLastRightPos, 3*FH, 'M');

  lcdDrawTextAlignedLeft(3*FH, STR_SD_SECTORS);
  lcdDrawNumber(10*FW, 3*FH,  sdGetFreeSectors()/1000, LEFT);
  lcdDrawChar(lcdLastRightPos, 3*FH, '/');
  lcdDrawNumber(lcdLastRightPos+FW, 3*FH, sdGetNoSectors()/1000, LEFT);
  lcdDrawChar(lcdLastRightPos, 3*FH, 'k');
}

inline bool isFilenameGreater(bool isfile, const char * fn, const char * line)
{
  return (isfile && IS_DIRECTORY(line)) || (isfile==IS_FILE(line) && strcasecmp(fn, line) > 0);
}

inline bool isFilenameLower(bool isfile, const char * fn, const char * line)
{
  return (!isfile && IS_FILE(line)) || (isfile==IS_FILE(line) && strcasecmp(fn, line) < 0);
}

void getSelectionFullPath(char * lfn)
{
  f_getcwd(lfn, FF_MAX_LFN);
  strcat(lfn, "/");
  strcat(lfn, reusableBuffer.sdManager.lines[menuVerticalPosition - HEADER_LINE - menuVerticalOffset]);
}

#if defined(PXX2)

#include "pulses/pxx2_ota.h"

void onUpdateConfirmation(const char * result)
{
  if (result == STR_OK) {
    OtaUpdateInformation * destination = moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].otaUpdateInformation;
    Pxx2OtaUpdate otaUpdate(reusableBuffer.sdManager.otaUpdateInformation.module, destination->candidateReceiversNames[destination->selectedReceiverIndex]);
    otaUpdate.flashFirmware(destination->filename, drawProgressScreen);
  }
  else {
    moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode = MODULE_MODE_NORMAL;
  }
}

void onUpdateStateChanged()
{
  if (reusableBuffer.sdManager.otaUpdateInformation.step == BIND_INFO_REQUEST) {
    uint8_t modelId = reusableBuffer.sdManager.otaUpdateInformation.receiverInformation.modelID;
    if (isPXX2ReceiverOptionAvailable(modelId, RECEIVER_OPTION_OTA_TO_UPDATE_SELF)) {
      POPUP_CONFIRMATION(getPXX2ReceiverName(modelId), onUpdateConfirmation);
      char *tmp = strAppend(reusableBuffer.sdManager.otaReceiverVersion, TR_CURRENT_VERSION);
      tmp = strAppendUnsigned(tmp, 1 + reusableBuffer.sdManager.otaUpdateInformation.receiverInformation.swVersion.major);
      *tmp++ = '.';
      tmp = strAppendUnsigned(tmp, reusableBuffer.sdManager.otaUpdateInformation.receiverInformation.swVersion.minor);
      *tmp++ = '.';
      tmp = strAppendUnsigned(tmp, reusableBuffer.sdManager.otaUpdateInformation.receiverInformation.swVersion.revision);
      SET_WARNING_INFO(reusableBuffer.sdManager.otaReceiverVersion, tmp - reusableBuffer.sdManager.otaReceiverVersion, 0);
    }
    else {
      POPUP_WARNING(STR_OTA_UPDATE_ERROR);
      SET_WARNING_INFO(STR_UNSUPPORTED_RX, sizeof(TR_UNSUPPORTED_RX) - 1, 0);
      moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode = MODULE_MODE_NORMAL;
    }
  }
}
#endif

void onSdManagerMenu(const char * result)
{
  TCHAR lfn[FF_MAX_LFN+1];

  // TODO possible buffer overflows here!

  uint8_t index = menuVerticalPosition - HEADER_LINE - menuVerticalOffset;
  char * line = reusableBuffer.sdManager.lines[index];

  if (result == STR_SD_INFO) {
    pushMenu(menuRadioSdManagerInfo);
  }
  else if (result == STR_COPY_FILE) {
    clipboard.type = CLIPBOARD_TYPE_SD_FILE;
    f_getcwd(clipboard.data.sd.directory, CLIPBOARD_PATH_LEN);
    strncpy(clipboard.data.sd.filename, line, CLIPBOARD_PATH_LEN-1);
  }
  else if (result == STR_PASTE) {
    char destFileName[2 * CLIPBOARD_PATH_LEN + 1];
    f_getcwd(lfn, FF_MAX_LFN);
    // if destination is dir, copy into that dir
    if (IS_DIRECTORY(line)) {
      strcat(lfn, "/");
      strcat(lfn, line);
    }
    char *destNamePtr = clipboard.data.sd.filename;
    if (!strcmp(clipboard.data.sd.directory, lfn)) {
        // prevent copying to the same directory under the same name
        destNamePtr =
            strAppend(destFileName, FILE_COPY_PREFIX, CLIPBOARD_PATH_LEN);
        destNamePtr = strAppend(destNamePtr, clipboard.data.sd.filename,
                                CLIPBOARD_PATH_LEN);
        destNamePtr = destFileName;
    }
    POPUP_WARNING(sdCopyFile(clipboard.data.sd.filename,
                             clipboard.data.sd.directory, destNamePtr, lfn));
    REFRESH_FILES();
  }
  else if (result == STR_RENAME_FILE) {
    memcpy(reusableBuffer.sdManager.originalName, line, sizeof(reusableBuffer.sdManager.originalName));
    uint8_t fnlen = 0, extlen = 0;
    getFileExtension(line, 0, LEN_FILE_EXTENSION_MAX, &fnlen, &extlen);
    // write spaces to allow extending the length of a filename
    memset(line + fnlen - extlen, ' ', SD_SCREEN_FILE_LENGTH - fnlen + extlen);
    line[SD_SCREEN_FILE_LENGTH-extlen] = '\0';
    s_editMode = EDIT_MODIFY_STRING;
    editNameCursorPos = 0;
  }
  else if (result == STR_DELETE_FILE) {
    getSelectionFullPath(lfn);
    f_unlink(lfn);
    strncpy(statusLineMsg, line, 13);
    strcpy(statusLineMsg+min((uint8_t)strlen(statusLineMsg), (uint8_t)13), STR_REMOVED);
    showStatusLine();
    REFRESH_FILES();
  }
  else if (result == STR_PLAY_FILE) {
    getSelectionFullPath(lfn);
    audioQueue.stopAll();
    audioQueue.playFile(lfn, 0, ID_PLAY_FROM_SD_MANAGER);
  }
#if LCD_DEPTH > 1
  else if (result == STR_ASSIGN_BITMAP) {
    strAppendFilename(g_model.header.bitmap, line, LEN_BITMAP_NAME);
    memcpy(modelHeaders[g_eeGeneral.currModel].bitmap, g_model.header.bitmap, LEN_BITMAP_NAME);
    storageDirty(EE_MODEL);
  }
#endif
  else if (result == STR_VIEW_TEXT) {
    getSelectionFullPath(lfn);
    pushMenuTextView(lfn);
  }
#if defined(PCBTARANIS)
  else if (result == STR_FLASH_BOOTLOADER) {
    getSelectionFullPath(lfn);
    BootloaderFirmwareUpdate bootloader;
    bootloader.flashFirmware(lfn, drawProgressScreen);
  }
  else if (result == STR_FLASH_INTERNAL_MODULE) {
    getSelectionFullPath(lfn);
    FrskyDeviceFirmwareUpdate device(INTERNAL_MODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
#if defined(HARDWARE_EXTERNAL_MODULE)
  else if (result == STR_FLASH_EXTERNAL_MODULE) {
    // needed on X-Lite (as the R9M needs 2S while the external device flashing port only provides 5V)
    getSelectionFullPath(lfn);
    FrskyDeviceFirmwareUpdate device(EXTERNAL_MODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
#endif
  else if (result == STR_FLASH_EXTERNAL_DEVICE) {
    getSelectionFullPath(lfn);
    FrskyDeviceFirmwareUpdate device(SPORT_MODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
#if defined(MULTIMODULE)
#if defined(INTERNAL_MODULE_MULTI)
  else if (result == STR_FLASH_INTERNAL_MULTI) {
    getSelectionFullPath(lfn);
    MultiDeviceFirmwareUpdate device(INTERNAL_MODULE, MULTI_TYPE_MULTIMODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  else if (result == STR_FLASH_EXTERNAL_MULTI) {
    getSelectionFullPath(lfn);
    MultiDeviceFirmwareUpdate device(EXTERNAL_MODULE, MULTI_TYPE_MULTIMODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
  else if (result == STR_FLASH_EXTERNAL_ELRS) {
    getSelectionFullPath(lfn);
    MultiDeviceFirmwareUpdate device(EXTERNAL_MODULE, MULTI_TYPE_ELRS);
    device.flashFirmware(lfn, drawProgressScreen);
  }
#endif
#endif
#if defined(BLUETOOTH)
  else if (result == STR_FLASH_BLUETOOTH_MODULE) {
    getSelectionFullPath(lfn);
    bluetooth.flashFirmware(lfn, drawProgressScreen);
  }
#endif
#if defined(PXX2) && defined(HARDWARE_EXTERNAL_MODULE)
  else if (result == STR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA || result == STR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA) {
    memclear(&reusableBuffer.sdManager.otaUpdateInformation, sizeof(OtaUpdateInformation));
    getSelectionFullPath(reusableBuffer.sdManager.otaUpdateInformation.filename);
    reusableBuffer.sdManager.otaUpdateInformation.module = (result == STR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA ? INTERNAL_MODULE : EXTERNAL_MODULE);
    moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].startBind(&reusableBuffer.sdManager.otaUpdateInformation, onUpdateStateChanged);
  }
  else if (result == STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA || result == STR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA) {
    memclear(&reusableBuffer.sdManager.otaUpdateInformation, sizeof(OtaUpdateInformation));
    getSelectionFullPath(reusableBuffer.sdManager.otaUpdateInformation.filename);
    reusableBuffer.sdManager.otaUpdateInformation.module = (result == STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA ? INTERNAL_MODULE : EXTERNAL_MODULE);
    moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].startBind(&reusableBuffer.sdManager.otaUpdateInformation, onUpdateStateChanged);
  }
#endif
#endif
#if defined(LUA)
  else if (result == STR_EXECUTE_FILE) {
    getSelectionFullPath(lfn);
    luaExec(lfn);
  }
#endif
}

#if defined(PXX2)
void onUpdateReceiverSelection(const char * result)
{
  if (result != STR_EXIT) {
    reusableBuffer.sdManager.otaUpdateInformation.selectedReceiverIndex = (result - reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversNames[0]) / sizeof(reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversNames[0]);
    reusableBuffer.sdManager.otaUpdateInformation.step = BIND_INFO_REQUEST;
#if defined(SIMU)
    reusableBuffer.sdManager.otaUpdateInformation.receiverInformation.modelID = 0x01;
    onUpdateStateChanged();
#endif
  }
  else {
    // the user pressed [Exit]
    moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode = MODULE_MODE_NORMAL;
  }
}
#endif

void menuRadioSdManager(event_t _event)
{
#if defined(NAVIGATION_X9D)
  int lastPos = menuVerticalPosition;
#endif

  if (_event == EVT_ENTRY) {
    f_chdir(ROOT_PATH);
#if defined(NAVIGATION_X9D)
    lastPos = -1;
#endif
  }

  if (_event == EVT_ENTRY || _event == EVT_ENTRY_UP) {
    memclear(&reusableBuffer.sdManager, sizeof(reusableBuffer.sdManager));
    REFRESH_FILES();
  }

#if defined(PXX2)
  if (EVT_KEY_MASK(_event) == KEY_EXIT && moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode == MODULE_MODE_BIND) {
    moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode = MODULE_MODE_NORMAL;
    CLEAR_POPUP();
    killEvents(KEY_EXIT);
    _event = 0;
  }
#endif

  event_t event = (EVT_KEY_MASK(_event) == KEY_ENTER ? 0 : _event);
  uint8_t old_editMode = s_editMode;
  SIMPLE_MENU(STR_SD_CARD, menuTabGeneral, MENU_RADIO_SD_MANAGER, HEADER_LINE + reusableBuffer.sdManager.count);

  switch (_event) {

#if defined(KEYS_GPIO_REG_MENU)
    case EVT_KEY_LONG(KEY_MENU):
      if (SD_CARD_PRESENT() && s_editMode == 0) {
        POPUP_MENU_ADD_ITEM(STR_SD_INFO);
        POPUP_MENU_START(onSdManagerMenu);
      }
      break;
#endif

    case EVT_KEY_BREAK(KEY_EXIT):
      REFRESH_FILES();
      break;

#if !defined(PCBTARANIS)
    case EVT_KEY_FIRST(KEY_RIGHT):
#endif
    case EVT_KEY_BREAK(KEY_ENTER):
      if (s_editMode > 0) {
        break;
      }
      else {
        int index = menuVerticalPosition - HEADER_LINE - menuVerticalOffset;
        if (IS_DIRECTORY(reusableBuffer.sdManager.lines[index])) {
          f_chdir(reusableBuffer.sdManager.lines[index]);
          menuVerticalOffset = 0;
          menuVerticalPosition = HEADER_LINE;
          REFRESH_FILES();
          return;
        }
      }
      break;

    case EVT_KEY_LONG(KEY_ENTER):
#if (HEADER_LINE > 0)
      if (menuVerticalPosition < HEADER_LINE) {
        POPUP_MENU_ADD_ITEM(STR_SD_INFO);
        POPUP_MENU_START(onSdManagerMenu);
        break;
      }
#endif
      TCHAR lfn[FF_MAX_LFN + 1];
      getSelectionFullPath(lfn);

      if (SD_CARD_PRESENT() && s_editMode <= 0) {
        int index = menuVerticalPosition - HEADER_LINE - menuVerticalOffset;
        char * line = reusableBuffer.sdManager.lines[index];
        if (!strcmp(line, "..")) {
          break; // no menu for parent dir
        }
        const char * ext = getFileExtension(line);
        if (ext) {
          if (!strcasecmp(ext, SOUNDS_EXT)) {
            POPUP_MENU_ADD_ITEM(STR_PLAY_FILE);
          }
#if LCD_DEPTH > 1
          else if (isExtensionMatching(ext, BITMAPS_EXT)) {
            if ((ext-line) <= LEN_BITMAP_NAME) {
              POPUP_MENU_ADD_ITEM(STR_ASSIGN_BITMAP);
            }
          }
#endif
#if defined(LUA)
          else if (isExtensionMatching(ext, SCRIPTS_EXT)) {
            POPUP_MENU_ADD_ITEM(STR_EXECUTE_FILE);
          }
#endif
#if defined(MULTIMODULE) && !defined(DISABLE_MULTI_UPDATE)
          if (!strcasecmp(ext, MULTI_FIRMWARE_EXT)) {
            MultiFirmwareInformation information;
            if (information.readMultiFirmwareInformation(line) == nullptr) {
#if defined(INTERNAL_MODULE_MULTI)
              POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MULTI);
#endif
              POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MULTI);
            }
          }

          if (!strcasecmp(ext, ELRS_FIRMWARE_EXT)) {
            TCHAR lfn[FF_MAX_LFN + 1];
            getSelectionFullPath(lfn);
            POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_ELRS);
          }
#endif
#if defined(PCBTARANIS)
          if (!strcasecmp(ext, FIRMWARE_EXT)) {
            if (isBootloader(lfn)) {
              POPUP_MENU_ADD_ITEM(STR_FLASH_BOOTLOADER);
            }
          }
          else if (!strcasecmp(ext, SPORT_FIRMWARE_EXT)) {
            auto mod_desc = modulePortGetModuleDescription(SPORT_MODULE);
            if (mod_desc && mod_desc->set_pwr)
              POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_DEVICE);

            if (g_eeGeneral.internalModule == MODULE_TYPE_XJT_PXX1 ||
                g_eeGeneral.internalModule == MODULE_TYPE_ISRM_PXX2)
              POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MODULE);

            POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);
          }
          else if (!strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
            FrSkyFirmwareInformation information;
            if (readFrSkyFirmwareInformation(line, information) == nullptr) {

              if (information.productFamily == FIRMWARE_FAMILY_INTERNAL_MODULE &&
                  (g_eeGeneral.internalModule == MODULE_TYPE_XJT_PXX1 ||
                   g_eeGeneral.internalModule == MODULE_TYPE_ISRM_PXX2))
                POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MODULE);

              if (information.productFamily == FIRMWARE_FAMILY_EXTERNAL_MODULE)
                POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);

              if (information.productFamily == FIRMWARE_FAMILY_RECEIVER ||
                  information.productFamily == FIRMWARE_FAMILY_SENSOR) {

                auto mod_desc = modulePortGetModuleDescription(SPORT_MODULE);
                if (mod_desc && mod_desc->set_pwr)
                  POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_DEVICE);
                else
                  POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);
              }
#if defined(PXX2) && defined(HARDWARE_EXTERNAL_MODULE)
              if (information.productFamily == FIRMWARE_FAMILY_RECEIVER) {
                if (isReceiverOTAEnabledFromModule(INTERNAL_MODULE, information.productId))
                  POPUP_MENU_ADD_ITEM(STR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA);
                if (isReceiverOTAEnabledFromModule(EXTERNAL_MODULE, information.productId))
                  POPUP_MENU_ADD_ITEM(STR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA);
              }
              if (information.productFamily == FIRMWARE_FAMILY_FLIGHT_CONTROLLER) {
                POPUP_MENU_ADD_ITEM(STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA);
                POPUP_MENU_ADD_ITEM(STR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA);
              }
#endif
#if defined(BLUETOOTH)
              if (information.productFamily == FIRMWARE_FAMILY_BLUETOOTH_CHIP)
                POPUP_MENU_ADD_ITEM(STR_FLASH_BLUETOOTH_MODULE);
#endif
            }
          }
#endif
          if (isExtensionMatching(ext, TEXT_EXT) || isExtensionMatching(ext, SCRIPTS_EXT)) {
            POPUP_MENU_ADD_ITEM(STR_VIEW_TEXT);
          }
        }
        if (IS_FILE(line))
          POPUP_MENU_ADD_ITEM(STR_COPY_FILE);
        if (clipboard.type == CLIPBOARD_TYPE_SD_FILE)
          POPUP_MENU_ADD_ITEM(STR_PASTE);
        POPUP_MENU_ADD_ITEM(STR_RENAME_FILE);
        if (IS_FILE(line))
          POPUP_MENU_ADD_ITEM(STR_DELETE_FILE);
        POPUP_MENU_START(onSdManagerMenu);
      }
      break;
  }

  if (SD_CARD_PRESENT()) {
    if (reusableBuffer.sdManager.offset != menuVerticalOffset) {
      FILINFO fno;
      DIR dir;

      if (menuVerticalOffset == reusableBuffer.sdManager.offset + 1) {
        memmove(reusableBuffer.sdManager.lines[0], reusableBuffer.sdManager.lines[1], (NUM_BODY_LINES-1)*sizeof(reusableBuffer.sdManager.lines[0]));
        memset(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1], 0xff, SD_SCREEN_FILE_LENGTH);
        NODE_TYPE(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1]) = 1;
      }
      else if (menuVerticalOffset == reusableBuffer.sdManager.offset - 1) {
        memmove(reusableBuffer.sdManager.lines[1], reusableBuffer.sdManager.lines[0], (NUM_BODY_LINES-1)*sizeof(reusableBuffer.sdManager.lines[0]));
        memset(reusableBuffer.sdManager.lines[0], 0, sizeof(reusableBuffer.sdManager.lines[0]));
      }
      else {
        reusableBuffer.sdManager.offset = menuVerticalOffset;
        memset(reusableBuffer.sdManager.lines, 0, sizeof(reusableBuffer.sdManager.lines));
      }

      reusableBuffer.sdManager.count = 0;

      FRESULT res = f_opendir(&dir, "."); // Open the directory
      if (res == FR_OK) {
        bool firstTime = true;
        for (;;) {
          res = sdReadDir(&dir, &fno, firstTime);
          if (res != FR_OK || fno.fname[0] == 0) break;              /* Break on error or end of dir */
          if (strlen(fno.fname) > SD_SCREEN_FILE_LENGTH) continue;
          if (fno.fattrib & (AM_HID|AM_SYS)) continue;               /* Ignore hidden and system files */
          if (fno.fname[0] == '.' && fno.fname[1] != '.') continue;  /* Ignore UNIX hidden files, but not .. */

          reusableBuffer.sdManager.count++;

          bool isfile = !(fno.fattrib & AM_DIR);

          if (menuVerticalOffset == 0) {
            for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
              char * line = reusableBuffer.sdManager.lines[i];
              if (line[0] == '\0' || isFilenameLower(isfile, fno.fname, line)) {
                if (i < NUM_BODY_LINES-1) memmove(reusableBuffer.sdManager.lines[i+1], line, sizeof(reusableBuffer.sdManager.lines[i]) * (NUM_BODY_LINES-1-i));
                memset(line, 0, sizeof(reusableBuffer.sdManager.lines[0]));
                strcpy(line, fno.fname);
                NODE_TYPE(line) = isfile;
                break;
              }
            }
          }
          else if (reusableBuffer.sdManager.offset == menuVerticalOffset) {
            for (int8_t i=NUM_BODY_LINES-1; i>=0; i--) {
              char * line = reusableBuffer.sdManager.lines[i];
              if (line[0] == '\0' || isFilenameGreater(isfile, fno.fname, line)) {
                if (i > 0) memmove(reusableBuffer.sdManager.lines[0], reusableBuffer.sdManager.lines[1], sizeof(reusableBuffer.sdManager.lines[0]) * i);
                memset(line, 0, sizeof(reusableBuffer.sdManager.lines[0]));
                strcpy(line, fno.fname);
                NODE_TYPE(line) = isfile;
                break;
              }
            }
          }
          else if (menuVerticalOffset > reusableBuffer.sdManager.offset) {
            if (isFilenameGreater(isfile, fno.fname, reusableBuffer.sdManager.lines[NUM_BODY_LINES-2]) && isFilenameLower(isfile, fno.fname, reusableBuffer.sdManager.lines[NUM_BODY_LINES-1])) {
              memset(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1], 0, sizeof(reusableBuffer.sdManager.lines[0]));
              strcpy(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1], fno.fname);
              NODE_TYPE(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1]) = isfile;
            }
          }
          else {
            if (isFilenameLower(isfile, fno.fname, reusableBuffer.sdManager.lines[1]) && isFilenameGreater(isfile, fno.fname, reusableBuffer.sdManager.lines[0])) {
              memset(reusableBuffer.sdManager.lines[0], 0, sizeof(reusableBuffer.sdManager.lines[0]));
              strcpy(reusableBuffer.sdManager.lines[0], fno.fname);
              NODE_TYPE(reusableBuffer.sdManager.lines[0]) = isfile;
            }
          }
        }
        f_closedir(&dir);
      }
    }

    reusableBuffer.sdManager.offset = menuVerticalOffset;
    int index = menuVerticalPosition - HEADER_LINE - menuVerticalOffset;

    for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
      coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
      lcdNextPos = 0;
      LcdFlags attr = (index == i ? INVERS : 0);
      if (reusableBuffer.sdManager.lines[i][0]) {
        if (IS_DIRECTORY(reusableBuffer.sdManager.lines[i])) {
          lcdDrawChar(0, y, '[', s_editMode == EDIT_MODIFY_STRING ? 0 : attr);
        }
        if (s_editMode == EDIT_MODIFY_STRING && attr) {
          uint8_t extlen, efflen;
          const char * ext = getFileExtension(reusableBuffer.sdManager.originalName, 0, 0, nullptr, &extlen);

          editName(lcdNextPos, y, reusableBuffer.sdManager.lines[i],
                   SD_SCREEN_FILE_LENGTH - extlen, _event, attr, 0, old_editMode);

          efflen = effectiveLen(reusableBuffer.sdManager.lines[i],
                                SD_SCREEN_FILE_LENGTH - extlen);

          if (s_editMode == 0) {
            if (ext) {
              strAppend(&reusableBuffer.sdManager.lines[i][efflen], ext);
            }
            else {
              reusableBuffer.sdManager.lines[i][efflen] = 0;
            }
            f_rename(reusableBuffer.sdManager.originalName, reusableBuffer.sdManager.lines[i]);
            REFRESH_FILES();
          }
        }
        else {
          lcdDrawText(lcdNextPos, y, reusableBuffer.sdManager.lines[i], attr);
        }
        if (IS_DIRECTORY(reusableBuffer.sdManager.lines[i])) {
          lcdDrawChar(lcdNextPos, y, ']', s_editMode == EDIT_MODIFY_STRING ? 0 : attr);
        }
      }
    }

#if defined(PXX2)
    if (moduleState[reusableBuffer.sdManager.otaUpdateInformation.module].mode == MODULE_MODE_BIND) {
      if (reusableBuffer.sdManager.otaUpdateInformation.step == BIND_INIT) {
        if (reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversCount > 0) {
          if (reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversCount != popupMenuItemsCount) {
            CLEAR_POPUP();
            popupMenuItemsCount = min<uint8_t>(reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversCount, PXX2_MAX_RECEIVERS_PER_MODULE);
            for (auto rx = 0; rx < popupMenuItemsCount; rx++) {
              popupMenuItems[rx] = reusableBuffer.sdManager.otaUpdateInformation.candidateReceiversNames[rx];
            }
            POPUP_MENU_TITLE(STR_PXX2_SELECT_RX);
            POPUP_MENU_START(onUpdateReceiverSelection);
          }
        }
        else {
          POPUP_WAIT(STR_WAITING_FOR_RX);
        }
      }
    }
#endif

#if LCD_DEPTH > 1
    const char * ext = getFileExtension(reusableBuffer.sdManager.lines[index]);
    if (ext && isExtensionMatching(ext, BITMAPS_EXT)) {
      if (lastPos != menuVerticalPosition) {
        if (!lcdLoadBitmap(modelBitmap, reusableBuffer.sdManager.lines[index], MODEL_BITMAP_WIDTH, MODEL_BITMAP_HEIGHT)) {
          memcpy(modelBitmap, logo_taranis, MODEL_BITMAP_SIZE);
        }
      }
      lcdDrawBitmap(22*FW+2, 2*FH+FH/2, modelBitmap);
    }
#endif
  }
  else {
    lcdDrawCenteredText(LCD_H/2, STR_NO_SDCARD);
    REFRESH_FILES();
  }
}
