/*
 * Copyright (c) 2016 Adafruit Industries
 * 
 * MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "uf2_ghostfat.h"
#include "board.h"

#include "uf2/uf2.h"
#include "io/uf2.h"

#include "hal/flash_driver.h"
#include "fw_desc.h"

#include "debug.h"

#include <string.h>

typedef struct {
    uint8_t JumpInstruction[3];
    uint8_t OEMInfo[8];
    uint16_t SectorSize;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FATCopies;
    uint16_t RootDirectoryEntries;
    uint16_t TotalSectors16;
    uint8_t MediaDescriptor;
    uint16_t SectorsPerFAT;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;
    uint8_t PhysicalDriveNum;
    uint8_t Reserved;
    uint8_t ExtendedBootSig;
    uint32_t VolumeSerialNumber;
    uint8_t VolumeLabel[11];
    uint8_t FilesystemIdentifier[8];
} __attribute__((packed)) FAT_BootBlock;

typedef struct {
    char name[8];
    char ext[3];
    uint8_t attrs;
    uint8_t reserved;
    uint8_t createTimeFine;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t highStartCluster;
    uint16_t updateTime;
    uint16_t updateDate;
    uint16_t startCluster;
    uint32_t size;
} __attribute__((packed)) DirEntry;
static_assert(sizeof(DirEntry) == 32);

struct TextFile {
  char const name[11];
  char const *content;
};

#define NUM_FAT_BLOCKS UF2_NUM_BLOCKS
#define UF2_VOLUME_LABEL {'E', 'D', 'G', 'E', 'T', 'X', '_', 'U', 'F', '2', ' ' }
#define UF2_INDEX_URL "https://edgetx.org"

#define STR0(x) #x
#define STR(x) STR0(x)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define __YEAR_INT__ ((( \
  (__DATE__ [ 7u] - '0')  * 10u + \
  (__DATE__ [ 8u] - '0')) * 10u + \
  (__DATE__ [ 9u] - '0')) * 10u + \
  (__DATE__ [10u] - '0'))

#define __MONTH_INT__ ( \
  (__DATE__ [2u] == 'n' && __DATE__ [1u] == 'a') ?  1u  /*Jan*/ \
: (__DATE__ [2u] == 'b'                        ) ?  2u  /*Feb*/ \
: (__DATE__ [2u] == 'r' && __DATE__ [1u] == 'a') ?  3u  /*Mar*/ \
: (__DATE__ [2u] == 'r'                        ) ?  4u  /*Apr*/ \
: (__DATE__ [2u] == 'y'                        ) ?  5u  /*May*/ \
: (__DATE__ [2u] == 'n'                        ) ?  6u  /*Jun*/ \
: (__DATE__ [2u] == 'l'                        ) ?  7u  /*Jul*/ \
: (__DATE__ [2u] == 'g'                        ) ?  8u  /*Aug*/ \
: (__DATE__ [2u] == 'p'                        ) ?  9u  /*Sep*/ \
: (__DATE__ [2u] == 't'                        ) ? 10u  /*Oct*/ \
: (__DATE__ [2u] == 'v'                        ) ? 11u  /*Nov*/ \
:                                                  12u  /*Dec*/ )

#define __DAY_INT__ ( \
   (__DATE__ [4u] == ' ' ? 0u : __DATE__ [4u] - '0') * 10u \
 + (__DATE__ [5u] - '0')                                   )

// __TIME__ expands to an eight-character string constant
// "23:59:01", or (if cannot determine time) "??:??:??" 
#define __HOUR_INT__ ( \
   (__TIME__ [0u] == '?' ? 0u : __TIME__ [0u] - '0') * 10u \
 + (__TIME__ [1u] == '?' ? 0u : __TIME__ [1u] - '0')       )

#define __MINUTE_INT__ ( \
   (__TIME__ [3u] == '?' ? 0u : __TIME__ [3u] - '0') * 10u \
 + (__TIME__ [4u] == '?' ? 0u : __TIME__ [4u] - '0')       )

#define __SECONDS_INT__ ( \
   (__TIME__ [6u] == '?' ? 0u : __TIME__ [6u] - '0') * 10u \
 + (__TIME__ [7u] == '?' ? 0u : __TIME__ [7u] - '0')       )


#define __DOSDATE__ ( \
	((__YEAR_INT__  - 1980u) << 9u) | \
	( __MONTH_INT__          << 5u) | \
	( __DAY_INT__            << 0u) )

#define __DOSTIME__ ( \
	( __HOUR_INT__    << 11u) | \
	( __MINUTE_INT__  <<  5u) | \
	( __SECONDS_INT__ <<  0u) )

#include "stamp.h"

const char infoUf2File[] =
    "UF2 Bootloader " VERSION "\r\n"
    "Board-ID: " FLAVOUR "\r\n"
    "Date: " DATE "\r\n";

const char indexFile[] = //
    "<!doctype html>\n"
    "<html>"
    "<body>"
    "<script>\n"
    "location.replace(\"" UF2_INDEX_URL "\");\n"
    "</script>"
    "</body>"
    "</html>\n";

// WARNING -- code presumes only one NULL .content for .UF2 file
//            and requires it be the last element of the array
static struct TextFile const info[] = {
    {.name = {'I','N','F','O','_','U','F','2','T','X','T'}, .content = infoUf2File},
    {.name = {'I','N','D','E','X',' ',' ',' ','H','T','M'}, .content = indexFile},
    {.name = {'C','U','R','R','E','N','T',' ','U','F','2'}, .content = nullptr},
};

// WARNING -- code presumes each non-UF2 file content fits in single sector
//            Cannot programmatically statically assert .content length
//            for each element above.
static_assert(ARRAY_SIZE(indexFile) < 512);


#define NUM_FILES (ARRAY_SIZE(info))
#define NUM_DIRENTRIES (NUM_FILES + 1) /* Code adds volume label as first root directory entry */

#ifndef BOOTLOADER_ADDRESS
#define BOOTLOADER_ADDRESS FIRMWARE_ADDRESS
#endif

#define UF2_SIZE           (current_flash_size() * 2 + 512 * REBOOT_BLOCK)
#define UF2_SECTORS        (UF2_SIZE / 512)
#define UF2_FIRST_SECTOR   (NUM_FILES + 1) /* WARNING -- code presumes each non-UF2 file content fits in single sector */
#define UF2_LAST_SECTOR    (UF2_FIRST_SECTOR + UF2_SECTORS - 1)

#define RESERVED_SECTORS   1
#define ROOT_DIR_SECTORS   4
#define SECTORS_PER_FAT    ((NUM_FAT_BLOCKS * 2 + 511) / 512) /* 256 sectors per FAT */

#define START_FAT0         RESERVED_SECTORS
#define START_FAT1         (START_FAT0 + SECTORS_PER_FAT)        /* 1 + 256 */
#define START_ROOTDIR      (START_FAT1 + SECTORS_PER_FAT)        /* 1 + 256 + 256 */
#define START_CLUSTERS     (START_ROOTDIR + ROOT_DIR_SECTORS)    /* 1 + 256 + 256 + 4 */
#define DATA_SECTORS       (NUM_FAT_BLOCKS - 2 - START_CLUSTERS) /* 65535 - 2 - (1 + 256 + 256 + 4) = 65016 */

#if BOOTLOADER_ADDRESS == FIRMWARE_ADDRESS
#define REBOOT_BLOCK 0
#else
#define REBOOT_BLOCK 1
#endif

#define UF2_MAX_FW_SIZE (512 * (DATA_SECTORS / 2 - REBOOT_BLOCK) / 2) /* 8126 kB */

#if defined(FLASHSIZE)
static_assert(FLASHSIZE > UF2_MAX_FW_SIZE, "FLASHSIZE is smaller than UF2_MAX_FW_SIZE");
#endif

#define UF2_MAX_BLOCKS (UF2_MAX_FW_SIZE / 256)

#define UF2_ERASE_BLOCK_SIZE (4 * 1024)
#define UF2_ERASE_BLOCKS (UF2_MAX_FW_SIZE / UF2_ERASE_BLOCK_SIZE)

// all directory entries must fit in a single sector
// because otherwise current code overflows buffer
#define DIRENTRIES_PER_SECTOR (512/sizeof(DirEntry))

static_assert(NUM_DIRENTRIES < DIRENTRIES_PER_SECTOR * ROOT_DIR_SECTORS);

static FAT_BootBlock const BootBlock = {
    .JumpInstruction      = {0xeb, 0x3c, 0x90},
    .OEMInfo              = {'M','S','W','I','N','4','.','1'},
    .SectorSize           = 512,
    .SectorsPerCluster    = 1,
    .ReservedSectors      = RESERVED_SECTORS,
    .FATCopies            = 2,
    .RootDirectoryEntries = (ROOT_DIR_SECTORS * DIRENTRIES_PER_SECTOR),
    .TotalSectors16       = 0,
    .MediaDescriptor      = 0xF8,
    .SectorsPerFAT        = SECTORS_PER_FAT,
    .SectorsPerTrack      = 1,
    .Heads                = 1,
    .TotalSectors32       = NUM_FAT_BLOCKS - 2,
    .PhysicalDriveNum     = 0x80, // to match MediaDescriptor of 0xF8
    .ExtendedBootSig      = 0x29,
    .VolumeSerialNumber   = 0x00000000,
    .VolumeLabel          = UF2_VOLUME_LABEL,
    .FilesystemIdentifier = {'F','A','T','1','6',' ',' ',' '},
};

static uf2_fat_write_state_t _uf2_write_state;
static uint32_t _flash_sz;

static uint32_t _written_mask[UF2_MAX_BLOCKS / sizeof(uint32_t)];
static uint32_t _erased_mask[UF2_ERASE_BLOCKS / sizeof(uint32_t)];

void uf2_fat_reset_state()
{
  _flash_sz = 0;
  memset(&_uf2_write_state, 0, sizeof(_uf2_write_state));
  memset(_written_mask, 0, sizeof(_written_mask));
  memset(_erased_mask, 0, sizeof(_erased_mask));
}

const uf2_fat_write_state_t* uf2_fat_get_state()
{
  return &_uf2_write_state;
}

static inline bool is_firmware_length_valid(uint32_t len)
{
  return len && len <= UF2_MAX_FW_SIZE;
}

static inline bool is_address_in_firmware(uintptr_t addr)
{
  return (addr & FIRMWARE_ADDRESS) == FIRMWARE_ADDRESS;
}

static inline bool is_firmware_valid(firmware_description_t const* fw_desc)
{
  return fw_desc && is_firmware_length_valid(fw_desc->length) &&
         is_address_in_firmware(fw_desc->version_ptr) &&
         is_address_in_firmware((uintptr_t)fw_desc->reset_handler);
}

// get current.uf2 flash size in bytes, round up to 256 bytes
static uint32_t current_flash_size(void)
{
  uint32_t result = _flash_sz; // presumes atomic 32-bit read/write and static result

  // only need to compute once
  if ( result == 0 ) {
    firmware_description_t const *fw_desc =
        (firmware_description_t const *)APP_START_ADDRESS;

    if (is_firmware_valid(fw_desc)) {
      // round up to 256 bytes
      result = (fw_desc->length + BOOTLOADER_SIZE + 255U) & (~255U);
      // Sometime corrupted firmware return very large size, leave not enough space to flash a fix
      if (result >= UF2_MAX_FW_SIZE)
        result = UF2_MAX_FW_SIZE;
    } else {
      result = UF2_MAX_FW_SIZE;
    }
    _flash_sz = result; // presumes atomic 32-bit read/write and static result
  }

  return _flash_sz;
}

static void padded_memcpy (char *dst, char const *src, int len)
{
    for (int i = 0; i < len; ++i) {
        if (*src)
            *dst = *src++;
        else
            *dst = ' ';
        dst++;
    }
}


/*------------------------------------------------------------------*/
/* Read
 *------------------------------------------------------------------*/
void uf2_fat_read_block(uint32_t block_no, uint8_t *data)
{
    memset(data, 0, 512);
    uint32_t sectionIdx = block_no;

    if (block_no == 0) { // Requested boot block
        memcpy(data, &BootBlock, sizeof(BootBlock));
        data[510] = 0x55;
        data[511] = 0xaa;
        // logval("data[0]", data[0]);
    } else if (block_no < START_ROOTDIR) {  // Requested FAT table sector
        sectionIdx -= START_FAT0;
        // logval("sidx", sectionIdx);
        if (sectionIdx >= SECTORS_PER_FAT)
            sectionIdx -= SECTORS_PER_FAT; // second FAT is same as the first...
        if (sectionIdx == 0) {
            data[0] = 0xf8; // first FAT entry must match BPB MediaDescriptor
            // WARNING -- code presumes only one NULL .content for .UF2 file
            //            and all non-NULL .content fit in one sector
            //            and requires it be the last element of the array
            for (unsigned i = 1; i < NUM_FILES * 2 + 4; ++i) {
                data[i] = 0xff;
            }
        }
        for (unsigned i = 0; i < 256; ++i) { // Generate the FAT chain for the firmware "file"
            uint32_t v = sectionIdx * 256 + i;
            if (UF2_FIRST_SECTOR <= v && v <= UF2_LAST_SECTOR)
                ((uint16_t *)(void *)data)[i] = v == UF2_LAST_SECTOR ? 0xffff : v + 1;
        }
    } else if (block_no < START_CLUSTERS) { // Requested root directory sector

        sectionIdx -= START_ROOTDIR;

        DirEntry *d = (DirEntry *)data;
        int remainingEntries = DIRENTRIES_PER_SECTOR;
        if (sectionIdx == 0) { // volume label first
            // volume label is first directory entry
            padded_memcpy(d->name, (char const *) BootBlock.VolumeLabel, 11);
            d->attrs = 0x28;
            d++;
            remainingEntries--;
        }

        for (unsigned i = DIRENTRIES_PER_SECTOR * sectionIdx;
             remainingEntries > 0 && i < NUM_FILES;
             i++, d++) {

            // WARNING -- code presumes all but last file take exactly one sector
            uint16_t startCluster = i + 2;

            struct TextFile const * inf = &info[i];
            padded_memcpy(d->name, inf->name, 11);
            d->createTimeFine   = __SECONDS_INT__ % 2 * 100;
            d->createTime       = __DOSTIME__;
            d->createDate       = __DOSDATE__;
            d->lastAccessDate   = __DOSDATE__;
            d->highStartCluster = startCluster >> 8;
            // DIR_WrtTime and DIR_WrtDate must be supported
            d->updateTime       = __DOSTIME__;
            d->updateDate       = __DOSDATE__;
            d->startCluster     = startCluster & 0xFF;
            // WARNING -- code presumes only one NULL .content for .UF2 file
            //            and requires it be the last element of the array
            d->size = inf->content ? strlen(inf->content) : UF2_SIZE;
        }

    } else {
        sectionIdx -= START_CLUSTERS;
        if (sectionIdx < NUM_FILES - 1) {
            memcpy(data, info[sectionIdx].content, strlen(info[sectionIdx].content));
        } else { // generate the UF2 file data on-the-fly
            sectionIdx -= NUM_FILES - 1;

            uint32_t addr = 0;
            if (sectionIdx >= UF2_MAX_FW_SIZE / 256 + REBOOT_BLOCK) return;

            UF2_Block *bl = (UF2_Block *)data;
            bl->magicStart0 = UF2_MAGIC_START0;
            bl->magicStart1 = UF2_MAGIC_START1;
            bl->magicEnd = UF2_MAGIC_END;
            bl->blockNo = sectionIdx;
            bl->numBlocks = current_flash_size() / 256 + REBOOT_BLOCK;
            bl->flags = 0;
            bl->reserved = 0;

            if (REBOOT_BLOCK && (sectionIdx == BOOTLOADER_SIZE / 256)) {
                writeUF2RebootBlock(bl);
                writeUF2FirmwareVersion(bl);
            } else {
                if (sectionIdx < BOOTLOADER_SIZE / 256) {
                    addr = BOOTLOADER_ADDRESS + sectionIdx * 256;
                } else {
                    sectionIdx -= BOOTLOADER_SIZE / 256 + REBOOT_BLOCK;
                    addr = APP_START_ADDRESS + sectionIdx * 256;
                }

                bl->targetAddr = addr;
                bl->payloadSize = 256;
                memcpy(bl->data, (void *)addr, bl->payloadSize);
                writeUF2FirmwareVersion(bl);
            }
        }
    }
}

/*------------------------------------------------------------------*/
/* Write UF2
 *------------------------------------------------------------------*/

/** Write an block
 *
 * @return number of bytes processed, only the following values
 *  -1 : if not an uf2 block
 * 512 : write is successful
 */
int uf2_fat_write_block(uint32_t block_no, uint8_t *data)
{
    uf2_fat_write_state_t *wr_st = &_uf2_write_state;
    UF2_Block *bl = (UF2_Block *)data;
    // TRACE("Write magic: %x", bl->magicStart0);

    if (!isUF2Block((const uint8_t*)bl, 512)) {
        return -1;
    }

    // only accept block with same family id
    if (UF2_FAMILY_ID && !((bl->flags & UF2_FLAG_FAMILY_ID) &&
                           (bl->reserved == UF2_FAMILY_ID))) {
        return -1;
    }

    if ((bl->flags & UF2_FLAG_NOFLASH) || bl->payloadSize > 256 ||
        (bl->targetAddr & 0xff) || bl->targetAddr < BOOTLOADER_ADDRESS ||
        bl->targetAddr + bl->payloadSize >
            (FIRMWARE_ADDRESS + UF2_MAX_FW_SIZE)) {
        // block doesn't seem valid; we still want to count these blocks
        // to reset properly
    } else {
        uint32_t wr_block = bl->blockNo;
        if (REBOOT_BLOCK && (wr_block >= BOOTLOADER_SIZE / 256)) wr_block--;

        uint32_t erase_sector = wr_block / (UF2_ERASE_BLOCK_SIZE / 256);
        uint32_t mask = 1 << (erase_sector & 0x1F);
        uint32_t pos = erase_sector >> 5;

        uint32_t addr = bl->targetAddr;
        if (wr_st && !(_erased_mask[pos] & mask)) {
            TRACE_DEBUG("[UF2] erase 0x%08x\n", bl->targetAddr);

            auto drv = flashFindDriver(addr);
            if (drv) {
                if (drv->erase_sector(addr) < 0) return -1;

                // mark additional erased sectors
                uint32_t sector = drv->get_sector(addr);
                uint32_t sect_len = drv->get_sector_size(sector);
                uint32_t media_len = flashGetSize(addr);
                if (media_len < sect_len) sect_len = media_len;
                uint32_t erased_sectors = sect_len / UF2_ERASE_BLOCK_SIZE;

                while (erased_sectors-- != 0) {
                    pos = erase_sector >> 5;
                    mask = 1 << (erase_sector & 0x1F);
                    _erased_mask[pos] |= mask;
                    erase_sector++;
                }
            }
        }

        TRACE_DEBUG("[UF2] write 0x%08x\n", bl->targetAddr);

        auto drv = flashFindDriver(addr);
        if (drv) {
          uint32_t len = bl->payloadSize;
          uint8_t* data = (uint8_t*)bl->data;
          if (drv->program(addr, data, len) < 0) return -1;
        }
    }

    if (wr_st && bl->numBlocks) {
        if (wr_st->num_blocks != bl->numBlocks) {
            if (bl->numBlocks >= UF2_MAX_BLOCKS || wr_st->num_blocks) {
                wr_st->num_blocks = UF2_INVALID_NUM_BLOCKS;
            } else {
                wr_st->num_blocks = bl->numBlocks;
            }
        }
        if (bl->blockNo < UF2_MAX_BLOCKS) {
            uint32_t mask = 1 << (bl->blockNo & 0x1F);
            uint32_t pos = bl->blockNo >> 5;
            if (!(_written_mask[pos] & mask)) {
                _written_mask[pos] |= mask;
                wr_st->num_written++;
                TRACE_DEBUG("[UF2] wr #%d (%d / %d)\n", bl->blockNo,
                            wr_st->num_written, bl->numBlocks);
            }
            if (wr_st->num_written >= wr_st->num_blocks) {
                TRACE_DEBUG("[UF2] done: reboot\n");
            }
        }
    }

    return 512;
}
