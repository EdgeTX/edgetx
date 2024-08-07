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

#include "edgetx.h"

const char * eepromFile = nullptr;
FILE * fp = nullptr;
uint32_t eeprom_pointer;
uint8_t * eeprom_buffer_data;
volatile int32_t eeprom_buffer_size;
bool eeprom_read_operation;
bool eeprom_thread_running = false;
uint8_t * eeprom = nullptr;
sem_t * eeprom_write_sem;

void eepromReadBlock (uint8_t * buffer, size_t address, size_t size)
{
  assert(size);

  if (fp) {
    // TRACE("EEPROM read (pos=%d, size=%d)", pointer_eeprom, size);
    if (fseek(fp, address, SEEK_SET) < 0)
      perror("error in fseek");
    if (fread(buffer, size, 1, fp) <= 0)
      perror("error in fread");
  }
  else if (eeprom) {
    memcpy(buffer, &eeprom[address], size);
  }
}

void eepromSimuWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);

  if (fp) {
    // TRACE("EEPROM write (pos=%d, size=%d)", pointer_eeprom, size);
    if (fseek(fp, address, SEEK_SET) < 0)
      perror("error in fseek");
    if (fwrite(buffer, size, 1, fp) <= 0)
      perror("error in fwrite");
  }
  else if (eeprom) {
    memcpy(&eeprom[address], buffer, size);
  }
}

volatile uint8_t eepromTransferComplete = 1;
void * eeprom_thread_function(void *)
{
  eeprom_thread_running = true;

  while (!sem_wait(eeprom_write_sem)) {
    if (!eeprom_thread_running)
      return nullptr;
    assert(eeprom_buffer_size);
    if (eeprom_read_operation) {
      eepromReadBlock(eeprom_buffer_data, eeprom_pointer, eeprom_buffer_size);
    }
    else {
      eepromSimuWriteBlock(eeprom_buffer_data, eeprom_pointer, eeprom_buffer_size);
    }
    eepromTransferComplete = 1;
  }

  return nullptr;
}

uint8_t eepromReadStatus()
{
  return 1;
}

uint8_t eepromIsTransferComplete()
{
  return eepromTransferComplete;
}

void eepromTransmitData(uint32_t address, uint8_t * buffer, uint32_t size, bool read)
{
  eeprom_pointer = address;
  eeprom_buffer_data = buffer;
  eeprom_buffer_size = size;
  eeprom_read_operation = read;
  eepromTransferComplete = 0;
  sem_post(eeprom_write_sem);
}

#if defined(EEPROM_BLOCK_SIZE)
void eepromBlockErase(uint32_t address)
{
  static uint8_t erasedBlock[EEPROM_BLOCK_SIZE]; // can't be on the stack!
  memset(erasedBlock, 0xff, sizeof(erasedBlock));
  eepromTransmitData(address, erasedBlock, EEPROM_BLOCK_SIZE, false);
}
#endif

void eepromStartRead(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);
  eepromTransmitData(address, buffer, size, true);
}

void eepromStartWrite(uint8_t * buffer, size_t address, size_t size)
{
  assert(size);
  eepromTransmitData(address, buffer, size, false);
}

bool _eeprom_write_simu_delay = true;

void eepromWriteBlock(uint8_t * buffer, size_t address, size_t size)
{
  eepromStartWrite(buffer, address, size);

  while (!eepromIsTransferComplete()) {
    if (_eeprom_write_simu_delay)
      sleep(1/*ms*/);
  }
}

pthread_t eeprom_thread_pid;

void startEepromThread(const char * filename)
{
  eepromFile = filename;
  if (eepromFile) {
    fp = fopen(eepromFile, "rb+");
    if (!fp)
      fp = fopen(eepromFile, "wb+");
    if (!fp)
      perror("error in fopen");
  }

#ifdef __APPLE__
  eeprom_write_sem = sem_open("eepromsem", O_CREAT, S_IRUSR | S_IWUSR, 0);
#else
  eeprom_write_sem = (sem_t *)malloc(sizeof(sem_t));
  sem_init(eeprom_write_sem, 0, 0);
#endif

  RTOS_CREATE_TASK(eeprom_thread_pid, eeprom_thread_function, "eeprom");
}

void stopEepromThread()
{
  eeprom_thread_running = false;
  sem_post(eeprom_write_sem);
  pthread_join(eeprom_thread_pid, nullptr);

#ifdef __APPLE__
  sem_close(eeprom_write_sem);
#else
  sem_destroy(eeprom_write_sem);
  free(eeprom_write_sem);
#endif

  if (fp)
    fclose(fp);
}
