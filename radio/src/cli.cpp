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

#include "opentx.h"
#include "diskio.h"
#include <ctype.h>
#include <malloc.h>
#include <new>

#include "cli.h"

#if defined(INTMODULE_USART)
#include "intmodule_serial_driver.h"
#endif

#define CLI_COMMAND_MAX_ARGS           8
#define CLI_COMMAND_MAX_LEN            256

// CLI receive buffer size
#define CLI_RX_BUFFER_SIZE 256

RTOS_TASK_HANDLE cliTaskId;
RTOS_DEFINE_STACK(cliStack, CLI_STACK_SIZE);

static uint8_t cliRxBufferStorage[CLI_RX_BUFFER_SIZE];
static StaticStreamBuffer_t cliRxBufferStatic;

// CLI receive stream buffer
static StreamBufferHandle_t cliRxBuffer;

// CLI receive call back
void (*cliReceiveCallBack)(uint8_t* buf, uint32_t len) = nullptr;

uint8_t cliTracesEnabled = true;
char cliLastLine[CLI_COMMAND_MAX_LEN+1];

typedef int (* CliFunction) (const char ** args);
int cliExecLine(char * line);
int cliExecCommand(const char ** argv);
int cliHelp(const char ** argv);

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
  serialPutc('>');
  serialPutc(' ');
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
      serialPrint("%s: Invalid argument \"%s\"", argv[0], argv[index]);
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
      serialPrint(fno.fname);
    }
    f_closedir(&dir);
  }
  else {
    serialPrint("%s: Invalid directory \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliRead(const char ** argv)
{
  FIL file;
  uint32_t bytesRead = 0;
  int bufferSize;
  if (toInt(argv, 2, &bufferSize) == 0 || bufferSize < 0 ) {
    serialPrint("%s: Invalid buffer size \"%s\"", argv[0], argv[2]);
    return 0;
  }

  uint8_t * buffer = (uint8_t*) malloc(bufferSize);
  if (!buffer) {
    serialPrint("Not enough memory");
    return 0;
  }

  FRESULT result = f_open(&file, argv[1], FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    free(buffer);
    serialPrint("%s: File not found \"%s\"", argv[0], argv[1]);
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
  serialPrint("Read %d bytes in %d ms, speed %d kB/s", bytesRead, elapsedTime, speed);
  free(buffer);
  return 0;
}

int cliReadSD(const char ** argv)
{
  int startSector;
  int numberOfSectors;
  int bufferSectors;
  if (toInt(argv, 1, &startSector) == 0 || startSector < 0 ) {
    serialPrint("%s: Invalid start sector \"%s\"", argv[0], argv[1]);
    return 0;
  }
  if (toInt(argv, 2, &numberOfSectors) == 0 || numberOfSectors < 0 ) {
    serialPrint("%s: Invalid number of sectors \"%s\"", argv[0], argv[2]);
    return 0;
  }

  if (toInt(argv, 3, &bufferSectors) == 0 || bufferSectors < 0 ) {
    serialPrint("%s: Invalid number of buffer sectors \"%s\"", argv[0], argv[3]);
    return 0;
  }

  uint8_t * buffer = (uint8_t*) malloc(512*bufferSectors);
  if (!buffer) {
    serialPrint("Not enough memory");
    return 0;
  }

  uint32_t bytesRead = numberOfSectors * 512;
  tmr10ms_t start = get_tmr10ms();

  while (numberOfSectors > 0) {
    DRESULT res = __disk_read(0, buffer, startSector, bufferSectors);
    if (res != RES_OK) {
      serialPrint("disk_read error: %d, sector: %d(%d)", res, startSector, numberOfSectors);
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
    serialPrint("sector %d(%d) checksumm: %u", startSector, numberOfSectors, summ);
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
  serialPrint("Read %d bytes in %d ms, speed %d kB/s", bytesRead, elapsedTime, speed);
  free(buffer);
  return 0;
}

int cliTestSD(const char ** argv)
{
  // Do the read test on the SD card and report back the result

  // get sector count
  uint32_t sectorCount;
  if (disk_ioctl(0, GET_SECTOR_COUNT, &sectorCount) != RES_OK) {
    serialPrint("Error: can't read sector count");
    return 0;
  }
  serialPrint("SD card has %u sectors", sectorCount);

  // read last 16 sectors one sector at the time
  serialPrint("Starting single sector read test, reading 16 sectors one by one");
  uint8_t * buffer = (uint8_t*) malloc(512);
  if (!buffer) {
    serialPrint("Not enough memory");
    return 0;
  }
  for (uint32_t s = sectorCount - 16; s<sectorCount; ++s) {
    DRESULT res = __disk_read(0, buffer, s, 1);
    if (res != RES_OK) {
      serialPrint("sector %d read FAILED, err: %d", s, res);
    }
    else {
      serialPrint("sector %d read OK", s);
    }
  }
  free(buffer);
  serialCrlf();

  // read last 16 sectors, two sectors at the time with a multi-block read
  buffer = (uint8_t *) malloc(512*2);
  if (!buffer) {
    serialPrint("Not enough memory");
    return 0;
  }

  serialPrint("Starting multiple sector read test, reading two sectors at the time");
  for (uint32_t s = sectorCount - 16; s<sectorCount; s+=2) {
    DRESULT res = __disk_read(0, buffer, s, 2);
    if (res != RES_OK) {
      serialPrint("sector %d-%d read FAILED, err: %d", s, s+1, res);
    }
    else {
      serialPrint("sector %d-%d read OK", s, s+1);
    }
  }
  free(buffer);
  serialCrlf();

  // read last 16 sectors, all sectors with single multi-block read
  buffer = (uint8_t*) malloc(512*16);
  if (!buffer) {
    serialPrint("Not enough memory");
    return 0;
  }

  serialPrint("Starting multiple sector read test, reading 16 sectors at the time");
  DRESULT res = __disk_read(0, buffer, sectorCount-16, 16);
  if (res != RES_OK) {
    serialPrint("sector %d-%d read FAILED, err: %d", sectorCount-16, sectorCount-1, res);
  }
  else {
    serialPrint("sector %d-%d read OK", sectorCount-16, sectorCount-1);
  }
  free(buffer);
  serialCrlf();

  return 0;
}

int cliTestNew()
{
  char * tmp = 0;
  serialPrint("Allocating 1kB with new()");
  RTOS_WAIT_MS(200);
  tmp = new char[1024];
  if (tmp) {
    serialPrint("\tsuccess");
    delete[] tmp;
    tmp = 0;
  }
  else {
    serialPrint("\tFAILURE");
  }

  serialPrint("Allocating 10MB with (std::nothrow) new()");
  RTOS_WAIT_MS(200);
  tmp = new (std::nothrow) char[1024*1024*10];
  if (tmp) {
    serialPrint("\tFAILURE, tmp = %p", tmp);
    delete[] tmp;
    tmp = 0;
  }
  else {
    serialPrint("\tsuccess, allocaton failed, tmp = 0");
  }

  serialPrint("Allocating 10MB with new()");
  RTOS_WAIT_MS(200);
  tmp = new char[1024*1024*10];
  if (tmp) {
    serialPrint("\tFAILURE, tmp = %p", tmp);
    delete[] tmp;
    tmp = 0;
  }
  else {
    serialPrint("\tsuccess, allocaton failed, tmp = 0");
  }
  serialPrint("Test finished");
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

void testDrawBlackOverlay() { lcdDrawBlackOverlay(); }

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
  const uint32_t start = RTOS_GET_MS();
  uint32_t noRuns = 0;
  uint32_t actualRuntime = 0;
  while ((actualRuntime = RTOS_GET_MS() - start) < runtime) {
    for (uint16_t n = 0; n < step; n++) {
      func();
    }
    lcdRefresh();
    noRuns += step;
  }
  const float result = (noRuns * 500.0f) / (float)actualRuntime;  // runs/second
  serialPrint("Test %s speed: %lu.%02u, (%lu runs in %lu ms)", name,
              uint32_t(result), uint16_t((result - uint32_t(result)) * 100.0f),
              noRuns, actualRuntime);
  RTOS_WAIT_MS(200);
  return result;
}

int cliTestGraphics()
{
  serialPrint("Starting graphics performance test...");
  RTOS_WAIT_MS(200);

  watchdogSuspend(6000 /*60s*/);
  if (pulsesStarted()) {
    pausePulses();
  }
  pauseMixerCalculations();
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

  serialPrint("Total speed: %lu.%02u", uint32_t(result),
              uint16_t((result - uint32_t(result)) * 100.0f));

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  resumeMixerCalculations();
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
  serialPrint("Starting memory speed test...");
  RTOS_WAIT_MS(200);

  watchdogSuspend(6000 /*60s*/);
  if (pulsesStarted()) {
    pausePulses();
  }
  pauseMixerCalculations();
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
  serialPrint("Disabling LCD...");
  RTOS_WAIT_MS(200);

  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_RAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryReadFrom_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_RAM_to_SDRAM_32bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_8bit, 200);
  result += RUN_MEMORY_TEST(testMemoryCopyFrom_SDRAM_to_SDRAM_32bit, 200);

  serialPrint("Total speed: %lu.%02u", uint32_t(result),
              uint16_t((result - uint32_t(result)) * 100.0f));

  LTDC_Cmd(ENABLE);

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  resumeMixerCalculations();
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

  serialPrint("Starting fetching RF data 100x...");
  const uint32_t start = RTOS_GET_MS();

  const list<ModelsCategory *> &cats = modList.getCategories();
  while (1) {
    for (list<ModelsCategory *>::const_iterator cat_it = cats.begin();
         cat_it != cats.end(); ++cat_it) {
      for (ModelsCategory::iterator mod_it = (*cat_it)->begin();
           mod_it != (*cat_it)->end(); mod_it++) {
        if (!(*mod_it)->fetchRfData()) {
          serialPrint("Error while fetching RF data...");
          return 0;
        }

        if (++count >= 100) goto done;
      }
    }
  }

done:
  serialPrint("Done fetching %ix RF data: %lu ms", count,
              (RTOS_GET_MS() - start));

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
    serialPrint("%s: Invalid argument \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliTrace(const char ** argv)
{
  if (!strcmp(argv[1], "on")) {
    cliTracesEnabled = true;
  }
  else if (!strcmp(argv[1], "off")) {
    cliTracesEnabled = false;
  }
  else {
    serialPrint("%s: Invalid argument \"%s\"", argv[0], argv[1]);
  }
  return 0;
}

int cliStackInfo(const char ** argv)
{
  serialPrint("[MAIN] %d available / %d bytes", stackAvailable()*4, stackSize()*4);
  serialPrint("[MENUS] %d available / %d bytes", menusStack.available()*4, menusStack.size());
  serialPrint("[MIXER] %d available / %d bytes", mixerStack.available()*4, mixerStack.size());
  serialPrint("[AUDIO] %d available / %d bytes", audioStack.available()*4, audioStack.size());
  serialPrint("[CLI] %d available / %d bytes", cliStack.available()*4, cliStack.size());
  return 0;
}

extern int _end;
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
  serialPrint("mallinfo:");
  serialPrint("\tarena    %d bytes", info.arena);
  serialPrint("\tordblks  %d bytes", info.ordblks);
  serialPrint("\tuordblks %d bytes", info.uordblks);
  serialPrint("\tfordblks %d bytes", info.fordblks);
  serialPrint("\tkeepcost %d bytes", info.keepcost);

  serialPrint("\nHeap:");
  serialPrint("\tstart %p", (unsigned char *)&_end);
  serialPrint("\tend   %p", (unsigned char *)&_heap_end);
  serialPrint("\tcurr  %p", heap);
  serialPrint("\tused  %d bytes", (int)(heap - (unsigned char *)&_end));
  serialPrint("\tfree  %d bytes", (int)((unsigned char *)&_heap_end - heap));

#if defined(LUA)
  serialPrint("\nLua:");
  uint32_t s = luaGetMemUsed(lsScripts);
  serialPrint("\tScripts %u", s);
#if defined(COLORLCD)
  uint32_t w = luaGetMemUsed(lsWidgets);
  uint32_t e = luaExtraMemoryUsage;
  serialPrint("\tWidgets %u", w);
  serialPrint("\tExtra   %u", e);
  serialPrint("------------");
  serialPrint("\tTotal   %u", s + w + e);
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
    pausePulses();
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
      serialPrint("%s: Invalid arguments \"%s\" \"%s\"", argv[0], argv[1],
                  argv[2]);
      return -1;
    }
  }
#if !defined(SOFTWARE_VOLUME)
  else if (!strcmp(argv[1], "volume")) {
    int level = 0;
    if (toInt(argv, 2, &level) > 0) {
      setVolume(level);
    } else {
      serialPrint("%s: Invalid argument \"%s\" \"%s\"", argv[0], argv[1],
                  argv[2]);
      return -1;
    }
  }
#endif
  else if (!strcmp(argv[1], "rfmod")) {
    int module = 0;
    if (toInt(argv, 2, &module) < 0) {
      serialPrint("%s: invalid module argument '%s'", argv[0], argv[2]);
      return -1;
    }
    if (!strcmp(argv[3], "power")) {
      if (!strcmp("on", argv[4])) {
        if (module == 0) {
          INTERNAL_MODULE_ON();
        } else {
          EXTERNAL_MODULE_ON();
        }
      } else if (!strcmp("off", argv[4])) {
        if (module == 0) {
          INTERNAL_MODULE_OFF();
        } else {
          EXTERNAL_MODULE_OFF();
        }
      } else {
        serialPrint("%s: invalid power argument '%s'", argv[0], argv[4]);
        return -1;
      }
      serialPrint("%s: rfmod %d power %d", argv[0], module, argv[4]);
    }
#if defined(INTMODULE_BOOTCMD_GPIO)
    else if (!strcmp(argv[3], "bootpin")) {
      int level = 0;
      if (toInt(argv, 4, &level) < 0) {
        serialPrint("%s: invalid bootpin argument '%s'", argv[0], argv[4]);
        return -1;
      }
      if (module == 0) {
        if (level) {
#if defined(INTMODULE_BOOTCMD_INVERTED)
          GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#else
          GPIO_SetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#endif
          serialPrint("%s: bootpin set", argv[0]);
        } else {
#if defined(INTMODULE_BOOTCMD_INVERTED)
          GPIO_SetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#else
          GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#endif
          serialPrint("%s: bootpin reset", argv[0]);
        }
      }
    }
#endif
  }
  else if (!strcmp(argv[1], "pulses")) {
    int level = 0;
    if (toInt(argv, 2, &level) < 0) {
      serialPrint("%s: invalid level argument '%s'", argv[0], argv[2]);
      return -1;
    }

    if (level) {
      serialPrint("%s: pulses start", argv[0]);
      startPulses();
    } else {
      serialPrint("%s: pulses stop", argv[0]);
      stopPulses();
    }
  }

  return 0;
}

#if defined(ENABLE_SERIAL_PASSTHROUGH)
static void spInternalModuleTx(uint8_t* buf, uint32_t len)
{
  intmoduleSendBuffer(buf, len);
  intmoduleWaitForTxCompleted();
}

static void spInternalModuleSetBaudRate(uint32_t baud)
{
  etx_serial_init params;
  params.baudrate = baud;
  params.rx_enable = true;

  // re-configure serial port
  intmoduleSerialStart(&params);
}

// TODO: use proper method instead
extern bool cdcConnected;

int cliSerialPassthrough(const char **argv)
{
  const char* port_type = argv[1];
  const char* port_num  = argv[2];

  // 3rd argument (baudrate is optional)
  if (!port_type || !port_num) {
    serialPrint("%s: missing argument", argv[0]);
    return -1;
  }

  int port_n = 0;
  int err = toInt(argv, 2, &port_n);
  if (err == -1) return err;
  if (err == 0) {
    serialPrint("%s: missing port #", argv[0]);
    return -1;
  }

  int baudrate = 0;
  err = toInt(argv, 3, &baudrate);
  if (err <= 0) {
    // use current USB CDC baudrate
    baudrate = usbSerialBaudRate();
    if (!baudrate) {
      // default to 115200
      baudrate = 115200;
      serialPrint("%s: USB baudrate is 0, default to 115200", argv[0]);
    }
  }

  if (!strcmp("rfmod", port_type)) {

    if (port_n >= NUM_MODULES
        // only internal module supported for now
        && port_n != INTERNAL_MODULE) {
      serialPrint("%s: invalid port # '%s'", port_num);
      return -1;
    }
    
    //  stop pulses
    watchdogSuspend(200/*2s*/);
    stopPulses();

    // suspend RTOS scheduler
    vTaskSuspendAll();

#if defined(HARDWARE_INTERNAL_MODULE)
    if (port_n == INTERNAL_MODULE) {

      // setup serial com

      // TODO: '8n1' param
      etx_serial_init params;
      params.baudrate = baudrate;
      params.rx_enable = true;
      intmoduleSerialStart(&params);

      // backup and swap CLI input
      auto backupCB = cliReceiveCallBack;
      cliReceiveCallBack = spInternalModuleTx;

      // setup CDC baudrate callback
      usbSerialSetBaudRateCb(spInternalModuleSetBaudRate);

      // loop until cable disconnected
      while (cdcConnected) {

        uint8_t data;
        if (intmoduleFifo.pop(data)) {

          uint8_t timeout = 10; // 10 ms
          while(!usbSerialFreeSpace() && (timeout > 0)) {
            delay_ms(1);
            timeout--;
          }

          usbSerialPutc(data);
        }

        // keep us up & running
        WDG_RESET();
      }

      // restore callsbacks
      usbSerialSetBaudRateCb(nullptr);
      cliReceiveCallBack = backupCB;

      // and stop module
      intmoduleStop();

      // power off the module and wait for a bit
      INTERNAL_MODULE_OFF();
      delay_ms(200);
    }
#endif

    // TODO:
    //  - external module (S.PORT?)

    watchdogSuspend(200/*2s*/);
    startPulses();

    // suspend RTOS scheduler
    xTaskResumeAll();
    
  } else {
    serialPrint("%s: invalid port type '%s'", port_type);
    return -1;
  }
  
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
  serialPrint("Interrupts count in the last %u ms:", (get_tmr10ms() - ic.resetTime) * 10);
  for(int n = 0; n < INT_LAST; n++) {
    serialPrint("%s: %u", interruptNames[n], ic.cnt[n]);
  }
}
#endif //#if defined(DEBUG_INTERRUPTS)

#if defined(DEBUG_TASKS)
void printTaskSwitchLog()
{
  serialPrint("Tasks legend [<task_id>, <task name>]:");
  for(int n = 0; n <= CFG_MAX_USER_TASKS+1; n++) {
    if (0 == n) {
      serialPrint("%d: Idle", n);
    }
    if (cliTaskId == n) {
      serialPrint("%d: CLI", n);
    }
    else if (menusTaskId == n) {
      serialPrint("%d: menus", n);
    }
    else if (mixerTaskId == n) {
      serialPrint("%d: mixer", n);
    }
    else if (audioTaskId == n) {
      serialPrint("%d: audio", n);
    }
  }
  serialCrlf();

  serialPrint("Tasks switch log at %u [<time>, <task_id>]:", get_tmr10ms());
  uint32_t lastSwitchTime = 0;
  uint32_t * tsl = new uint32_t[DEBUG_TASKS_LOG_SIZE];
  if (!tsl) {
    serialPrint("Not enough memory");
    return;
  }
  memcpy(tsl, taskSwitchLog, sizeof(taskSwitchLog));
  uint32_t * p = tsl + taskSwitchLogPos;
  uint32_t * end = tsl + DEBUG_TASKS_LOG_SIZE;
  for(int n = 0; n < DEBUG_TASKS_LOG_SIZE; n++) {
    uint32_t taskId = *p >> 24;
    uint32_t switchTime = *p & 0xFFFFFF;
    if (lastSwitchTime != switchTime) {
      serialPrintf("\r\n%06x: ", switchTime);
      lastSwitchTime = switchTime;
    }
    serialPrintf("%u ", taskId);
    if ( ++p >= end ) {
      p = tsl;
    }
  }
  delete[] tsl;
  serialCrlf();
}
#endif // #if defined(DEBUG_TASKS)

#if defined(DEBUG_TIMERS)
void printDebugTime(uint32_t time)
{
  if (time >= 30000) {
    serialPrintf("%dms", time/1000);
  }
  else {
    serialPrintf("%d.%03dms", time/1000, time%1000);
  }
}

void printDebugTimer(const char * name, DebugTimer & timer)
{
  serialPrintf("%s: ", name);
  printDebugTime( timer.getMin());
  serialPrintf(" - ");
  printDebugTime(timer.getMax());
  serialCrlf();
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
    serialPrint("Audio Buffer %d: size: %u, ", n,
                (uint32_t)audioBuffers[n].size);
    dump((uint8_t *)audioBuffers[n].data, 32);
  }
  serialPrint("fragments:");
  for (int n = 0; n < AUDIO_QUEUE_LENGTH; n++) {
    serialPrint("%d: type %u: id: %u, repeat: %u, ", n,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].type,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].id,
                (uint32_t)audioQueue.fragmentsFifo.fragments[n].repeat);
    if (audioQueue.fragmentsFifo.fragments[n].type == FRAGMENT_FILE) {
      serialPrint(" file: %s", audioQueue.fragmentsFifo.fragments[n].file);
    }
  }

  serialPrint("FragmentFifo:  ridx: %d, widx: %d",
              audioQueue.fragmentsFifo.ridx, audioQueue.fragmentsFifo.widx);
  serialPrint("audioQueue:  readIdx: %d, writeIdx: %d, full: %d",
              audioQueue.buffersFifo.readIdx, audioQueue.buffersFifo.writeIdx,
              audioQueue.buffersFifo.bufferFull);

  serialPrint("normalContext: %u",
              (uint32_t)audioQueue.normalContext.fragment.type);
}
#endif

#if defined(DEBUG)
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
    for (int i=0; i<TRM_BASE; i++) {
      char name[8];
      uint8_t len = STR_VKEYS[0];
      strncpy(name, STR_VKEYS+1+len*i, len);
      name[len] = '\0';
      serialPrint("[%s] = %s", name, keys[i].state() ? "on" : "off");
    }
#if defined(ROTARY_ENCODER_NAVIGATION)
    serialPrint("[Enc.] = %d", rotencValue / ROTARY_ENCODER_GRANULARITY);
#endif
    for (int i=TRM_BASE; i<=TRM_LAST; i++) {
      serialPrint("[Trim%d] = %s", i-TRM_BASE, keys[i].state() ? "on" : "off");
    }
    for (int i=MIXSRC_FIRST_SWITCH; i<=MIXSRC_LAST_SWITCH; i++) {
      mixsrc_t sw = i - MIXSRC_FIRST_SWITCH;
      if (SWITCH_EXISTS(sw)) {
        char swName[LEN_SWITCH_NAME + 1];
        strAppend(swName, STR_VSWITCHES+1+sw*STR_VSWITCHES[0], STR_VSWITCHES[0]);
        static const char * const SWITCH_POSITIONS[] = { "down", "mid", "up" };
        serialPrint("[%s] = %s", swName, SWITCH_POSITIONS[1 + getValue(i) / 1024]);
      }
    }
  }
  else if (!strcmp(argv[1], "adc")) {
    for (int i=0; i<NUM_ANALOGS; i++) {
      serialPrint("adc[%d] = %04X", i, (int)adcValues[i]);
    }
  }
  else if (!strcmp(argv[1], "outputs")) {
    for (int i=0; i<MAX_OUTPUT_CHANNELS; i++) {
      serialPrint("outputs[%d] = %04d", i, (int)channelOutputs[i]);
    }
  }
  else if (!strcmp(argv[1], "rtc")) {
    struct gtm utm;
    gettime(&utm);
    serialPrint("rtc = %4d-%02d-%02d %02d:%02d:%02d.%02d0", utm.tm_year+TM_YEAR_BASE, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
  }
#if !defined(SOFTWARE_VOLUME)
  else if (!strcmp(argv[1], "volume")) {
    serialPrint("volume = %d", getVolume());
  }
#endif
#if defined(STM32)
  else if (!strcmp(argv[1], "uid")) {
    char str[LEN_CPU_UID+1];
    getCPUUniqueID(str);
    serialPrint("uid = %s", str);
  }
#endif
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
        case 8:
          tim = TIM8;
          break;
        case 13:
          tim = TIM13;
          break;
        default:
          return 0;
      }
      serialPrint("TIM%d", timerNumber);
      serialPrint(" CR1    0x%x", tim->CR1);
      serialPrint(" CR2    0x%x", tim->CR2);
      serialPrint(" DIER   0x%x", tim->DIER);
      serialPrint(" SR     0x%x", tim->SR);
      serialPrint(" EGR    0x%x", tim->EGR);
      serialPrint(" CCMR1  0x%x", tim->CCMR1);
      serialPrint(" CCMR2  0x%x", tim->CCMR2);

      serialPrint(" CNT    0x%x", tim->CNT);
      serialPrint(" ARR    0x%x", tim->ARR);
      serialPrint(" PSC    0x%x", tim->PSC);

      serialPrint(" CCER   0x%x", tim->CCER);
      serialPrint(" CCR1   0x%x", tim->CCR1);
      serialPrint(" CCR2   0x%x", tim->CCR2);
      serialPrint(" CCR3   0x%x", tim->CCR3);
      serialPrint(" CCR4   0x%x", tim->CCR4);
    }
  }
  else if (!strcmp(argv[1], "dma")) {
    serialPrint("DMA1_Stream7");
    serialPrint(" CR    0x%x", DMA1_Stream7->CR);
  }
#if defined(DEBUG_INTERRUPTS)
  else if (!strcmp(argv[1], "int")) {
    printInterrupts();
  }
#endif
#if defined(DEBUG_TASKS)
  else if (!strcmp(argv[1], "tsl")) {
    printTaskSwitchLog();
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
    serialPrint("Disk Cache stats: w:%u r: %u, h: %u(%0.1f%%), m: %u", stats.noWrites, (stats.noHits + stats.noMisses), stats.noHits, hitRate*0.1f, stats.noMisses);
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
#if defined(PCBHORUS)
  extern uint32_t ioMutexReq, ioMutexRel;
  extern uint32_t sdReadRetries;
  serialPrint("ioMutexReq=%d", ioMutexReq);
  serialPrint("ioMutexRel=%d", ioMutexRel);
  serialPrint("sdReadRetries=%d", sdReadRetries);
#if defined(ACCESS_DENIED)
  extern volatile int32_t authenticateFrames;
  serialPrint("authenticateFrames=%d", authenticateFrames);
#endif
#elif defined(PCBTARANIS)
  serialPrint("telemetryErrors=%d", telemetryErrors);
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
    const TickType_t xTimeout = 20 / RTOS_MS_PER_TICK;
    while (!xStreamBufferReceive(cliRxBuffer, &c, 1, xTimeout)
           || !(c == '\r' || c == '\n' || c == ' ')) {

      if (++counter >= interval) {
        cliExecCommand(&argv[2]);
        counter = 0;
      }
    }
  }
  else {
    serialPrint("%s: Invalid arguments", argv[0]);
  }
  return 0;
}
#endif

#if defined(JITTER_MEASURE)
int cliShowJitter(const char ** argv)
{
  serialPrint(  "#   anaIn   rawJ   avgJ");
  for (int i=0; i<NUM_ANALOGS; i++) {
    serialPrint("A%02d %04X %04X %3d %3d", i, getAnalogValue(i), anaIn(i), rawJitter[i].get(), avgJitter[i].get());
    if (IS_POT_MULTIPOS(i)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
      for (int j=0; j<calib->count; j++) {
        serialPrint("    s%d %04X", j, calib->steps[j]);
      }
    }
  }
  return 0;
}
#endif

#if defined(INTERNAL_GPS)
int cliGps(const char ** argv)
{
  int baudrate = 0;

  if (argv[1][0] == '$') {
    // send command to GPS
    gpsSendFrame(argv[1]);
  }
#if defined(DEBUG)
  else if (!strcmp(argv[1], "trace")) {
    gpsTraceEnabled = !gpsTraceEnabled;
  }
#endif
  else if (toInt(argv, 1, &baudrate) > 0 && baudrate > 0) {
    gpsInit(baudrate);
    serialPrint("GPS baudrate set to %d", baudrate);
  }
  else {
    serialPrint("%s: Invalid arguments", argv[0]);
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
    serialPrint("<BT %s", line);
  }
  else if (toInt(argv, 1, &baudrate) > 0) {
    if (baudrate > 0) {
      bluetoothInit(baudrate, true);
      char * line = bluetooth.readline();
      serialPrint("<BT %s", line);
    }
    else {
      bluetoothDisable();
      serialPrint("BT turned off");
    }
  }
  else {
    serialPrint("%s: Invalid arguments", argv[0]);
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
  serialPrint("Input:");
  dumpBody(cryptInput, sizeof(cryptInput));
  dumpEnd();

  watchdogSuspend(2000/* 20s */);
  if (pulsesStarted()) {
    pausePulses();
  }
  pauseMixerCalculations();
  perMainEnabled = false;

  uint32_t startMs = RTOS_GET_MS();
  testAccessDenied(100000);
  serialPrintf("access_denied: %f us/run\r\n", (RTOS_GET_MS() - startMs)*1000.0f / 100000.0f);

  serialPrint("Decrypted (SW):");
  dumpBody(cryptOutput, sizeof(cryptOutput));
  dumpEnd();

  perMainEnabled = true;
  if (pulsesStarted()) {
    resumePulses();
  }
  resumeMixerCalculations();
  watchdogSuspend(0);

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
  { "serialpassthrough", cliSerialPassthrough, "<port type> <port number>"},
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
#if defined(BLUETOOTH)
  { "bt", cliBlueTooth, "<baudrate>|<command>" },
#endif
#if defined(ACCESS_DENIED) && defined(DEBUG_CRYPT)
  { "crypt", cliCrypt, "<string to be encrypted>" },
#endif
  { nullptr, nullptr, nullptr }  /* sentinel */
};

int cliHelp(const char ** argv)
{
  for (const CliCommand * command = cliCommands; command->name != nullptr; command++) {
    if (argv[1][0] == '\0' || !strcmp(command->name, argv[0])) {
      serialPrint("%s %s", command->name, command->args);
      if (argv[1][0] != '\0') {
        return 0;
      }
    }
  }
  if (argv[1][0] != '\0') {
    serialPrint("Invalid command \"%s\"", argv[0]);
  }
  return -1;
}

int cliExecCommand(const char ** argv)
{
  if (argv[0][0] == '\0')
    return 0;

  for (const CliCommand * command = cliCommands; command->name != nullptr; command++) {
    if (!strcmp(command->name, argv[0])) {
      return command->func(argv);
    }
  }
  serialPrint("Invalid command \"%s\"", argv[0]);
  return -1;
}

int cliExecLine(char * line)
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
#define CHAR_BACKSPACE  0xFF

void cliTask(void * pdata)
{
  char line[CLI_COMMAND_MAX_LEN+1];
  int pos = 0;

  cliPrompt();

  for (;;) {
    uint8_t c;

    // TODO: implement block read instead
    //       of going byte-by-byte.
    
    /* Block for max 100ms. */
    const TickType_t xTimeout = 100 / RTOS_MS_PER_TICK;
    size_t xReceivedBytes = xStreamBufferReceive(cliRxBuffer, &c, 1, xTimeout);

    if (s_pulses_paused) {
      WDG_RESET();
    }

    if (!xReceivedBytes) {
      continue;
    }

    switch(c) {
    case CHAR_NEWPAGE:
      // clear screen
      serialPrint("\033[2J\033[1;1H");
      cliPrompt();
      break;

    case CHAR_BACKSPACE:
      if (pos) {
        line[--pos] = '\0';
        serialPutc(c);
      }
      break;

    case CHAR_CR:
      // ignore
      break;

    case CHAR_LF:
      // enter
      serialCrlf();
      line[pos] = '\0';
      if (pos == 0 && cliLastLine[0]) {
        // execute (repeat) last command
        strcpy(line, cliLastLine);
      }
      else {
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
        serialPutc(c);
      }
      break;
    }
  }
}

// Called from receive ISR (either USB or UART)
void cliReceiveData(uint8_t* buf, uint32_t len)
{
  if (cliReceiveCallBack)
    cliReceiveCallBack(buf, len);
}

static void cliDefaultRx(uint8_t *buf, uint32_t len)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xStreamBufferSendFromISR(cliRxBuffer, buf, len, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

  // Setup USB callbacks
  usbSerialSetReceiveDataCb(cliReceiveData);
  // usbSerialSetBaudRateCb(...);
  // usbSerialSetCtrlLineStateCb(...);

  RTOS_CREATE_TASK(cliTaskId, cliTask, "CLI", cliStack, CLI_STACK_SIZE,
                   CLI_TASK_PRIO);
}
