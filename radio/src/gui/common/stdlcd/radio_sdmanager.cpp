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
#include "opentx.h"
#include "VirtualFS.h"
#include "io/frsky_firmware_update.h"
#include "io/multi_firmware_update.h"
#include "io/bootloader_flash.h"
#include "libopenui/src/libopenui_file.h"

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

  lcdDrawTextAlignedLeft(2*FH, STR_SD_TYPE);
  lcdDrawText(10*FW, 2*FH, SD_IS_HC() ? STR_SDHC_CARD : STR_SD_CARD);

  lcdDrawTextAlignedLeft(3*FH, STR_SD_SIZE);
  lcdDrawNumber(10*FW, 3*FH, sdGetSize(), LEFT);
  lcdDrawChar(lcdLastRightPos, 3*FH, 'M');

  lcdDrawTextAlignedLeft(4*FH, STR_SD_SECTORS);
#if defined(SD_GET_FREE_BLOCKNR)
  lcdDrawNumber(10*FW, 4*FH,  SD_GET_FREE_BLOCKNR()/1000, LEFT);
  lcdDrawChar(lcdLastRightPos, 4*FH, '/');
  lcdDrawNumber(lcdLastRightPos+FW, 4*FH, sdGetNoSectors()/1000, LEFT);
#else
  lcdDrawNumber(10*FW, 4*FH, sdGetNoSectors()/1000, LEFT);
#endif
  lcdDrawChar(lcdLastRightPos, 4*FH, 'k');

  lcdDrawTextAlignedLeft(5*FH, STR_SD_SPEED);
  lcdDrawNumber(10*FW, 5*FH, SD_GET_SPEED()/1000, LEFT);
  lcdDrawText(lcdLastRightPos, 5*FH, "kb/s");
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

void onSdFormatConfirm(const char * result)
{
  if (result == STR_OK) {
    showMessageBox(STR_FORMATTING);
    logsClose();
    audioQueue.stopSD();
    if (sdCardFormat()) {
      f_chdir(ROOT_PATH);
      REFRESH_FILES();
    }
  }
}

#if defined(PXX2)
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
    if (modelId > 0 && modelId < DIM(PXX2ReceiversNames)) {
      if (isPXX2ReceiverOptionAvailable(modelId, RECEIVER_OPTION_OTA)) {
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
    else {
      POPUP_WARNING(STR_OTA_UPDATE_ERROR);
      SET_WARNING_INFO(STR_UNKNOWN_RX, sizeof(TR_UNKNOWN_RX) - 1, 0);
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
  else if (result == STR_SD_FORMAT) {
    POPUP_CONFIRMATION(STR_CONFIRM_FORMAT, onSdFormatConfirm);
  }
  else if (result == STR_COPY_FILE) {
    clipboard.type = CLIPBOARD_TYPE_SD_FILE;
    f_getcwd(clipboard.data.sd.directory, CLIPBOARD_PATH_LEN);
    strncpy(clipboard.data.sd.filename, line, CLIPBOARD_PATH_LEN-1);
  }
  else if (result == STR_PASTE) {
    f_getcwd(lfn, FF_MAX_LFN);
    // if destination is dir, copy into that dir
    if (IS_DIRECTORY(line)) {
      strcat(lfn, "/");
      strcat(lfn, line);
    }
    char *destNamePtr = clipboard.data.sd.filename;
    if (!strcmp(clipboard.data.sd.directory, lfn)) {
        // prevent copying to the same directory under the same name
        char destFileName[2 * CLIPBOARD_PATH_LEN + 1];
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
    strAppendFilename(g_model.header.bitmap, line, sizeof(g_model.header.bitmap));
    memcpy(modelHeaders[g_eeGeneral.currModel].bitmap, g_model.header.bitmap, sizeof(g_model.header.bitmap));
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
  else if (result == STR_FLASH_EXTERNAL_MODULE) {
    // needed on X-Lite (as the R9M needs 2S while the external device flashing port only provides 5V)
    getSelectionFullPath(lfn);
    FrskyDeviceFirmwareUpdate device(EXTERNAL_MODULE);
    device.flashFirmware(lfn, drawProgressScreen);
  }
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
#if defined(BLUETOOTH)
  else if (result == STR_FLASH_BLUETOOTH_MODULE) {
    getSelectionFullPath(lfn);
    bluetooth.flashFirmware(lfn, drawProgressScreen);
  }
#endif
#if defined(HARDWARE_POWER_MANAGEMENT_UNIT)
  else if (result == STR_FLASH_POWER_MANAGEMENT_UNIT) {
    getSelectionFullPath(lfn);
    FrskyChipFirmwareUpdate device;
    device.flashFirmware(lfn, drawProgressScreen);
  }
#endif
#if defined(PXX2)
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
#if LCD_DEPTH > 1
  int lastPos = menuVerticalPosition;
#endif

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
  SIMPLE_MENU(SD_IS_HC() ? STR_SDHC_CARD : STR_SD_CARD, menuTabGeneral, MENU_RADIO_SD_MANAGER, HEADER_LINE + reusableBuffer.sdManager.count);

  switch (_event) {
    case EVT_ENTRY:
      f_chdir(ROOT_PATH);
#if LCD_DEPTH > 1
      lastPos = -1;
#endif
      // no break

    case EVT_ENTRY_UP:
      memclear(&reusableBuffer.sdManager, sizeof(reusableBuffer.sdManager));
      REFRESH_FILES();
      break;

#if defined(PCBX9) || defined(RADIO_X7) || defined(RADIO_X7ACCESS) // TODO NO_MENU_KEY
    case EVT_KEY_LONG(KEY_MENU):
      if (SD_CARD_PRESENT() && !READ_ONLY() && s_editMode == 0) {
        killEvents(_event);
        POPUP_MENU_ADD_ITEM(STR_SD_INFO);
        POPUP_MENU_ADD_ITEM(STR_SD_FORMAT);
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
          killEvents(_event);
          return;
        }
      }
      break;

    case EVT_KEY_LONG(KEY_ENTER):
#if !defined(PCBX9) && !defined(RADIO_X7) && !defined(RADIO_X7ACCESS) // TODO NO_HEADER_LINE
      if (menuVerticalPosition < HEADER_LINE) {
        killEvents(_event);
        POPUP_MENU_ADD_ITEM(STR_SD_INFO);
        POPUP_MENU_ADD_ITEM(STR_SD_FORMAT);
        POPUP_MENU_START(onSdManagerMenu);
        break;
      }
#endif
      TCHAR lfn[FF_MAX_LFN + 1];
      getSelectionFullPath(lfn);

      if (SD_CARD_PRESENT() && s_editMode <= 0) {
        killEvents(_event);
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
            if (!READ_ONLY() && (ext-line) <= (int)sizeof(g_model.header.bitmap)) {
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
          if (!READ_ONLY() && !strcasecmp(ext, MULTI_FIRMWARE_EXT)) {
            MultiFirmwareInformation information;
            if (information.readMultiFirmwareInformation(line) == nullptr) {
#if defined(INTERNAL_MODULE_MULTI)
              POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MULTI);
#endif
              POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MULTI);
            }
          }

          if (!READ_ONLY() && !strcasecmp(ext, ELRS_FIRMWARE_EXT)) {
            TCHAR lfn[FF_MAX_LFN + 1];
            getSelectionFullPath(lfn);
            POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_ELRS);
          }
#endif
#if defined(PCBTARANIS)
          if (!READ_ONLY() && !strcasecmp(ext, FIRMWARE_EXT)) {
            if (isBootloader(lfn)) {
              POPUP_MENU_ADD_ITEM(STR_FLASH_BOOTLOADER);
            }
          }
          else if (!READ_ONLY() && !strcasecmp(ext, SPORT_FIRMWARE_EXT)) {
            if (HAS_SPORT_UPDATE_CONNECTOR())
              POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_DEVICE);
#if defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)
            POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MODULE);
#endif
            POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);
          }
          else if (!READ_ONLY() && !strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
            FrSkyFirmwareInformation information;
            if (readFrSkyFirmwareInformation(line, information) == nullptr) {
#if defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)
              if (information.productFamily == FIRMWARE_FAMILY_INTERNAL_MODULE)
                POPUP_MENU_ADD_ITEM(STR_FLASH_INTERNAL_MODULE);
#endif
              if (information.productFamily == FIRMWARE_FAMILY_EXTERNAL_MODULE)
                POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);
              if (information.productFamily == FIRMWARE_FAMILY_RECEIVER || information.productFamily == FIRMWARE_FAMILY_SENSOR) {
                if (HAS_SPORT_UPDATE_CONNECTOR())
                  POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_DEVICE);
                else
                  POPUP_MENU_ADD_ITEM(STR_FLASH_EXTERNAL_MODULE);
              }
#if defined(PXX2)
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
#if defined(HARDWARE_POWER_MANAGEMENT_UNIT)
              if (information.productFamily == FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT)
                POPUP_MENU_ADD_ITEM(STR_FLASH_POWER_MANAGEMENT_UNIT);
#endif
            }
          }
#endif
          if (isExtensionMatching(ext, TEXT_EXT) || isExtensionMatching(ext, SCRIPTS_EXT)) {
            POPUP_MENU_ADD_ITEM(STR_VIEW_TEXT);
          }
        }
        if (!READ_ONLY()) {
          if (IS_FILE(line))
            POPUP_MENU_ADD_ITEM(STR_COPY_FILE);
          if (clipboard.type == CLIPBOARD_TYPE_SD_FILE)
            POPUP_MENU_ADD_ITEM(STR_PASTE);
          POPUP_MENU_ADD_ITEM(STR_RENAME_FILE);
          if (IS_FILE(line))
            POPUP_MENU_ADD_ITEM(STR_DELETE_FILE);
        }
        POPUP_MENU_START(onSdManagerMenu);
      }
      break;
  }

  if (SD_CARD_PRESENT()) {
    if (reusableBuffer.sdManager.offset != menuVerticalOffset) {
      VfsFileInfo fno;
      VfsDir dir;

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

      VfsError res = VirtualFS::instance().openDirectory(dir, "."); // Open the directory
      if (res == VfsError::OK) {
        bool firstTime = true;
        for (;;) {
          res = dir.read(fno);
          std::string name = fno.getName();
          if (res != VfsError::OK || name.length() == 0) break;              /* Break on error or end of dir */
          if (name.length() > SD_SCREEN_FILE_LENGTH) continue;
//          if (fno.fattrib & AM_HID) continue;                        /* Ignore Windows hidden files */
          if (name[0] == '.' && name[1] != '.') continue;  /* Ignore UNIX hidden files, but not .. */

          reusableBuffer.sdManager.count++;

          bool isfile = (fno.getType() != VfsType::DIR);

          if (menuVerticalOffset == 0) {
            for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
              char * line = reusableBuffer.sdManager.lines[i];
              if (line[0] == '\0' || isFilenameLower(isfile, name.c_str(), line)) {
                if (i < NUM_BODY_LINES-1) memmove(reusableBuffer.sdManager.lines[i+1], line, sizeof(reusableBuffer.sdManager.lines[i]) * (NUM_BODY_LINES-1-i));
                memset(line, 0, sizeof(reusableBuffer.sdManager.lines[0]));
                strcpy(line, name.c_str());
                NODE_TYPE(line) = isfile;
                break;
              }
            }
          }
          else if (reusableBuffer.sdManager.offset == menuVerticalOffset) {
            for (int8_t i=NUM_BODY_LINES-1; i>=0; i--) {
              char * line = reusableBuffer.sdManager.lines[i];
              if (line[0] == '\0' || isFilenameGreater(isfile, name.c_str(), line)) {
                if (i > 0) memmove(reusableBuffer.sdManager.lines[0], reusableBuffer.sdManager.lines[1], sizeof(reusableBuffer.sdManager.lines[0]) * i);
                memset(line, 0, sizeof(reusableBuffer.sdManager.lines[0]));
                strcpy(line, name.c_str());
                NODE_TYPE(line) = isfile;
                break;
              }
            }
          }
          else if (menuVerticalOffset > reusableBuffer.sdManager.offset) {
            if (isFilenameGreater(isfile, name.c_str(), reusableBuffer.sdManager.lines[NUM_BODY_LINES-2]) && isFilenameLower(isfile, name.c_str(), reusableBuffer.sdManager.lines[NUM_BODY_LINES-1])) {
              memset(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1], 0, sizeof(reusableBuffer.sdManager.lines[0]));
              strcpy(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1], name.c_str());
              NODE_TYPE(reusableBuffer.sdManager.lines[NUM_BODY_LINES-1]) = isfile;
            }
          }
          else {
            if (isFilenameLower(isfile, name.c_str(), reusableBuffer.sdManager.lines[1]) && isFilenameGreater(isfile, name.c_str(), reusableBuffer.sdManager.lines[0])) {
              memset(reusableBuffer.sdManager.lines[0], 0, sizeof(reusableBuffer.sdManager.lines[0]));
              strcpy(reusableBuffer.sdManager.lines[0], name.c_str());
              NODE_TYPE(reusableBuffer.sdManager.lines[0]) = isfile;
            }
          }
        }
        dir.close();
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
