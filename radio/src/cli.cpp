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

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/stream_buffer.h>

#include "hal/adc_driver.h"
#include "hal/audio_driver.h"
#include "hal/fatfs_diskio.h"
#include "hal/module_port.h"
#include "hal/serial_driver.h"
#include "hal/storage.h"
#include "hal/usb_driver.h"
#include "hal/watchdog_driver.h"

#include "edgetx.h"
#include "os/sleep.h"
#include "os/time.h"
#include "os/task.h"
#include "timers_driver.h"

#if defined(BLUETOOTH)
#include "bluetooth_driver.h"
#endif

#include "tasks.h"
#include "tasks/mixer_task.h"

#include "cli.h"

#include <ctype.h>
#include <malloc.h>
#include <new>
#include <stdarg.h>

#include "lua/lua_states.h"

#define CLI_COMMAND_MAX_ARGS           8
#define CLI_COMMAND_MAX_LEN            256

// CLI receive buffer size
#define CLI_RX_BUFFER_SIZE 256

#define CLI_PRINT_BUFFER_SIZE 128

task_handle_t cliTaskId;
TASK_DEFINE_STACK(cliStack, CLI_STACK_SIZE);

static uint8_t cliRxBufferStorage[CLI_RX_BUFFER_SIZE];
static StaticStreamBuffer_t cliRxBufferStatic;

// CLI receive stream buffer
static StreamBufferHandle_t cliRxBuffer;

// CLI receive call back
void (*cliReceiveCallBack)(uint8_t* buf, uint32_t len) = nullptr;

// CLI send call back
static void (*cliSendCb)(void*, uint8_t) = nullptr;
static void* cliSendCtx = nullptr;

static const etx_serial_driver_t* cliSerialDriver = nullptr;
static void* cliSerialDriverCtx = nullptr;

static uint8_t cliTracesEnabled = false;
static void (*cliTracesOldCb)(void*, uint8_t);
static void* cliTracesOldCbCtx;

void cliSetSendCb(void* ctx, void (*cb)(void*, uint8_t))
{
  cliSendCb = nullptr;
  cliSendCtx = ctx;
  cliSendCb = cb;
}

// Called from receive ISR (either USB or UART)
static void cliReceiveData(uint8_t* buf, uint32_t len)
{
  if (cliReceiveCallBack)
    cliReceiveCallBack(buf, len);
}

// Assumes it is called from ISR...
static void cliDefaultRx(uint8_t *buf, uint32_t len)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xStreamBufferSendFromISR(cliRxBuffer, buf, len, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void cliEnableDbg()
{
  if (dbgSerialGetSendCb() != cliSendCb) {
    cliTracesOldCb = dbgSerialGetSendCb();
    cliTracesOldCbCtx = dbgSerialGetSendCbCtx();
    dbgSerialSetSendCb(nullptr, nullptr);
    dbgSerialSetSendCb(cliSendCtx, cliSendCb);
  }
}

static void cliDisableDbg()
{
  if (dbgSerialGetSendCb() == cliSendCb) {
    dbgSerialSetSendCb(nullptr, nullptr);
    dbgSerialSetSendCb(cliTracesOldCbCtx, cliTracesOldCb);
  }
}

void cliSetSerialDriver(void* ctx, const etx_serial_driver_t* drv)
{
  cliSerialDriver = nullptr;
  cliSerialDriverCtx = ctx;
  cliSerialDriver = drv;

  if (drv) {
    if (drv->setReceiveCb) drv->setReceiveCb(ctx, cliReceiveData);
    cliSetSendCb(ctx, drv->sendByte);
    if (cliTracesEnabled) {
      cliEnableDbg();
    }
  } else {
    cliSetSendCb(nullptr, nullptr);
    cliDisableDbg();
  }
}

static void cliSerialPrintf(const char * format, ...)
{
  va_list arglist;
  char tmp[CLI_PRINT_BUFFER_SIZE];

  // no need to do anything if we don't have an output
  if (!cliSendCb) return;
  
  va_start(arglist, format);
  vsnprintf(tmp, CLI_PRINT_BUFFER_SIZE-1, format, arglist);
  tmp[CLI_PRINT_BUFFER_SIZE-1] = '\0';
  va_end(arglist);

  const char *t = tmp;
  while (*t && cliSendCb) {
    cliSendCb(cliSendCtx, *t++);
  }
}

#define cliSerialPrint(...) \
  do { cliSerialPrintf(__VA_ARGS__); cliSerialCrlf(); } while(0)

static void cliSerialPutc(uint8_t c)
{
  if (cliSendCb) cliSendCb(cliSendCtx, c);
}

static void cliSerialCrlf()
{
  cliSerialPutc('\r');
  cliSerialPutc('\n');
}

static uint32_t cliGetBaudRate()
{
  auto drv = cliSerialDriver;
  auto ctx = cliSerialDriverCtx;

  if (drv && drv->getBaudrate) {
    return drv->getBaudrate(ctx);
  }

  return 0;
}


char cliLastLine[CLI_COMMAND_MAX_LEN+1];

typedef int (* CliFunction) (const char ** args);
static int cliExecLine(char * line);
static int cliExecCommand(const char ** argv);
static int cliHelp(const char ** argv);

struct CliCommand
{
  const char * name;
  CliFunction func;
  const char * args;
};

struct MemArea
{
  const char * name;
  void * start;
  int size;
};

void cliPrompt()
{
  cliSerialPutc('>');
  cliSerialPutc(' ');
}

int toLongLongInt(const char ** argv, int index, long long int * val)
{
  if (*argv[index] == '\0') {
    return 0;
  }
  else {
    int base = 10;
    const char * s = argv[index];
    if (strlen(s) > 2 && s[0] == '0' && s[1] == 'x') {
      base = 16;
      s = &argv[index][2];
    }
    char * endptr = nullptr;
    *val = strtoll(s, &endptr, base);
    if (*endptr == '\0')
      return 1;
    else {
      cliSerialPrint("%s: Invalid argument \"%s\"", argv[0], argv[index]);
      return -1;
    }
  }
}

int toInt(const char ** argv, int index, int * val)
{
  long long int lval = 0;
  int result = toLongLongInt(argv, index, &lval);
  *val = (int)lval;
  return result;
}

int cliBeep(const char ** argv)
{
  int freq = BEEP_DEFAULT_FREQ;
  int duration = 100;
  if (toInt(argv, 1, &freq) >= 0 && toInt(argv, 2, &duration) >= 0) {
    audioQueue.playTone(freq, duration, 20, PLAY_NOW);
  }
  return 0;
}

int cliPlay(const char ** argv)
{
  audioQueue.playFile(argv[1], PLAY_NOW);
  return 0;
}

int cliLs(const char ** argv)
{
  FILINFO fno;
  DIR dir;

  FRESULT res = f_opendir(&dir, argv[1]);        /* Open the directory */
  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      cliSerialPrint(fno.fname);
    }
    f_closedir(&dir);
  }
  else {
    cliSerialPrint("%s: Invalid directory \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliRead(const char ** argv)
{
  FIL file;
  uint32_t bytesRead = 0;
  int bufferSize;
  if (toInt(argv, 2, &bufferSize) == 0 || bufferSize < 0 ) {
    cliSerialPrint("%s: Invalid buffer size \"%s\"", argv[0], argv[2]);
    return 0;
  }

  uint8_t * buffer = (uint8_t*) malloc(bufferSize);
  if (!buffer) {
    cliSerialPrint("Not enough memory");
    return 0;
  }

  FRESULT result = f_open(&file, argv[1], FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    free(buffer);
    cliSerialPrint("%s: File not found \"%s\"", argv[0], argv[1]);
    return 0;
  }

  tmr10ms_t start = get_tmr10ms();

  while (true) {
    UINT read;
    result = f_read(&file, buffer, sizeof(buffer), &read);
    if (result == FR_OK) {
      if (read == 0) {
        // end of file
        f_close(&file);
        break;
      }
      bytesRead += read;
    }
  }
  uint32_t elapsedTime = (get_tmr10ms() - start) * 10;
  if (elapsedTime == 0) elapsedTime = 1;
  uint32_t speed = bytesRead / elapsedTime;
  cliSerialPrint("Read %d bytes in %d ms, speed %d kB/s", bytesRead, elapsedTime, speed);
  free(buffer);
  return 0;
}

int cliReadSD(const char ** argv)
{
  int startSector;
  int numberOfSectors;
  int bufferSectors;
  if (toInt(argv, 1, &startSector) == 0 || startSector < 0 ) {
    cliSerialPrint("%s: Invalid start sector \"%s\"", argv[0], argv[1]);
    return 0;
  }
  if (toInt(argv, 2, &numberOfSectors) == 0 || numberOfSectors < 0 ) {
    cliSerialPrint("%s: Invalid number of sectors \"%s\"", argv[0], argv[2]);
    return 0;
  }

  if (toInt(argv, 3, &bufferSectors) == 0 || bufferSectors < 0 ) {
    cliSerialPrint("%s: Invalid number of buffer sectors \"%s\"", argv[0], argv[3]);
    return 0;
  }

  uint8_t * buffer = (uint8_t*) malloc(512*bufferSectors);
  if (!buffer) {
    cliSerialPrint("Not enough memory");
    return 0;
  }

  uint32_t bytesRead = numberOfSectors * 512;
  tmr10ms_t start = get_tmr10ms();

  auto drv = storageGetDefaultDriver();
  while (numberOfSectors > 0) {
    DRESULT res = drv->read(0, buffer, startSector, bufferSectors);
    if (res != RES_OK) {
      cliSerialPrint("disk_read error: %d, sector: %d(%d)", res, startSector, numberOfSectors);
    }
#if 0
    for(uint32_t n=0; n<bufferSectors; ++n) {
      dump(buffer + n*512, 32);
    }
#endif
#if 0
    // calc checksumm
    uint32_t summ = 0;
    for(int n=0; n<(bufferSectors*512); ++n) {
      summ += buffer[n];
    }
    cliSerialPrint("sector %d(%d) checksumm: %u", startSector, numberOfSectors, summ);
#endif
    if (numberOfSectors >= bufferSectors) {
      numberOfSectors -= bufferSectors;
      startSector += bufferSectors;
    }
    else {
      numberOfSectors = 0;
    }
  }

  uint32_t elapsedTime = (get_tmr10ms() - start) * 10;
  if (elapsedTime == 0) elapsedTime = 1;
  uint32_t speed = bytesRead / elapsedTime;
  cliSerialPrint("Read %d bytes in %d ms, speed %d kB/s", bytesRead, elapsedTime, speed);
  free(buffer);
  return 0;
}

int cliTestSD(const char ** argv)
{
  // Do the read test on the SD card and report back the result
  auto drv = storageGetDefaultDriver();

  // get sector count
  uint32_t sectorCount;
  if (drv->ioctl(0, GET_SECTOR_COUNT, &sectorCount) != RES_OK) {
    cliSerialPrint("Error: can't read sector count");
    return 0;
  }
  cliSerialPrint("SD card has %u sectors", sectorCount);

  // read last 16 sectors one sector at the time
  cliSerialPrint("Starting single sector read test, reading 16 sectors one by one");
  uint8_t * buffer = (uint8_t*) malloc(512);
  if (!buffer) {
    cliSerialPrint("Not enough memory");
    return 0;
  }

  for (uint32_t s = sectorCount - 16; s<sectorCount; ++s) {
    DRESULT res = drv->read(0, buffer, s, 1);
    if (res != RES_OK) {
      cliSerialPrint("sector %d read FAILED, err: %d", s, res);
    }
    else {
      cliSerialPrint("sector %d read OK", s);
    }
  }
  free(buffer);
  cliSerialCrlf();

  // read last 16 sectors, two sectors at the time with a multi-block read
  buffer = (uint8_t *) malloc(512*2);
  if (!buffer) {
    cliSerialPrint("Not enough memory");
    return 0;
  }

  cliSerialPrint("Starting multiple sector read test, reading two sectors at the time");
  for (uint32_t s = sectorCount - 16; s < sectorCount; s += 2) {
    DRESULT res = drv->read(0, buffer, s, 2);
    if (res != RES_OK) {
      cliSerialPrint("sector %d-%d read FAILED, err: %d", s, s+1, res);
    }
    else {
      cliSerialPrint("sector %d-%d read OK", s, s+1);
    }
  }
  free(buffer);
  cliSerialCrlf();

  // read last 16 sectors, all sectors with single multi-block read
  buffer = (uint8_t*) malloc(512*16);
  if (!buffer) {
    cliSerialPrint("Not enough memory");
    return 0;
  }

  cliSerialPrint("Starting multiple sector read test, reading 16 sectors at the time");
  DRESULT res = drv->read(0, buffer, sectorCount - 16, 16);
  if (res != RES_OK) {
    cliSerialPrint("sector %d-%d read FAILED, err: %d", sectorCount-16, sectorCount-1, res);
  }
  else {
    cliSerialPrint("sector %d-%d read OK", sectorCount-16, sectorCount-1);
  }
  free(buffer);
  cliSerialCrlf();

  return 0;
}

int cliTestNew()
{
  char * tmp = nullptr;
  cliSerialPrint("Allocating 1kB with new()");
  sleep_ms(200);
  tmp = new char[1024];
  if (tmp) {
    cliSerialPrint("\tsuccess");
    delete[] tmp;
    tmp = nullptr;
  }
  else {
    cliSerialPrint("\tFAILURE");
  }

  cliSerialPrint("Allocating 10MB with (std::nothrow) new()");
  sleep_ms(200);
  tmp = new (std::nothrow) char[1024*1024*10];
  if (tmp) {
    cliSerialPrint("\tFAILURE, tmp = %p", tmp);
    delete[] tmp;
    tmp = nullptr;
  }
  else {
    cliSerialPrint("\tsuccess, allocaton failed, tmp = 0");
  }

  cliSerialPrint("Allocating 10MB with new()");
  sleep_ms(200);
  tmp = new char[1024*1024*10];
  if (tmp) {
    cliSerialPrint("\tFAILURE, tmp = %p", tmp);
    delete[] tmp;
    tmp = nullptr;
  }
  else {
    cliSerialPrint("\tsuccess, allocaton failed, tmp = 0");
  }
  cliSerialPrint("Test finished");
  return 0;
}

#if defined(COLORLCD)

extern bool perMainEnabled;
typedef void (*timedTestFunc_t)(void);

#if defined(DEBUG_LCD)
void testDrawSolidFilledRectangle()
{
  lcdDrawFilledRect(0, 0, LCD_W, LCD_H, SOLID, COLOR_THEME_SECONDARY3);
}

void testDrawFilledRectangle()
{
  lcdDrawFilledRect(0, 0, LCD_W, LCD_H, DOTTED, COLOR_THEME_SECONDARY3);
}

void testDrawSolidFilledRoundedRectangle()
{
  lcdDrawFilledRect(0, 0, LCD_W / 2, LCD_H / 2, SOLID,
                    ROUND | COLOR_THEME_SECONDARY3);
}

void testDrawSolidHorizontalLine1() { lcdDrawSolidHorizontalLine(0, 0, 1, 0); }

void testDrawSolidHorizontalLine2()
{
  lcdDrawSolidHorizontalLine(0, 0, LCD_W, 0);
}

void testDrawSolidVerticalLine1() { lcdDrawSolidVerticalLine(0, 0, 1, 0); }

void testDrawSolidVerticalLine2() { lcdDrawSolidVerticalLine(0, 0, LCD_H, 0); }

void testDrawDiagonalLine()
{
  lcdDrawLine(0, 0, LCD_W, LCD_H, SOLID, COLOR_THEME_SECONDARY1);
}

void testEmpty() {}

void testDrawRect()
{
  lcdDrawRect(0, 0, LCD_W, LCD_H, 2, SOLID, COLOR_THEME_SECONDARY1);
}

void testDrawText()
{
  lcdDrawText(0, LCD_H / 2, "The quick brown fox jumps over the lazy dog",
              COLOR_THEME_SECONDARY1);
}

void testDrawTextVertical()
{
  lcdDrawText(30, LCD_H, "The quick brown fox ",
              COLOR_THEME_SECONDARY1 | VERTICAL);
}

void testClear() { lcdClear(); }

#define RUN_GRAPHICS_TEST(name, runtime) \
  runTimedFunctionTest(name, #name, runtime, 100)

float runTimedFunctionTest(timedTestFunc_t func, const char *name,
                           uint32_t runtime, uint16_t step)
{
  const uint32_t start = time_get_ms();
  uint32_t noRuns = 0;
  uint32_t actualRuntime = 0;
  while ((actualRuntime = time_get_ms() - start) < runtime) {
    for (uint16_t n = 0; n < step; n++) {
      func();
    }
    lcdRefresh();
    noRuns += step;
  }
  const float result = (noRuns * 500.0f) / (float)actualRuntime;  // runs/second
  cliSerialPrint("Test %s speed: %lu.%02u, (%lu runs in %lu ms)", name,
              uint32_t(result), uint16_t((result - uint32_t(result)) * 100.0f),
              noRuns, actualRuntime);
  sleep_ms(200);
  return result;
}

int cliTestGraphics()
{
  cliSerialPrint("Starting graphics performance test...");
  sleep_ms(200);

  watchdogSuspend(6000 /*60s*/);
  if (pulsesStarted()) {
    pausePulses();
  }
  mixerTaskStop();
  perMainEnabled = false;

  float result = 0;
  RUN_GRAPHICS_TEST(testEmpty, 1000);
  // result += RUN_GRAPHICS_TEST(testDrawSolidHorizontalLine1, 1000);
  result += RUN_GRAPHICS_TEST(testDrawSolidHorizontalLine2, 1000);
  // result += RUN_GRAPHICS_TEST(testDrawSolidVerticalLine1, 1000);
  result += RUN_GRAPHICS_TEST(testDrawSolidVerticalLine2, 1000);
  result += RUN_GRAPHICS_TEST(testDrawDiagonalLine, 1000);
  result += RUN_GRAPHICS_TEST(testDrawSolidFilledRectangle, 1000);
  result += RUN_GRAPHICS_TEST(testDrawSolidFilledRoundedRectangle, 1000);
  result += RUN_GRAPHICS_TEST(testDrawRect, 1000);
  result += RUN_GRAPHICS_TEST(testDrawFilledRectangle, 1000);
  result += RUN_GRAPHICS_TEST(testDrawBlackOverlay, 1000);
  result += RUN_GRAPHICS_TEST(testDrawText, 1000);
  result += RUN_GRAPHICS_TEST(testDrawTextVertical, 1000);
  result += RUN_GRAPHICS_TEST(testClear, 1000);

  cliSerialPrint("Total speed: %lu.%02u", uint32_t(result),
              uint16_t((result - uint32_t(result)) * 100.0f));

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  mixerTaskStart();
  watchdogSuspend(0);

  return 0;
}
#endif

#if defined(DEBUG_RAM)
void memoryRead(const uint8_t *src, uint32_t size)
{
  // uint8_t data;
  while (size--) {
    /*data =*/*(const uint8_t volatile *)src;
    ++src;
  }
}

void memoryRead(const uint32_t *src, uint32_t size)
{
  while (size--) {
    *(const uint32_t volatile *)src;
    ++src;
  }
}

uint32_t *testbuff[100];

void memoryCopy(uint8_t *dest, const uint8_t *src, uint32_t size)
{
  while (size--) {
    *dest = *src;
    ++src;
    ++dest;
  }
}

void memoryCopy(uint32_t *dest, const uint32_t *src, uint32_t size)
{
  while (size--) {
    *dest = *src;
    ++src;
    ++dest;
  }
}

#define MEMORY_SPEED_BLOCK_SIZE (4 * 1024)

void testMemoryReadFrom_RAM_8bit()
{
  memoryRead((const uint8_t *)cliLastLine, MEMORY_SPEED_BLOCK_SIZE);
}

void testMemoryReadFrom_RAM_32bit()
{
  memoryRead((const uint32_t *)0x20000000, MEMORY_SPEED_BLOCK_SIZE / 4);
}

void testMemoryReadFrom_SDRAM_8bit()
{
  memoryRead((const uint8_t *)0xD0000000, MEMORY_SPEED_BLOCK_SIZE);
}

void testMemoryReadFrom_SDRAM_32bit()
{
  memoryRead((const uint32_t *)0xD0000000, MEMORY_SPEED_BLOCK_SIZE / 4);
}

extern uint8_t *LCD_FIRST_FRAME_BUFFER;
extern uint8_t *LCD_SECOND_FRAME_BUFFER;

void testMemoryCopyFrom_RAM_to_SDRAM_32bit()
{
  memoryCopy((uint32_t *)LCD_FIRST_FRAME_BUFFER, (const uint32_t *)cliLastLine,
             MEMORY_SPEED_BLOCK_SIZE / 4);
}

void testMemoryCopyFrom_RAM_to_SDRAM_8bit()
{
  memoryCopy((uint8_t *)LCD_FIRST_FRAME_BUFFER, (const uint8_t *)cliLastLine,
             MEMORY_SPEED_BLOCK_SIZE);
}

void testMemoryCopyFrom_SDRAM_to_SDRAM_32bit()
{
  memoryCopy((uint32_t *)LCD_FIRST_FRAME_BUFFER,
             (const uint32_t *)LCD_SECOND_FRAME_BUFFER,
             MEMORY_SPEED_BLOCK_SIZE / 4);
}

void testMemoryCopyFrom_SDRAM_to_SDRAM_8bit()
{
  memoryCopy((uint8_t *)LCD_FIRST_FRAME_BUFFER,
             (const uint8_t *)LCD_SECOND_FRAME_BUFFER, MEMORY_SPEED_BLOCK_SIZE);
}

#define RUN_MEMORY_TEST(name, runtime) \
  runTimedFunctionTest(name, #name, runtime, 100)

int cliTestMemorySpeed()
{
  cliSerialPrint("Starting memory speed test...");
  sleep_ms(200);

  watchdogSuspend(6000 /*60s*/);
  if (pulsesStarted()) {
    pausePulses();
  }
  mixerTaskStop();
  perMainEnabled = false;

  float result = 0;
  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_32bit, 200);

  LTDC_Cmd(DISABLE);
  cliSerialPrint("Disabling LCD...");
  sleep_ms(200);

  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_32bit, 200);

  cliSerialPrint("Total speed: %lu.%02u", uint32_t(result),
              uint16_t((result - uint32_t(result)) * 100.0f));

  LTDC_Cmd(ENABLE);

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  mixerTaskStart();
  watchdogSuspend(0);

  return 0;
}
#endif

#if defined(DEBUG_MODELSLIST)
#include "modelslist.h"
using std::list;

int cliTestModelsList()
{
  ModelsList modList;
  modList.load();

  int count = 0;

  cliSerialPrint("Starting fetching RF data 100x...");
  const uint32_t start = time_get_ms();

  const list<ModelsCategory *> &cats = modList.getCategories();
  while (1) {
    for (list<ModelsCategory *>::const_iterator cat_it = cats.begin();
         cat_it != cats.end(); ++cat_it) {
      for (ModelsCategory::iterator mod_it = (*cat_it)->begin();
           mod_it != (*cat_it)->end(); mod_it++) {
        if (!(*mod_it)->fetchRfData()) {
          cliSerialPrint("Error while fetching RF data...");
          return 0;
        }

        if (++count >= 100) goto done;
      }
    }
  }

done:
  cliSerialPrint("Done fetching %ix RF data: %lu ms", count,
              (time_get_ms() - start));

  return 0;
}
#endif

#endif  // #if defined(COLORLCD)

#if defined(DEBUG)
int cliTest(const char ** argv)
{
  if (!strcmp(argv[1], "new")) {
    return cliTestNew();
  }
#if defined(COLORLCD)
#if defined(DEBUG_LCD)
  else if (!strcmp(argv[1], "graphics")) {
    return cliTestGraphics();
  }
#endif
#if defined(DEBUG_RAM)
  else if (!strcmp(argv[1], "memspd")) {
    return cliTestMemorySpeed();
  }
#endif
#if defined(DEBUG_MODELSLIST)
  else if (!strcmp(argv[1], "modelslist")) {
    return cliTestModelsList();
  }
#endif
#endif
  else {
    cliSerialPrint("%s: Invalid argument \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliTrace(const char ** argv)
{
  if (!strcmp(argv[1], "on")) {
    cliEnableDbg();
    cliTracesEnabled = true;
  }
  else if (!strcmp(argv[1], "off")) {
    cliTracesEnabled = false;
    cliDisableDbg();
  }
  else {
    cliSerialPrint("%s: Invalid argument \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliStackInfo(const char ** argv)
{
  cliSerialPrint("[MENUS] %d available / %d bytes",
                 task_get_stack_usage(&menusTaskId) * 4,
                 task_get_stack_size(&menusTaskId));
  cliSerialPrint("[MIXER] %d available / %d bytes",
                 task_get_stack_usage(&mixerTaskId) * 4,
                 task_get_stack_size(&mixerTaskId));
#if defined(AUDIO)
  cliSerialPrint("[AUDIO] %d available / %d bytes",
                 task_get_stack_usage(&audioTaskId) * 4,
                 task_get_stack_size(&audioTaskId));
#endif
#if defined(CLI)
  cliSerialPrint("[CLI] %d available / %d bytes",
                 task_get_stack_usage(&cliTaskId),
                 task_get_stack_size(&cliTaskId));
#endif
  return 0;
}

extern int _heap_start;
extern int _heap_end;
extern unsigned char *heap;

int cliMemoryInfo(const char ** argv)
{
  // struct mallinfo {
  //   int arena;    /* total space allocated from system */
  //   int ordblks;  /* number of non-inuse chunks */
  //   int smblks;   /* unused -- always zero */
  //   int hblks;    /* number of mmapped regions */
  //   int hblkhd;   /* total space in mmapped regions */
  //   int usmblks;  /* unused -- always zero */
  //   int fsmblks;  /* unused -- always zero */
  //   int uordblks; /* total allocated space */
  //   int fordblks; /* total non-inuse space */
  //   int keepcost; /* top-most, releasable (via malloc_trim) space */
  // };
  struct mallinfo info = mallinfo();
  cliSerialPrint("mallinfo:");
  cliSerialPrint("\tarena    %d bytes", info.arena);
  cliSerialPrint("\tordblks  %d bytes", info.ordblks);
  cliSerialPrint("\tuordblks %d bytes", info.uordblks);
  cliSerialPrint("\tfordblks %d bytes", info.fordblks);
  cliSerialPrint("\tkeepcost %d bytes", info.keepcost);

  cliSerialPrint("\nHeap:");
  cliSerialPrint("\tstart %p", (unsigned char *)&_heap_start);
  cliSerialPrint("\tend   %p", (unsigned char *)&_heap_end);
  cliSerialPrint("\tcurr  %p", heap);
  cliSerialPrint("\tused  %d bytes", (int)(heap - (unsigned char *)&_heap_start));
  cliSerialPrint("\tfree  %d bytes", (int)((unsigned char *)&_heap_end - heap));

#if defined(LUA)
  cliSerialPrint("\nLua:");
  uint32_t s = luaGetMemUsed(lsScripts);
  cliSerialPrint("\tScripts %u", s);
#if defined(COLORLCD)
  uint32_t w = luaGetMemUsed(lsWidgets);
  uint32_t e = luaExtraMemoryUsage;
  cliSerialPrint("\tWidgets %u", w);
  cliSerialPrint("\tExtra   %u", e);
  cliSerialPrint("------------");
  cliSerialPrint("\tTotal   %u", s + w + e);
#endif
#endif
  return 0;
}
#endif

int cliReboot(const char ** argv)
{
#if !defined(SIMU)
  if (!strcmp(argv[1], "wdt")) {
    // do a user requested watchdog test by pausing mixer thread
    pulsesStop();
  }
  else {
    NVIC_SystemReset();
  }
#endif
  return 0;
}

const MemArea memAreas[] = {
  { "RCC", RCC, sizeof(RCC_TypeDef) },
  { "GPIOA", GPIOA, sizeof(GPIO_TypeDef) },
  { "GPIOB", GPIOB, sizeof(GPIO_TypeDef) },
  { "GPIOC", GPIOC, sizeof(GPIO_TypeDef) },
  { "GPIOD", GPIOD, sizeof(GPIO_TypeDef) },
  { "GPIOE", GPIOE, sizeof(GPIO_TypeDef) },
  { "GPIOF", GPIOF, sizeof(GPIO_TypeDef) },
  { "GPIOG", GPIOG, sizeof(GPIO_TypeDef) },
  { "USART1", USART1, sizeof(USART_TypeDef) },
  { "USART2", USART2, sizeof(USART_TypeDef) },
  { "USART3", USART3, sizeof(USART_TypeDef) },
  { nullptr, nullptr, 0 },
};

int cliSet(const char **argv)
{
  if (!strcmp(argv[1], "rtc")) {
    struct gtm t;
    int year, month, day, hour, minute, second;
    if (toInt(argv, 2, &year) > 0 && toInt(argv, 3, &month) > 0 &&
        toInt(argv, 4, &day) > 0 && toInt(argv, 5, &hour) > 0 &&
        toInt(argv, 6, &minute) > 0 && toInt(argv, 7, &second) > 0) {
      t.tm_year = year - TM_YEAR_BASE;
      t.tm_mon = month - 1;
      t.tm_mday = day;
      t.tm_hour = hour;
      t.tm_min = minute;
      t.tm_sec = second;
      // update local timestamp and get wday calculated
      g_rtcTime = gmktime(&t);
      rtcSetTime(&t);
    } else {
      cliSerialPrint("%s: Invalid arguments \"%s\" \"%s\"", argv[0], argv[1],
                  argv[2]);
      return -1;
    }
  }
#if !defined(SOFTWARE_VOLUME) && defined(AUDIO)
  else if (!strcmp(argv[1], "volume")) {
    int level = 0;
    if (toInt(argv, 2, &level) > 0) {
      audioSetVolume(level);
    } else {
      cliSerialPrint("%s: Invalid argument \"%s\" \"%s\"", argv[0], argv[1],
                  argv[2]);
      return -1;
    }
  }
#endif
  else if (!strcmp(argv[1], "rfmod")) {
    int module = 0;
    if (toInt(argv, 2, &module) < 0) {
      cliSerialPrint("%s: invalid module argument '%s'", argv[0], argv[2]);
      return -1;
    }
    if (!strcmp(argv[3], "power")) {
      if (!strcmp("on", argv[4])) {
        modulePortSetPower(module, true);
      } else if (!strcmp("off", argv[4])) {
        modulePortSetPower(module, false);
      } else {
        cliSerialPrint("%s: invalid power argument '%s'", argv[0], argv[4]);
        return -1;
      }
      cliSerialPrint("%s: rfmod %d power %s", argv[0], module, argv[4]);
    }
    else if (!strcmp(argv[3], "bootpin")) {
      int level = 0;
      if (toInt(argv, 4, &level) < 0) {
        cliSerialPrint("%s: invalid bootpin argument '%s'", argv[0], argv[4]);
        return -1;
      }
      const auto* mod = modulePortGetModuleDescription(INTERNAL_MODULE);
      if (!mod || !mod->set_bootcmd) {
        cliSerialPrint("%s: invalid module or has no bootcmd pin", argv[0]);
        return -1;
      }
      mod->set_bootcmd(level);
      if (level) {
        cliSerialPrint("%s: bootcmd set", argv[0]);
      } else {
        cliSerialPrint("%s: bootcmd reset", argv[0]);
      }
    }
    else {
      if (strlen(argv[2]) == 0) {
        cliSerialPrint("%s: missing rfmod arguments", argv[0]);
      } else {
        cliSerialPrint("%s: invalid rfmod argument '%s'", argv[0], argv[3]);
        return -1;
      }
    }
  }
  else if (!strcmp(argv[1], "pulses")) {
    int level = 0;
    if (toInt(argv, 2, &level) < 0) {
      cliSerialPrint("%s: invalid level argument '%s'", argv[0], argv[2]);
      return -1;
    }

    if (level) {
      cliSerialPrint("%s: pulses start", argv[0]);
      pulsesStart();
    } else {
      cliSerialPrint("%s: pulses stop", argv[0]);
      pulsesStop();
    }
  }

  return 0;
}

#if defined(ENABLE_SERIAL_PASSTHROUGH)

static const etx_serial_driver_t* _sp_drv = nullptr;
static void* _sp_ctx = nullptr;

static void _sp_Tx(uint8_t* buf, uint32_t len)
{
  while (len > 0) {
    _sp_drv->sendByte(_sp_ctx, *(buf++));
    len--;
  }
}

#if defined(HARDWARE_INTERNAL_MODULE) || defined(HARDWARE_EXTERNAL_MODULE)
static etx_module_state_t *spModuleState = nullptr;

static void spModuleInit(int port_n, int baudrate)
{
  etx_serial_init params = {
      .baudrate = (uint32_t)baudrate,
      .encoding = ETX_Encoding_8N1,
      .direction = ETX_Dir_TX_RX,
      .polarity = ETX_Pol_Normal,
  };

  spModuleState =
      modulePortInitSerial(port_n, ETX_MOD_PORT_UART, &params, false);
  _sp_drv = modulePortGetSerialDrv(spModuleState->rx);
  _sp_ctx = modulePortGetCtx(spModuleState->rx);
}

static void spModuleDeInit(int port_n)
{
  // and stop module
  modulePortDeInit(spModuleState);
  spModuleState = nullptr;

  // power off the module and wait for a bit
  modulePortSetPower(port_n, false);
  delay_ms(200);
}

#endif // HARDWARE_INTERNAL_MODULE

#if defined(FLYSKY_GIMBAL)
#include "flysky_gimbal_driver.h"

static void spGimbalInit(int port_n, int baudrate)
{
  etx_serial_init params = {
      .baudrate = (uint32_t)baudrate,
      .encoding = ETX_Encoding_8N1,
      .direction = ETX_Dir_TX_RX,
      .polarity = ETX_Pol_Normal,
  };

  (void)port_n;
  flysky_gimbal_deinit();

  auto port = flysky_gimbal_get_port();
  _sp_drv = port->uart;
  _sp_ctx = _sp_drv->init(port->hw_def, &params);
}

static void spGimbalDeInit(int port_n)
{
  (void)port_n;
  _sp_drv->deinit(_sp_ctx);
  flysky_gimbal_force_init();
}
#endif

// TODO: use proper method instead
extern bool cdcConnected;
extern uint32_t usbSerialBaudRate(void*);

int cliSerialPassthrough(const char **argv)
{
  const char* port_type = argv[1];

  if (!port_type) {
    cliSerialPrint("%s: missing argument", argv[0]);
    return -1;
  }

  int baudrate = 0;
  void (*initCB)(int, int) = nullptr;
  void (*deinitCB)(int) = nullptr;

  int port_n = 0;
  if (!strcmp("rfmod", port_type)) {
    // parse port number
    if (toInt(argv, 2, &port_n) <= 0) {
      cliSerialPrint("%s: invalid or missing port number", argv[0]);
      return -1;
    }
    // 3rd argument (baudrate is optional)
    if (toInt(argv, 3, &baudrate) <= 0) {
      baudrate = 0;
    }

    switch(port_n) {
#if defined(HARDWARE_INTERNAL_MODULE)
    case INTERNAL_MODULE:
      initCB = spModuleInit;
      deinitCB = spModuleDeInit;
      break;
#endif
    // TODO:
    //  - external module (S.PORT?)
    default:
      cliSerialPrint("%s: invalid port # '%i'", port_n);
      return -1;
    }
  } else if (!strcmp("gimbals", port_type)) {
#if defined(FLYSKY_GIMBAL)
    // 2nd argument (baudrate is optional)
    if (toInt(argv, 2, &baudrate) <= 0) {
      baudrate = 0;
    }

    initCB = spGimbalInit;
    deinitCB = spGimbalDeInit;
#else
    cliSerialPrint("%s: serial gimbals not supported");
    return -1;
#endif
  } else {
    cliSerialPrint("%s: invalid port type '%s'", port_type);
    return -1;
  }

  //  stop pulses
  watchdogSuspend(200/*2s*/);
  pulsesStop();

  // suspend RTOS scheduler
  vTaskSuspendAll();

  if (baudrate <= 0) {
    // use current baudrate
    baudrate = cliGetBaudRate();
    if (!baudrate) {
      baudrate = 115200;
      cliSerialPrint("%s: baudrate is 0, default to 115200", argv[0]);
    }
  }

  if (initCB) initCB(port_n, baudrate);

  if (_sp_drv != nullptr) {
    auto backupCB = cliReceiveCallBack;
    cliReceiveCallBack = _sp_Tx;

    // loop until cable disconnected
    while (usbPlugged()) {

      uint32_t cli_br = cliGetBaudRate();
      if (cli_br && (cli_br != (uint32_t)baudrate)) {
        baudrate = cli_br;
        _sp_drv->setBaudrate(_sp_ctx, baudrate);
      }

      uint8_t data;
      if (_sp_drv->getByte(_sp_ctx, &data) > 0) {
        uint8_t timeout = 10; // 10 ms
        while(!usbSerialFreeSpace() && (timeout > 0)) {
          delay_ms(1);
          timeout--;
        }

        cliSerialPutc(data);
      }

      // keep us up & running
      WDG_RESET();
    }

    // restore callsbacks
    cliReceiveCallBack = backupCB;
    if (deinitCB != nullptr) deinitCB(port_n);
  }

  watchdogSuspend(200/*2s*/);
  pulsesStart();

  // suspend RTOS scheduler
  xTaskResumeAll();

  return 0;
}
#endif

#if defined(DEBUG_INTERRUPTS)
void printInterrupts()
{
  __disable_irq();
  struct InterruptCounters ic = interruptCounters;
  memset(&interruptCounters, 0, sizeof(interruptCounters));
  interruptCounters.resetTime = get_tmr10ms();
  __enable_irq();
  cliSerialPrint("Interrupts count in the last %u ms:", (get_tmr10ms() - ic.resetTime) * 10);
  for(int n = 0; n < INT_LAST; n++) {
    cliSerialPrint("%s: %u", interruptNames[n], ic.cnt[n]);
  }
}
#endif //#if defined(DEBUG_INTERRUPTS)

#if defined(DEBUG_TIMERS)
void printDebugTime(uint32_t time)
{
  if (time >= 30000) {
    cliSerialPrintf("%dms", time/1000);
  }
  else {
    cliSerialPrintf("%d.%03dms", time/1000, time%1000);
  }
}

void printDebugTimer(const char * name, DebugTimer & timer)
{
  cliSerialPrintf("%s: ", name);
  printDebugTime( timer.getMin());
  cliSerialPrintf(" - ");
  printDebugTime(timer.getMax());
  cliSerialCrlf();
  timer.reset();
}
void printDebugTimers()
{
  for(int n = 0; n < DEBUG_TIMERS_COUNT; n++) {
    printDebugTimer(debugTimerNames[n], debugTimers[n]);
  }
}
#endif

#if defined(DEBUG_AUDIO)
void printAudioVars()
{
  for (int n = 0; n < AUDIO_BUFFER_COUNT; n++) {
    cliSerialPrint("Audio Buffer %d: size: %u, ", n,
                (uint32_t)audioBuffers[n].size);
    dump((uint8_t *)audioBuffers[n].data, 32);
  }
  cliSerialPrint("fragments:");
  for (int n = 0; n < AUDIO_QUEUE_LENGTH; n++) {
    cliSerialPrint("%d: type %u: id: %u, repeat: %u, ", n,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].type,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].id,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].repeat);
    if (audioQueue.fragmentsFifo.fragments[n].type == FRAGMENT_FILE) {
      cliSerialPrint(" file: %s", audioQueue.fragmentsFifo.fragments[n].file);
    }
  }

  cliSerialPrint("FragmentFifo:  ridx: %d, widx: %d",
              audioQueue.fragmentsFifo.ridx, audioQueue.fragmentsFifo.widx);
  cliSerialPrint("audioQueue:  readIdx: %d, writeIdx: %d, full: %d",
              audioQueue.buffersFifo.readIdx, audioQueue.buffersFifo.writeIdx,
              audioQueue.buffersFifo.bufferFull);

  cliSerialPrint("normalContext: %u",
              (uint32_t)audioQueue.normalContext.fragment.type);
}
#endif

#if defined(DEBUG)

#include "hal/switch_driver.h"

#if defined(DISK_CACHE)
#include "disk_cache.h"
#endif

int cliDisplay(const char ** argv)
{
  long long int address = 0;

  for (const MemArea * area = memAreas; area->name != nullptr; area++) {
    if (!strcmp(area->name, argv[1])) {
      dump((uint8_t *)area->start, area->size);
      return 0;
    }
  }

  if (!strcmp(argv[1], "keys")) {
    for (int i = 0; i <= MAX_KEYS; i++) {
      if (keysGetSupported() & (1 << i)) {
        cliSerialPrint("[Key %s] = %s",
                       keysGetLabel((EnumKeys)i),
                       keysGetState(i) ? "on" : "off");
      }
    }
    for (int i = 0; i < keysGetMaxTrims(); i++) {
      cliSerialPrint("[Trim %s] = %s", getTrimLabel(i),
                     keysGetTrimState(i) ? "on" : "off");
    }
    for (int i = 0; i < switchGetMaxSwitches(); i++) {
      if (SWITCH_EXISTS(i)) {
        static const char * const SWITCH_POSITIONS[] = { "up", "mid", "down" };
        auto pos = switchGetPosition(i);
        cliSerialPrint("[%s] = %s", switchGetName(i), SWITCH_POSITIONS[pos]);
      }
    }
  }
  else if (!strcmp(argv[1], "adc")) {
    for (int i = 0; i < adcGetMaxInputs(ADC_INPUT_ALL); i++) {
      cliSerialPrint("adc[%d] = %04X", i, getAnalogValue(i));
    }
  }
  else if (!strcmp(argv[1], "outputs")) {
    for (int i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
      cliSerialPrint("outputs[%d] = %04d", i, (int)channelOutputs[i]);
    }
  }
  else if (!strcmp(argv[1], "rtc")) {
    struct gtm utm;
    gettime(&utm);
    cliSerialPrint("rtc = %4d-%02d-%02d %02d:%02d:%02d.%02d0", utm.tm_year+TM_YEAR_BASE, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
  }
  else if (!strcmp(argv[1], "uid")) {
    char str[LEN_CPU_UID+1];
    getCPUUniqueID(str);
    cliSerialPrint("uid = %s", str);
  }
  else if (!strcmp(argv[1], "tim")) {
    int timerNumber;
    if (toInt(argv, 2, &timerNumber) > 0) {
      TIM_TypeDef * tim = TIM1;
      switch (timerNumber) {
        case 1:
          tim = TIM1;
          break;
        case 2:
          tim = TIM2;
          break;
#if defined(TIM8)
        case 8:
          tim = TIM8;
          break;
#endif
        case 13:
          tim = TIM13;
          break;
        default:
          return 0;
      }
      cliSerialPrint("TIM%d", timerNumber);
      cliSerialPrint(" CR1    0x%x", tim->CR1);
      cliSerialPrint(" CR2    0x%x", tim->CR2);
      cliSerialPrint(" DIER   0x%x", tim->DIER);
      cliSerialPrint(" SR     0x%x", tim->SR);
      cliSerialPrint(" EGR    0x%x", tim->EGR);
      cliSerialPrint(" CCMR1  0x%x", tim->CCMR1);
      cliSerialPrint(" CCMR2  0x%x", tim->CCMR2);

      cliSerialPrint(" CNT    0x%x", tim->CNT);
      cliSerialPrint(" ARR    0x%x", tim->ARR);
      cliSerialPrint(" PSC    0x%x", tim->PSC);

      cliSerialPrint(" CCER   0x%x", tim->CCER);
      cliSerialPrint(" CCR1   0x%x", tim->CCR1);
      cliSerialPrint(" CCR2   0x%x", tim->CCR2);
      cliSerialPrint(" CCR3   0x%x", tim->CCR3);
      cliSerialPrint(" CCR4   0x%x", tim->CCR4);
    }
  }
#if defined(DMA1_Stream7)
  else if (!strcmp(argv[1], "dma")) {
    cliSerialPrint("DMA1_Stream7");
    cliSerialPrint(" CR    0x%x", DMA1_Stream7->CR);
  }
#endif
#if defined(DEBUG_INTERRUPTS)
  else if (!strcmp(argv[1], "int")) {
    printInterrupts();
  }
#endif
#if defined(DEBUG_TIMERS)
  else if (!strcmp(argv[1], "dt")) {
    printDebugTimers();
  }
#endif
#if defined(DEBUG_AUDIO)
  else if (!strcmp(argv[1], "audio")) {
    printAudioVars();
  }
#endif
#if defined(DISK_CACHE)
  else if (!strcmp(argv[1], "dc")) {
    DiskCacheStats stats = diskCache.getStats();
    uint32_t hitRate = diskCache.getHitRate();
    cliSerialPrint("Disk Cache stats: w:%u r: %u, h: %u(%0.1f%%), m: %u", stats.noWrites, (stats.noHits + stats.noMisses), stats.noHits, hitRate*0.1f, stats.noMisses);
  }
#endif
  else if (toLongLongInt(argv, 1, &address) > 0) {
    int size = 256;
    if (toInt(argv, 2, &size) >= 0) {
      dump((uint8_t *)address, size);
    }
  }
  return 0;
}

int cliDebugVars(const char ** argv)
{
#if defined(INTERNAL_MODULE_PXX2) && defined(ACCESS_DENIED) && !defined(SIMU)
  extern volatile int32_t authenticateFrames;
  cliSerialPrint("authenticateFrames=%d", authenticateFrames);
#elif defined(PCBTARANIS)
  //cliSerialPrint("telemetryErrors=%d", telemetryErrors);
#endif

  return 0;
}
#endif

#if defined(DEBUG)
int cliRepeat(const char ** argv)
{
  int interval = 0;
  int counter = 0;
  if (toInt(argv, 1, &interval) > 0 && argv[2]) {
    interval *= 50;
    counter = interval;

    uint8_t c;
    const TickType_t xTimeout = 20 / portTICK_PERIOD_MS;
    while (!xStreamBufferReceive(cliRxBuffer, &c, 1, xTimeout)
           || !(c == '\r' || c == '\n' || c == ' ')) {

      if (++counter >= interval) {
        cliExecCommand(&argv[2]);
        counter = 0;
      }
    }
  }
  else {
    cliSerialPrint("%s: Invalid arguments", argv[0]);
  }
  return 0;
}
#endif

#if defined(JITTER_MEASURE)
int cliShowJitter(const char ** argv)
{
  cliSerialPrint(  "#   anaIn   rawJ   avgJ");
  for (int i = 0; i < MAX_ANALOG_INPUTS; i++) {
    cliSerialPrint("A%02d %04X %04X %3d %3d", i, getAnalogValue(i), anaIn(i),
                   rawJitter[i].get(), avgJitter[i].get());

    if (i >= MAX_STICKS && IS_POT_MULTIPOS(i - MAX_STICKS)) {
      StepsCalibData *calib = (StepsCalibData *)&g_eeGeneral.calib[i];
      for (int j = 0; j < calib->count; j++) {
        cliSerialPrint("    s%d %04X", j, calib->steps[j]);
      }
    }
  }
  return 0;
}
#endif

#if defined(INTERNAL_GPS)
int cliGps(const char ** argv)
{

  if (argv[1][0] == '$') {
    // send command to GPS
    gpsSendFrame(argv[1]);
  }
#if defined(DEBUG)
  else if (!strcmp(argv[1], "trace")) {
    gpsTraceEnabled = !gpsTraceEnabled;
  }
#endif
  else {
    cliSerialPrint("%s: Invalid arguments", argv[0]);
  }
  return 0;
}
#endif

#if defined(SPACEMOUSE)
int cliSpaceMouse(const char ** argv)
{
#if defined(DEBUG)
  if (!strcmp(argv[1], "trace")) {
    spacemouseTraceEnabled = !spacemouseTraceEnabled;
  } else
#endif
  if (!strcmp(argv[1], "poll")) {
    spacemousePoll();
  } else if (!strcmp(argv[1], "tare")) {
    spacemouseTare();
  } else if (!strcmp(argv[1], "startstreaming")) {
    spacemouseStartStreaming();
  } else if (!strcmp(argv[1], "stopstreaming")) {
    spacemouseStopStreaming();
  }
  return 0;
}
#endif

#if defined(BLUETOOTH)
int cliBlueTooth(const char ** argv)
{
  int baudrate = 0;
  if (!strncmp(argv[1], "AT", 2) || !strncmp(argv[1], "TTM", 3)) {
    bluetooth.writeString(argv[1]);
    char * line = bluetooth.readline();
    cliSerialPrint("<BT %s", line);
  }
  else if (toInt(argv, 1, &baudrate) > 0) {
    if (baudrate > 0) {
      bluetoothInit(baudrate, true);
      char * line = bluetooth.readline();
      cliSerialPrint("<BT %s", line);
    }
    else {
      bluetoothDisable();
      cliSerialPrint("BT turned off");
    }
  }
  else {
    cliSerialPrint("%s: Invalid arguments", argv[0]);
  }
  return 0;
}
#endif

#if defined(ACCESS_DENIED) && defined(DEBUG_CRYPT)

extern "C" {
#include "AccessDenied/access_denied.h"
}

static uint8_t cryptInput[16];
static uint8_t cryptOutput[16];

static void testAccessDenied(uint32_t runs)
{
  while(runs--)
    access_denied(0, cryptInput, cryptOutput);
}

int cliCrypt(const char ** argv)
{
  if (argv[1][0] == '\0')
    return -1;

  strncpy((char*)cryptInput, argv[1], sizeof(cryptInput));
  cliSerialPrint("Input:");
  dumpBody(cryptInput, sizeof(cryptInput));
  dumpEnd();

  watchdogSuspend(2000/* 20s */);
  if (pulsesStarted()) {
    pausePulses();
  }
  mixerTaskStop();
  perMainEnabled = false;

  uint32_t startMs = time_get_ms();
  testAccessDenied(100000);
  cliSerialPrintf("access_denied: %f us/run\r\n", (time_get_ms() - startMs)*1000.0f / 100000.0f);

  cliSerialPrint("Decrypted (SW):");
  dumpBody(cryptOutput, sizeof(cryptOutput));
  dumpEnd();

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  mixerTaskStart();
  watchdogSuspend(0);

  return 0;
}
#endif

#if defined(TP_GT911)
// from tp_gt911.cpp
extern uint8_t tp_gt911_cfgVer;

int cliResetGT911(const char** argv)
{
    (void)argv;

    if (!touchGT911Flag) {
        cliSerialPrint("GT911 not detected: exit\n");
        return 0;
    }

    // stop pulses & suspend RTOS scheduler
    watchdogSuspend(200/*2s*/);
    pulsesStop();
    vTaskSuspendAll();

    // reset touch controller
    touchPanelDeInit();
    cliSerialPrintf("GT911: old config version is %u\n", tp_gt911_cfgVer);
    tp_gt911_cfgVer = 0;
    touchPanelInit();
    cliSerialPrintf("GT911: new config version is %u\n", tp_gt911_cfgVer);

    // restart pulses & RTOS scheduler
    pulsesStart();
    xTaskResumeAll();

    return 0;
}
#endif

const CliCommand cliCommands[] = {
  { "beep", cliBeep, "[<frequency>] [<duration>]" },
  { "ls", cliLs, "<directory>" },
  { "read", cliRead, "<filename>" },
  { "readsd", cliReadSD, "<start sector> <sectors count> <read buffer size (sectors)>" },
  { "testsd", cliTestSD, "" },
  { "play", cliPlay, "<filename>" },
  { "reboot", cliReboot, "[wdt]" },
  { "set", cliSet, "<what> <value>" },
#if defined(ENABLE_SERIAL_PASSTHROUGH)
  { "serialpassthrough", cliSerialPassthrough, "<port type> [<port number>] [<baudrate>]"},
#endif
#if defined(DEBUG)
  { "print", cliDisplay, "<address> [<size>] | <what>" },
  { "p", cliDisplay, "<address> [<size>] | <what>" },
  { "stackinfo", cliStackInfo, "" },
  { "meminfo", cliMemoryInfo, "" },
  { "test", cliTest, "new | graphics | memspd" },
  { "trace", cliTrace, "on | off" },
  { "debugvars", cliDebugVars, "" },
  { "repeat", cliRepeat, "<interval> <command>" },
#endif
  { "help", cliHelp, "[<command>]" },
#if defined(JITTER_MEASURE)
  { "jitter", cliShowJitter, "" },
#endif
#if defined(INTERNAL_GPS)
  { "gps", cliGps, "<baudrate>|$<command>|trace" },
#endif
#if defined(SPACEMOUSE)
  { "spacemouse", cliSpaceMouse, "poll | tare | startstreaming | stopstreaming | trace" },
#endif
#if defined(BLUETOOTH)
  { "bt", cliBlueTooth, "<baudrate>|<command>" },
#endif
#if defined(ACCESS_DENIED) && defined(DEBUG_CRYPT)
  { "crypt", cliCrypt, "<string to be encrypted>" },
#endif
#if defined(TP_GT911)
  { "reset_gt911", cliResetGT911, ""},
#endif
  { nullptr, nullptr, nullptr }  /* sentinel */
};

static int cliHelp(const char ** argv)
{
  for (const CliCommand * command = cliCommands; command->name != nullptr; command++) {
    if (argv[1][0] == '\0' || !strcmp(command->name, argv[0])) {
      cliSerialPrint("%s %s", command->name, command->args);
      if (argv[1][0] != '\0') {
        return 0;
      }
    }
  }
  if (argv[1][0] != '\0') {
    cliSerialPrint("Invalid command \"%s\"", argv[0]);
  }
  return -1;
}

static int cliExecCommand(const char ** argv)
{
  if (argv[0][0] == '\0')
    return 0;

  for (const CliCommand * command = cliCommands; command->name != nullptr; command++) {
    if (!strcmp(command->name, argv[0])) {
      return command->func(argv);
    }
  }
  cliSerialPrint("Invalid command \"%s\"", argv[0]);
  return -1;
}

static int cliExecLine(char * line)
{
  int len = strlen(line);
  const char * argv[CLI_COMMAND_MAX_ARGS];
  memset(argv, 0, sizeof(argv));
  int argc = 1;
  argv[0] = line;
  for (int i=0; i<len; i++) {
    if (line[i] == ' ') {
      line[i] = '\0';
      if (argc < CLI_COMMAND_MAX_ARGS) {
        argv[argc++] = &line[i+1];
      }
    }
  }
  return cliExecCommand(argv);
}

#define CHAR_LF         0x0A
#define CHAR_NEWPAGE    0x0C
#define CHAR_CR         0x0D
#define CHAR_DEL        0x7F

static void cliTask()
{
  char line[CLI_COMMAND_MAX_LEN+1];
  int pos = 0;

  cliPrompt();

  for (;;) {
    uint8_t c;

    // TODO: implement block read instead
    //       of going byte-by-byte.
    
    /* Block for max 100ms. */
    const TickType_t xTimeout = 100 / portTICK_PERIOD_MS;
    size_t xReceivedBytes = xStreamBufferReceive(cliRxBuffer, &c, 1, xTimeout);

    if (!mixerTaskRunning()) {
      WDG_RESET();
    }

    if (!xReceivedBytes) {
      continue;
    }

    switch(c) {
    case CHAR_NEWPAGE:
      // clear screen
      cliSerialPrint("\033[2J\033[1;1H");
      cliPrompt();
      break;

    case CHAR_DEL:
      if (pos) {
        line[--pos] = '\0';
        cliSerialPutc('\010');
        cliSerialPutc(' ');
        cliSerialPutc('\010');
      }
      break;

    case CHAR_CR:
      // ignore
      break;

    case CHAR_LF:
      // enter
      cliSerialCrlf();
      line[pos] = '\0';
      if (pos == 0 && cliLastLine[0]) {
        // execute (repeat) last command
        strcpy(line, cliLastLine);
      } else {
        // save new command
        strcpy(cliLastLine, line);
      }
      // TODO: check return value
      cliExecLine(line);
      pos = 0;
      cliPrompt();
      break;

    default:
      if (isascii(c) && pos < CLI_COMMAND_MAX_LEN) {
        line[pos++] = c;
        cliSerialPutc(c);
      }
      break;
    }
  }
}

void cliStart()
{
  // Init stream buffer
  const size_t xTriggerLevel = 1;
  cliRxBuffer =
      xStreamBufferCreateStatic(sizeof(cliRxBufferStorage), xTriggerLevel,
                                cliRxBufferStorage, &cliRxBufferStatic);

  // Setup consumer callback
  cliReceiveCallBack = cliDefaultRx;

  task_create(&cliTaskId, cliTask, "CLI", cliStack, CLI_STACK_SIZE,
              CLI_TASK_PRIO);
}
