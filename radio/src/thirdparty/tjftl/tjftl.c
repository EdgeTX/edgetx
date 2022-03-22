//tjflt - Tiny Janky / Journalling Flash Translation Layer
/*
 * Copyright 2019 Jeroen Domburg <jeroen@spritesmods.com>. Licensed under
 * the beer-ware license.
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <jeroen@spritesmods.com> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. -Jeroen
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "tjftl.h"

//Description for one block. For a valid block, lba == ~lba_inv
//A free block has lba == lba_inv == 0xffffffff
//The highest bit has a special purpose: if it's 0 in both lba as well as lba_inv, the
//block is *likely* to be superseded by a newer block. (Note: This is only used in non-cached
//mode.

//Note that flash normally only has one block which is partially full, e.g. has a magic marker
//but lba's with 0xffffffff in it; if it exists, that's also the block with the highest serial.

//Note that the serial for blocks is 'only' 32-bit, which means that there is a maximum of 131 terabyte of
//traffic that can ever be written to the filesystem, assuming we don't implement a way to handle rollover.
//As this means an 16MiB bit of flash is then rewritten about 8 million times, I'd think this is not a very
//large issue.

//Note that this ftl has 1/64th overhead, plus some free blocks for garbage collection.

//#define DEBUG 1
#if DEBUG
#define TJ_MSG(...) do { printf("TJFTL: "); printf(__VA_ARGS__); } while(0)
#define TJ_CHECK(x, msg) do { if (!(x)) { printf("TJFTL: check fail: " #x " (%s:%d): %s\n", __FILE__, __LINE__, msg); abort(); }} while (0)
#else
#define TJ_MSG(...)
#define TJ_CHECK(x, msg)
#endif



//minimal free block count before we go garbage collect
#define GC_MIN_FREE_BLK_CNT 10
//minimum amount of blocks that the garbage collect routine will rewrite
//(if this frees up less than GC_MIN_FREE_BLK_CNT blocks, it will continue until that
//amount of blocks have been freed)
#define GC_CLEAR_BLOCKS 2



#define LBA_FREE 0xFFFFFFFF
#define LBA_MASK 0x7FFFFFFF
#define LBA_SUPERSEDED_MSK 0x80000000

typedef struct {
    uint32_t lba;
    uint32_t lba_inv;
} tjfl_blockdesc_t;


#define BLKSZ 32768
#define BLKHDR_MAGIC 0x1337B33F
#define SEC_PER_BLK 63
#define SEC_DATA_SIZE 512

//This struct is exactly 512 bytes large.
typedef struct {
    uint32_t magic;
    uint32_t serial; //always incrementing per block erased/written
    tjfl_blockdesc_t bd[SEC_PER_BLK];
} tjftl_block_t;

_Static_assert(sizeof(tjftl_block_t)==512, "tjftl_block_t is not 512 bytes!");

//With this enabled, the ftl will keep a cache of where each LBA resides instead of doing a linear search
//of the entire flash every time. This takes up 8K per megabyte cached.
#define CACHE_LBALOC 1

struct tjftl_t {
    flashcb_read_t flash_read;
    flashcb_erase_32k_t flash_erase;
    flashcb_program_t flash_program;
    void *flashcb_arg;
    int backing_blks;
    int sect_cnt;
    int current_serial;
    int current_write_block;
    int current_gc_block;
    int free_blk_cnt; //This has the amount of blocks that are invalid/erased/entirely empty.
    int prefer_first_sectors; //if this is 1, the first few sectors aren't entirely used. Prefer those so detecting a tjftl is easier.
#if CACHE_LBALOC
    uint32_t *lba_cache;
#endif
};

#if CACHE_LBALOC
//Note: cache contains sectors starting from 1, not 0 like the rest of the code, so we can mark
//0 as invalid.
static uint32_t lbacache_pair(int block, int sec) { return (block*64)+sec+1; }
static int lbacache_block(uint32_t cv) { return cv/64; }
static int lbacache_sec(uint32_t cv) { return (cv&63)-1; }
#endif

static bool find_block_for_lba(tjftl_t *tj, int lba, tjftl_block_t *blkh, int *blkno_out, int *sect_in_blk_out);


static bool read_blkhdr(tjftl_t *tj, int blk, tjftl_block_t *hdr) {
    int addr=blk*BLKSZ;
    bool ret=tj->flash_read(addr, (uint8_t*)hdr, sizeof(tjftl_block_t), tj->flashcb_arg);
    return ret;
}

static bool read_sect(tjftl_t *tj, int blk, int sect_in_blk, uint8_t *buf) {
    TJ_CHECK(sect_in_blk>=0 && sect_in_blk<64, "invalid sect in blk");
    int addr=blk*BLKSZ+(sect_in_blk+1)*SEC_DATA_SIZE;
    bool ret=tj->flash_read(addr, buf, SEC_DATA_SIZE, tj->flashcb_arg);
    return ret;
}

static bool write_blkhdr(tjftl_t *tj, int blk, const tjftl_block_t *hdr) {
    int addr=blk*BLKSZ;
    bool ret=tj->flash_program(addr, (uint8_t*)hdr, sizeof(tjftl_block_t), tj->flashcb_arg);
    if (!ret) {
        TJ_MSG("Write_blkhdr: flash_program failed at addr %d\n", addr);
    }
    return ret;
}

static bool write_sect(tjftl_t *tj, int blk, int sect_in_blk, const uint8_t *buf) {
    TJ_CHECK(sect_in_blk>=0 && sect_in_blk<64, "invalid sect in blk");
    int addr=blk*BLKSZ+(sect_in_blk+1)*SEC_DATA_SIZE;
    bool ret=tj->flash_program(addr, buf, SEC_DATA_SIZE, tj->flashcb_arg);
    return ret;
}

static bool lba_valid(const tjfl_blockdesc_t *b) {
    return (b->lba & LBA_MASK) == ((~b->lba_inv) & LBA_MASK);
}

static bool lba_erased(const tjfl_blockdesc_t *b) {
    return ((b->lba == 0xFFFFFFFF) && (b->lba_inv == 0xFFFFFFFF));
}

static int lba_sect(const tjfl_blockdesc_t *b) {
    if (!lba_valid(b)) return -1;
    return b->lba & LBA_MASK;
}


static bool lba_is_superseded(tjftl_t *tj, const tjfl_blockdesc_t *b, int blkno, int sect_in_blk) {
#if !CACHE_LBALOC
    //quick check before we do needless expensive lookup work
    if (!((b->lba & LBA_SUPERSEDED_MSK)==0) && ((b->lba_inv & LBA_SUPERSEDED_MSK)==0)) return false;
#endif
    int f_blkno, f_sect_in_blk;
    if (!lba_valid(b)) return false;
    bool found=find_block_for_lba(tj, lba_sect(b), NULL, &f_blkno, &f_sect_in_blk);
    if (found && (f_blkno!=blkno || f_sect_in_blk!=sect_in_blk)) {
        //yep, there certainly is a newer version
        return true;
    } else {
        return false;
    }
}

static bool lba_maybe_superseded(tjftl_t *tj, const tjfl_blockdesc_t *b, int blkno, int sect_in_blk) {
#if CACHE_LBALOC
    //Cache marks all blocks as maybe superseded, but we can easily check if the block is _actually_ superseded as
    //that routine is really fast.
    return lba_is_superseded(tj, b, blkno, sect_in_blk);
#else
    return ((b->lba & LBA_SUPERSEDED_MSK)==0) && ((b->lba_inv & LBA_SUPERSEDED_MSK)==0);
#endif
}

static bool blkh_valid(const tjftl_block_t *blkh) {
    return (blkh->magic==BLKHDR_MAGIC);
}

static bool blkh_is_empty(const tjftl_block_t *blkh) {
    for (int i=0; i<SEC_PER_BLK; i++) {
        if (!lba_erased(&blkh->bd[i])) return false;
    }
    return true;
}
static int blkh_next_free_sec(const tjftl_block_t *blkh) {
    if (!blkh_valid(blkh)) {
        TJ_MSG("blkh_next_free_sec: block not valid\n");
        return -1;
    }
    for (int i=0; i<SEC_PER_BLK; i++) {
        if (lba_erased(&blkh->bd[i])) {
            return i;
        }
    }
    return -1;
}

static bool blk_initialize(tjftl_t *tj, int blkno, tjftl_block_t *blkh) {
    TJ_MSG("Initializing block %d using serial %d\n", blkno, tj->current_serial+1);
    memset(blkh, 0xff, sizeof(tjftl_block_t));
    blkh->magic=BLKHDR_MAGIC;
    tj->current_serial++;
    blkh->serial=tj->current_serial;
    bool ret;
    ret=tj->flash_erase(blkno*BLKSZ, tj->flashcb_arg);
    if (!ret) {
        TJ_MSG("blk_initialize: flash_erase of block %d failed!\n", blkno);
        return false;
    }
    ret=write_blkhdr(tj, blkno, blkh);
    if (!ret) {
        TJ_MSG("blk_initialize: write_blkhdr of block %d failed!\n", blkno);
    }
    return ret;
}

static void blk_fill_cache(tjftl_t *tj, tjftl_block_t *blkh, int blkno) {
#if CACHE_LBALOC
    for (int j=0; j<SEC_PER_BLK; j++) {
        if (lba_valid(&blkh->bd[j])) {
            int lba=lba_sect(&blkh->bd[j]);
            if (tj->lba_cache[lba]!=0) {
                //Need to see if this lba superseded the other one
                tjftl_block_t oblkh;
                read_blkhdr(tj, lbacache_block(tj->lba_cache[lba]), &oblkh);
//              printf("cache fill: already read lba %d. old ser %d new ser %d\n", lba, oblkh.serial, blkh->serial);
                if (oblkh.serial < blkh->serial) {
                    //Yes, we supersede the old block.
                    tj->lba_cache[lba]=lbacache_pair(blkno, j);
                }
            } else {
                //this is the current version as we do not have an other version.
                tj->lba_cache[lba]=lbacache_pair(blkno, j);
            }
        }
    }
#endif
}

static void cache_update(tjftl_t *tj, int lba, int blkno, int sec) {
#if CACHE_LBALOC
    tj->lba_cache[lba]=lbacache_pair(blkno, sec);
#endif
}

//Check the first 4 blocks. If 2 of them have valid tjftl headers, we assume this is a tjftl
//partition.
int tjftl_detect(flashcb_read_t rf, void *arg) {
    tjftl_block_t blkh;
    int valid_blocks=0;
    for (int blk=0; blk<4; blk++) {
        int addr=blk*BLKSZ;
        bool ret=rf(addr, (uint8_t*)&blkh, sizeof(tjftl_block_t), arg);
        if (!ret) return 0; //flash error = not detected
        if (blkh_valid(&blkh)) valid_blocks++;
    }
    return valid_blocks>=2;
}

static bool garbage_collect(tjftl_t *tj);

tjftl_t *tjftl_init(flashcb_read_t rf, flashcb_erase_32k_t ef, flashcb_program_t pf, void *arg, int size, int sect_cnt, int verbose) {
    TJ_MSG("Initializing tjftl with size=%d, sect_cnt %d\n", size, sect_cnt);
    tjftl_t *ret=calloc(sizeof(tjftl_t), 1);
    if (!ret) return NULL;
#if CACHE_LBALOC
    ret->lba_cache=calloc(sect_cnt, sizeof(uint32_t));
    if (!ret->lba_cache) {
        free(ret);
        return NULL;
    }
    if (verbose) printf("tjfl: allocated %d bytes for cache\n", sect_cnt*sizeof(uint32_t));
#endif
    ret->flash_read=rf;
    ret->flash_erase=ef;
    ret->flash_program=pf;
    ret->flashcb_arg=arg;
    ret->backing_blks=size/BLKSZ;
    ret->sect_cnt=sect_cnt;
    ret->current_serial=0;
    ret->current_write_block=-1;
    ret->current_gc_block=-1;
    ret->free_blk_cnt=0;
    ret->prefer_first_sectors=0;
    bool all_ok=true;
    for (int i=0; i<ret->backing_blks; i++) {
        tjftl_block_t blkh;
        all_ok&=read_blkhdr(ret, i, &blkh);
        //If block is invalid or erased it counts as a free block for free_blk_cnt.
        if (blkh_valid(&blkh)) {
            if (!blkh_is_empty(&blkh)) {
                if (blkh.serial > ret->current_serial) ret->current_serial=blkh.serial;
                blk_fill_cache(ret, &blkh, i);
            }
        } else {
            if (i<4) ret->prefer_first_sectors=1;
            ret->free_blk_cnt++;
        }
    }
    if (verbose) printf("tjfl: %d of %d blocks free.\n", ret->free_blk_cnt, ret->backing_blks);
    if (ret->free_blk_cnt<GC_MIN_FREE_BLK_CNT) {
        TJ_MSG("Starting initial garbage collection run...\n");
        all_ok&=garbage_collect(ret);
        if (verbose) printf("After garbage collection: %d blocks free.\n", ret->free_blk_cnt);
    }
    if (!all_ok) {
        TJ_MSG("ERROR! tjftl failed to initialize.\n");
#if CACHE_LBALOC
        free(ret->lba_cache);
#endif
        free(ret);
        return NULL;
    } else {
        TJ_MSG("Tjftl initialized and ready.\n");
        return ret;
    }
}

//Find the (most recent) block for a given LBA
static bool find_block_for_lba(tjftl_t *tj, int lba, tjftl_block_t *blkh, int *blkno_out, int *sect_in_blk_out) {
#if CACHE_LBALOC
    if (tj->lba_cache[lba]==0) return false;
    *blkno_out=lbacache_block(tj->lba_cache[lba]);
    *sect_in_blk_out=lbacache_sec(tj->lba_cache[lba]);
    if (blkh!=NULL) {
        return read_blkhdr(tj, *blkno_out, blkh);
    } else {
        return true;
    }
#else
    if (blkh==NULL) blkh=alloca(sizeof(tjftl_block_t));

    //Do it manually...
    int last_ser=0;
    bool found=false;
    bool ret;
    for (int i=0; i<tj->backing_blks; i++) {
        ret=read_blkhdr(tj, i, blkh);
        if (!ret) return false;
        if (blkh_valid(blkh)) {
//          TJ_MSG("find_block_for_lba: looking for lba %d in block %d...\n", lba, i);
            for (int j=0; j<SEC_PER_BLK; j++) {
                //See if 1. lba is valid, 2. it's the lba we're searching for, and 3. if this is newer than what we had
//              printf("%d: %s valid, lba %d, ser %d (cur %d)\n",j ,lba_valid(&blkh->bd[j])?"is":"not", lba_sect(&blkh->bd[j]), blkh->serial, last_ser);
                if (lba_valid(&blkh->bd[j]) && (lba_sect(&blkh->bd[j])==lba) && (last_ser < blkh->serial)) {
                    //We found a possible block.
                    *blkno_out=i;
                    *sect_in_blk_out=j;
                    found=true;
                    if (!lba_maybe_superseded(tj, &blkh->bd[j]), i, j) {
//                      TJ_MSG("find_block_for_lba: found lba %d in block %d, sec %d.\n", lba, i, j);
                        //Block is current, we found its data, yay!
                        return true;
                    } else {
//                      TJ_MSG("find_block_for_lba: found maybe-superseded lba %d in block %d, sec %d.\n", lba, i, j);
                    }
                }
            }
        }
    }
    return found;
#endif
}


//This will find blocks with superseeded sectors in it and re-write the non-superseeded blocks
//to empty sectors. Once that is done, it will clear the sector so it can be re-used.
static bool garbage_collect(tjftl_t *tj) {
    int find_start=rand()%tj->backing_blks; //random starting point, yay wear leveling!
    int blkno=find_start;
    int gc_todo = GC_CLEAR_BLOCKS;
    tjftl_block_t blkh;
    bool ret;
    while (gc_todo>0 || tj->free_blk_cnt < GC_MIN_FREE_BLK_CNT) {
        read_blkhdr(tj, blkno, &blkh);
        bool should_gc=false;
        if (blkh_valid(&blkh) && blkno!=tj->current_write_block) {
            //Block is valid, see if there are superseded sectors or free sectors here.
            //We also gc on free sectors, as we cannot re-use them as the serial may be wrong.
            //Note that sectors are marked invalid when garbage collected, and only initialized to
            //all-empty when selected for write.
            for (int j=0; j<SEC_PER_BLK; j++) {
                if (lba_erased(&blkh.bd[j]) || lba_maybe_superseded(tj, &blkh.bd[j], blkno, j)) {
                    should_gc=true;
                }
            }
            //every now and then, move around a static block as well
            if ((rand()&0xff)==0) {
                should_gc=true;
            }
        }
        if (should_gc) {
            tj->current_gc_block=blkno;
            //Look at all the sectors, rewrite them if needed
            TJ_MSG("Starting garbage collect round. ToDo=%d, free_cnt=%d; cleaning up blk %d\n", gc_todo, tj->free_blk_cnt, blkno);
            int  moved=0;
            for (int j=0; j<SEC_PER_BLK; j++) {
                if (lba_valid(&blkh.bd[j]) && !lba_erased(&blkh.bd[j]) && !lba_is_superseded(tj, &blkh.bd[j], blkno, j)) {
                    uint8_t buf[SEC_DATA_SIZE];
                    ret=read_sect(tj, blkno, j, buf);
                    if (!ret) return false;
//                  TJ_MSG("Garbage collect: writing block %d sec %d (lba %d)\n", blkno, j, lba_sect(&blkh.bd[j]));
                    ret=tjftl_write(tj, lba_sect(&blkh.bd[j]), buf);
                    if (!ret) return false;
                    moved++;
                }
            }
            //Note: invalidate instead of initialize as we don't know what the serial is going to be when we
            //are going to use this. The write routine will erase and initialize when it gets to it.
            blkh.magic=0; //break block
            write_blkhdr(tj, blkno, &blkh);
            tj->free_blk_cnt++; //yaaaay
            gc_todo--;
            tj->current_gc_block=-1;
            TJ_MSG("Did garbage collect round. ToDo=%d, free_cnt=%d; cleaned up blk %d by moving %d sects\n", gc_todo, tj->free_blk_cnt, blkno, moved);
        }
        blkno++;
        if (blkno>=tj->backing_blks) blkno=0;
    }
    TJ_MSG("Garbage collection done; free_blk_cnt=%d.\n", tj->free_blk_cnt);
    return true;
}

bool tjftl_read(tjftl_t *tj, int lba, uint8_t *buf) {
    bool ret;
    int blkno, sect_in_blk;
    bool found=find_block_for_lba(tj, lba, NULL, &blkno, &sect_in_blk);
    if (found) {
//      TJ_MSG("tjftl_read: lba %d found in blk %d sect %d.\n", lba, blkno, sect_in_blk);
        ret=read_sect(tj, blkno, sect_in_blk, buf);
        return ret;
    } else {
//      TJ_MSG("tjftl_read: lba not found.\n");
        memset(buf, 0xff, SEC_DATA_SIZE);
        return true;
    }
}


bool tjftl_write(tjftl_t *tj, int lba, const uint8_t *buf) {
    tjftl_block_t blkh;
    bool ret;
    TJ_CHECK(lba>=0 && lba<tj->sect_cnt, "lba fucky");

//  TJ_MSG("tjfl_write lba %d, current_write_block %d\n", lba, tj->current_write_block);
    //First, find current version of the block and mark as maybe-superseded.
    //cache doesn't get a speed boost from non-superseded sectors, so we mark everything as superseded
    //from the start when we initially write the sector.
#if !CACHE_LBALOC
    int blkno, sect_in_blk;
    bool found=find_block_for_lba(tj, lba, &blkh, &blkno, &sect_in_blk);
    if (found && !lba_maybe_superseded(tj, &blkh.bd[sect_in_blk]), blkno, sect_in_blk) {
        TJ_MSG("tjfl_write: marking old sect (blk %d sec %d) as superseded\n", blkno, sect_in_blk);
        blkh.bd[sect_in_blk].lba &= ~LBA_SUPERSEDED_MSK;
        blkh.bd[sect_in_blk].lba_inv &= ~LBA_SUPERSEDED_MSK;
        ret=write_blkhdr(tj, blkno, &blkh);
        if (!ret) return false;
    }
#endif

    if (tj->current_write_block == -1) {
        //We don't have a block that can accept another sector. We need to do some effort to find one...
        //Let's look for a block that is either entirely empty, is invalid, or has some empty sectors in it.
        int find_start;
        if (tj->prefer_first_sectors) {
            find_start=0; //start allocating at the beginning
        } else {
            find_start=rand()%tj->backing_blks; //random starting point, yay wear leveling!
        }

        int blkno=find_start;
        TJ_MSG("tjfl_write: find new empty block, start at: %d, free_cnt=%d\n", blkno, tj->free_blk_cnt);
        do {
            ret=read_blkhdr(tj, blkno, &blkh);
            if (!ret) return false;
            if (blkno!=tj->current_gc_block && !blkh_valid(&blkh)) {
                //Found an invalid/erased block! Initialize it.
                TJ_MSG("tjfl_write: %d is invalid or empty: using it\n", blkno);
                ret=blk_initialize(tj, blkno, &blkh);
                if (!ret) {
                TJ_MSG("tjftl_write: Block initialize failed\n");
                    return false;
                }
                tj->current_write_block = blkno;
            } else {
                //No dice.
                //Note that we don't grab any blocks that exist and may have some free sectors, as these
                //may have an older serial and we can't update it without running the risk of making the
                //old sectors superseded.
                blkno++;
                if (blkno>=tj->backing_blks) blkno=0;
            }
        } while (tj->current_write_block == -1 && blkno!=find_start);
        if (blkno>4) tj->prefer_first_sectors=0;
    } else {
        //We already have an active block. Grab its header.
        ret=read_blkhdr(tj, tj->current_write_block, &blkh);
        if (!ret) {
            TJ_MSG("Huh? Read_blkhdr failed for block %d\n", tj->current_write_block);
            return false;
        }
    }
    if (tj->current_write_block == -1) {
        TJ_MSG("WtF, no free block found?\n");
        return false;
    }

    //We have a currently-active block with some free space when we end up here.
    //The (current) header is in blkh.
    int free_sec_in_blk=blkh_next_free_sec(&blkh);
    TJ_CHECK(free_sec_in_blk!=-1, "block should have free sec");
//  TJ_MSG("Going to write data to blk %d sec %d\n", tj->current_write_block, free_sec_in_blk);
    ret=write_sect(tj, tj->current_write_block, free_sec_in_blk, buf);
    if (!ret) {
        TJ_MSG("Write sect failed\n");
        return false;
    }
    blkh.bd[free_sec_in_blk].lba=lba;
    blkh.bd[free_sec_in_blk].lba_inv=~lba;
#if CACHE_LBALOC
    //We always mark blocks as superseded.
    blkh.bd[free_sec_in_blk].lba &= ~LBA_SUPERSEDED_MSK;
    blkh.bd[free_sec_in_blk].lba_inv &= ~LBA_SUPERSEDED_MSK;
    //Extra-special todo: if the old lba is in this block as well, nuke it, as the serial won't help us anymore.
    int oldblkno, oldsec;
    if (find_block_for_lba(tj, lba, NULL, &oldblkno, &oldsec)) {
        if (oldblkno==tj->current_write_block) {
            blkh.bd[oldsec].lba=0;
            blkh.bd[oldsec].lba_inv=0;
        }
    }
#endif
    ret=write_blkhdr(tj, tj->current_write_block, &blkh);
    if (!ret) {
        TJ_MSG("Write block header failed\n");
        return false;
    }
    cache_update(tj, lba, tj->current_write_block, free_sec_in_blk);
    //see if we used up the current block; if so we need to find a new one next
    //time. Also check if we need to gc.
    if (blkh_next_free_sec(&blkh)==-1) {
        TJ_MSG("Block %d ran out of space.\n", tj->current_write_block);
        tj->current_write_block=-1;
        tj->free_blk_cnt--; //technically should already decrease after the first sector is written, but
                            //we can safely do it here as well.
        //Garbage collect if we run out of free blocks, but not if we're already collecting garbage.
        if (tj->free_blk_cnt<GC_MIN_FREE_BLK_CNT && tj->current_gc_block==-1) {
            bool r=garbage_collect(tj);
            if (!r) {
                TJ_MSG("Garbage collect failed.\n");
                return false;
            }
        }
    }
    return true;
}

size_t tjftl_getSectorCount(tjftl_t *tj)
{
  return tj->sect_cnt;
}

size_t tjftl_getSectorSize(tjftl_t *tj)
{
  return SEC_DATA_SIZE;
}

