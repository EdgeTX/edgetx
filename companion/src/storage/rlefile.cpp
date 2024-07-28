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
#include <inttypes.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "eeprominterface.h"
#include "rlefile.h"

RleFile::RleFile():
eeprom(NULL),
eeprom_size(0),
eeFsArm(NULL),
eepromFatHeader(NULL)
{
}

#define EEPROM_SIZE           (4*1024*1024/8)
#define EEPROM_BLOCK_SIZE     (4*1024)
#define EEPROM_MARK           0x84697771 /* thanks ;) */
#define EEPROM_ZONE_SIZE      (8*1024)
#define EEPROM_FAT_SIZE       128
#define EEPROM_MAX_ZONES      (EEPROM_SIZE / EEPROM_ZONE_SIZE)
#define EEPROM_MAX_FILES      (EEPROM_MAX_ZONES - 1)
#define FIRST_FILE_AVAILABLE  (1+MAX_MODELS)

void RleFile::EeFsCreate(uint8_t *eeprom, int size, Board::Type board, unsigned int version)
{
  this->eeprom = eeprom;
  this->eeprom_size = size;
  this->board = board;
  this->version = version;

  if (IS_SKY9X(board)) {
    memset(eeprom, 0xFF, size);
    if (version >= 217) {
      memset(eeprom, 0, EEPROM_FAT_SIZE);
      eepromFatHeader = (EepromHeader *)(eeprom + EEPROM_FAT_SIZE);
      eepromFatHeader->mark = EEPROM_MARK;
      eepromFatHeader->index = 1;
      for (int i=0; i<EEPROM_MAX_FILES; i++) {
        eepromFatHeader->files[i].exists = 0;
        eepromFatHeader->files[i].zoneIndex = i+1;
      }
    }
  }
  else {
    eeFsArm = (EeFsArm *)eeprom;
    eeFsVersion = 5;
    eeFsSize = 8+4*62;
    eeFsBlockSize = 64;
    eeFsFirstBlock = 1;
    eeFsBlocksOffset = eeFsSize - eeFsBlockSize;
    eeFsBlocksMax = 1 + (Boards::getEEpromSize(board)-eeFsSize) / eeFsBlockSize;
    eeFsLinkSize = sizeof(int16_t);
    memset(eeprom, 0, size);
    eeFsArm->version  = eeFsVersion;
    eeFsArm->mySize   = eeFsSize;
    eeFsArm->freeList = 0;
    eeFsArm->bs       = 64;
    for (unsigned int i=eeFsFirstBlock; i<eeFsBlocksMax-1; i++)
      EeFsSetLink(i, i+1);
    EeFsSetLink(eeFsBlocksMax-1, 0);
    eeFsArm->freeList = eeFsFirstBlock;
    // EeFsFlush();
  }
}

PACK(struct EepromFileHeader
{
  uint16_t fileIndex;
  uint16_t size;
});

bool RleFile::searchFat()
{
  eepromFatHeader = NULL;
  uint32_t bestFatIndex = 0;
  for (int i=0; i<EEPROM_ZONE_SIZE/EEPROM_FAT_SIZE; i++) {
    EepromHeader * header = (EepromHeader *)(eeprom+i*EEPROM_FAT_SIZE);
    if (header->mark == EEPROM_MARK && header->index >= bestFatIndex) {
      eepromFatHeader = header;
      bestFatIndex = header->index;
    }
  }
  return (eepromFatHeader != NULL);
}

bool RleFile::EeFsOpen(uint8_t *eeprom, int size, Board::Type board)
{
  this->eeprom = eeprom;
  this->eeprom_size = size;
  this->board = board;

  if (IS_SKY9X(board)) {
    searchFat();
    return 1;
  }
  else {
    eeFsArm = (EeFsArm *)eeprom;
    eeFsVersion = eeFsArm->version;
    eeFsSize = 8+4*62;
    eeFsBlockSize = 64;
    eeFsLinkSize = sizeof(int16_t);
    eeFsFirstBlock = 1;
    eeFsBlocksOffset = eeFsSize - eeFsBlockSize;
    eeFsBlocksMax = 1 + (Boards::getEEpromSize(board)-eeFsSize) / eeFsBlockSize;
    return eeFsArm->mySize == eeFsSize;
  }
}

void RleFile::eeprom_read_block (void *pointer_ram, unsigned int pointer_eeprom, size_t size)
{
  memcpy(pointer_ram, &eeprom[pointer_eeprom], size);
}

void RleFile::eeprom_write_block(const void *pointer_ram, unsigned int pointer_eeprom, size_t size)
{
  memcpy(&eeprom[pointer_eeprom], pointer_ram, size);
}

uint8_t RleFile::EeFsRead(unsigned int blk, unsigned int ofs)
{
  uint8_t ret;
  eeprom_read_block(&ret, blk*eeFsBlockSize+ofs+eeFsBlocksOffset, 1);
  return ret;
}

void RleFile::EeFsWrite(unsigned int blk, unsigned int ofs, uint8_t val)
{
  eeprom_write_block(&val, blk*eeFsBlockSize+ofs+eeFsBlocksOffset, 1);
}

unsigned int RleFile::EeFsGetLink(unsigned int blk)
{
  int16_t ret;
  eeprom_read_block((uint8_t *)&ret, blk*eeFsBlockSize+eeFsBlocksOffset, eeFsLinkSize);
  return ret;
}

void RleFile::EeFsSetLink(unsigned int blk, unsigned int val)
{
  int16_t s_link = val;
  eeprom_write_block((uint8_t *)&s_link, (blk*eeFsBlockSize)+eeFsBlocksOffset, eeFsLinkSize);

}

uint8_t RleFile::EeFsGetDat(unsigned int blk, unsigned int ofs)
{
  return EeFsRead(blk, ofs+eeFsLinkSize);
}

void RleFile::EeFsSetDat(unsigned int blk, unsigned int ofs, const uint8_t *buf, unsigned int len)
{
  eeprom_write_block(buf, blk*eeFsBlockSize+ofs+eeFsLinkSize+eeFsBlocksOffset, len);
}

unsigned int RleFile::EeFsGetFree()
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;

  unsigned int ret = 0;
  unsigned int i;
  i = eeFsArm->freeList;

  while (i) {
    ret += eeFsBlockSize-eeFsLinkSize;
    i = EeFsGetLink(i);
  }
  return ret;
}

/*
 * free one or more blocks
 */
void RleFile::EeFsFree(unsigned int blk)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return;

  unsigned int i = blk;
  while (EeFsGetLink(i)) i = EeFsGetLink(i);
  EeFsSetLink(i, eeFsArm->freeList);
  eeFsArm->freeList = blk; //chain in front
}

/*
 * alloc one block from freelist
 */
unsigned int RleFile::EeFsAlloc()
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;
  unsigned int ret = eeFsArm->freeList;
  if (ret) {
    eeFsArm->freeList = EeFsGetLink(ret);
    EeFsSetLink(ret, 0);
  }
  return ret;
}

unsigned int RleFile::size(unsigned int id)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;
  else
    return eeFsArm->files[id].size;
}

unsigned int RleFile::openRd(unsigned int i_fileId)
{
  if (IS_FAMILY_HORUS_OR_T16(board)) {
    return 1;
  }
  else if (IS_SKY9X(board)) {
    if (eepromFatHeader) {
      m_fileId = eepromFatHeader->files[i_fileId].zoneIndex;
      if (eepromFatHeader->files[i_fileId].exists) {
        eeprom_read_block(&m_size, m_fileId*(1<<13)+sizeof(uint16_t), sizeof(uint16_t));
      }
      else {
        m_size = 0;
      }
      m_pos = sizeof(EepromFileHeader);
    }
    else {
      m_fileId = get_current_block_number(i_fileId * 2, &m_size);
      m_pos = sizeof(t_eeprom_header);
    }
    return 1;
  }
  else {
    m_fileId   = i_fileId;
    m_pos      = 0;
    m_currBlk  = eeFsArm->files[m_fileId].startBlk;
    m_ofs      = 0;
    m_zeroes   = 0;
    m_bRlc     = 0;
    m_err      = ERR_NONE;       //error reasons
    return eeFsArm->files[m_fileId].typ;
  }
}

unsigned int RleFile::read(uint8_t *buf, unsigned int i_len)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;

  unsigned int len = eeFsArm->files[m_fileId].size;
  len -= m_pos;
  if (i_len > len) i_len = len;
  len = i_len;
  while(len) {
    if (!m_currBlk) break;
    *buf++ = EeFsGetDat(m_currBlk, m_ofs++);
    if (m_ofs >= (eeFsBlockSize - 2)) {
      m_ofs = 0;
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    len--;
  }
  m_pos += i_len - len;
  return i_len - len;
}

// G: Read runlength (RLE) compressed bytes into buf.
unsigned int RleFile::readRlc12(uint8_t *buf, unsigned int i_len, bool rlc2)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;

  memset(buf, 0, i_len);

  if (IS_SKY9X(board)) {
    int len;
    if (eepromFatHeader) {
      len = std::min((int)i_len, (int)m_size + (int)sizeof(EepromFileHeader) - (int)m_pos);
      eeprom_read_block(buf, (m_fileId << 13) + m_pos, len);
      m_pos += len;
    }
    else {
      len = std::min((int)i_len, (int)m_size + (int)sizeof(t_eeprom_header) - (int)m_pos);
      if (len > 0) {
        eeprom_read_block(buf, (m_fileId << 12) + m_pos, len);
        m_pos += len;
      }
    }
    return len;
  }
  else {
    unsigned int i=0;
    for( ; 1; ) {
      uint8_t ln = std::min<uint16_t>(m_zeroes, i_len-i);
      memset(&buf[i], 0, ln);
      i        += ln;
      m_zeroes -= ln;
      if(m_zeroes) break;

      ln = std::min<uint16_t>(m_bRlc, i_len-i);
      uint8_t lr = read(&buf[i], ln);
      i        += lr ;
      m_bRlc   -= lr;
      if(m_bRlc) break;

      if (read(&m_bRlc, 1) !=1) break; //read how many bytes to read

      if (!(m_bRlc & 0x7f)) {
        qDebug() << "RLC decoding error!";
        return 0;
      }

      if (rlc2) {
        if(m_bRlc&0x80){ // if contains high byte
          m_zeroes  =(m_bRlc>>4) & 0x7;
          m_bRlc    = m_bRlc & 0x0f;
        }
        else if(m_bRlc&0x40){
          m_zeroes  = m_bRlc & 0x3f;
          m_bRlc    = 0;
        }
        //else   m_bRlc
      }
      else {
        if(m_bRlc&0x80){ // if contains high byte
          m_zeroes  = m_bRlc & 0x7f;
          m_bRlc    = 0;
        }
      }
    }
    return i;
  }
}

unsigned int importRlc(QByteArray & dst, QByteArray & src, unsigned int rlcVersion)
{
  uint8_t *buf = (uint8_t *)src.data();
  unsigned int len = src.size();
  uint8_t zeroes = 0;
  uint8_t bRlc = 0;

  dst.resize(0);

  for( ; 1; ) {
    if (zeroes > 0) {
      for (int i = 0; i < zeroes; i++)
        dst.append((char) 0);
      zeroes = 0;
    }

    if (len == 0)
      return dst.size();

    for (int i=0; i<bRlc; i++) {
      dst.append(*buf++);
      if (--len == 0)
        return dst.size();
    }

    bRlc = *buf++;
    --len;

    if (!(bRlc & 0x7f)) {
      qDebug() << "RLC decoding error!";
      return 0;
    }

    if (rlcVersion == 2) {
      if (bRlc&0x80){ // if contains high byte
        zeroes  = (bRlc>>4) & 0x07;
        bRlc    = bRlc & 0x0f;
      }
      else if (bRlc&0x40){
        zeroes = bRlc & 0x3f;
        bRlc   = 0;
      }
    }
    else {
      if (bRlc&0x80){ // if contains high byte
        zeroes = bRlc & 0x7f;
        bRlc   = 0;
      }
    }
  }
  return dst.size();
}

unsigned int RleFile::write1(uint8_t b)
{
  return write(&b, 1);
}

unsigned int RleFile::write(const uint8_t *buf, unsigned int i_len)
{
  unsigned int len = i_len;
  if (!m_currBlk && m_pos==0)
  {
    eeFsArm->files[m_fileId].startBlk = m_currBlk = EeFsAlloc();
  }
  while (len)
  {
    if (!m_currBlk) {
      m_err = ERR_FULL;
      return 0;
    }
    if (m_ofs >= eeFsBlockSize-eeFsLinkSize) {
      m_ofs = 0;
      if (!EeFsGetLink(m_currBlk) ){
        EeFsSetLink(m_currBlk, EeFsAlloc());
      }
      m_currBlk = EeFsGetLink(m_currBlk);
    }
    if (!m_currBlk) {
      m_err = ERR_FULL;
      return 0;
    }
    uint8_t l = eeFsBlockSize-eeFsLinkSize-m_ofs; if(l>len) l=len;
    EeFsSetDat(m_currBlk, m_ofs, buf, l);
    buf   +=l;
    m_ofs +=l;
    len   -=l;
  }
  m_pos += i_len - len;
  return i_len - len;
}

void RleFile::create(unsigned int i_fileId, unsigned int typ)
{
  openRd(i_fileId); //internal use
  eeFsArm->files[i_fileId].typ   = typ;
  eeFsArm->files[i_fileId].size  = 0;

}

void RleFile::closeTrunc()
{
  unsigned int fri=0;
  eeFsArm->files[m_fileId].size = m_pos;
  if (m_currBlk && ( fri = EeFsGetLink(m_currBlk))) EeFsSetLink(m_currBlk, 0);
  if(fri) EeFsFree( fri );  //chain in
}

unsigned int RleFile::writeRlc1(unsigned int i_fileId, unsigned int typ, const uint8_t *buf, unsigned int i_len)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;

  create(i_fileId, typ);
  bool state0 = true;
  uint8_t cnt = 0;
  uint16_t i;

  //RLE compression:
  //rb = read byte
  //if (rb | 0x80) write rb & 0x7F zeros
  //else write rb bytes
  for (i=0; i<=i_len; i++)
  {
    bool nst0 = (buf[i] == 0);
    if (nst0 && !state0 && buf[i+1]!=0) nst0 = false ;
    if (nst0 != state0 || cnt>=0x7f || i==i_len) {
      if (state0) {
        if(cnt>0) {
          cnt |= 0x80;
          if (write(&cnt,1)!=1)           goto error;
          cnt=0;
        }
      }
      else {
        if (cnt>0) {
          if (write(&cnt,1) !=1)            goto error;
          uint8_t ret=write(&buf[i-cnt],cnt);
          if (ret!=cnt) { cnt-=ret;        goto error;}
          cnt=0;
        }
      }
      state0 = nst0;
    }
    cnt++;
  }
  if(0){
    error:
    i_len = i - (cnt & 0x7f);
  }
  closeTrunc();
  return i_len;
}

/*
 * Write runlength (RLE) compressed bytes
 */
unsigned int RleFile::writeRlc2(unsigned int i_fileId, unsigned int typ, const uint8_t *buf, unsigned int i_len)
{
  if (IS_FAMILY_HORUS_OR_T16(board))
    return 0;

  if (IS_SKY9X(board)) {
    openRd(i_fileId);
    if (eepromFatHeader) {
      eepromFatHeader->files[i_fileId].exists = 1;
      eeprom_write_block(buf, (m_fileId << 13) + m_pos, i_len);
      {
        EepromFileHeader header;
        header.fileIndex = i_fileId;
        header.size = i_len;
        eeprom_write_block(&header, (m_fileId << 13), sizeof(header));
      }
    }
    else {
      eeprom_write_block(buf, (m_fileId << 12) + m_pos, i_len);
      t_eeprom_header header;
      header.sequence_no = 1;
      header.data_size = i_len;
      header.flags = 0;
      header.hcsum = byte_checksum((uint8_t *) &header, 7);
      eeprom_write_block(&header, (m_fileId << 12), sizeof(header));
    }
    return i_len;
  }
  else {
    create(i_fileId, typ);
    bool    run0   = buf[0] == 0;
    uint8_t cnt    = 1;
    uint8_t cnt0   = 0;
    uint16_t i     = 0;
    if (i_len==0) goto close;

    //RLE compression:
    //rb = read byte
    //if (rb | 0x80) write rb & 0x7F zeros
    //else write rb bytes
    for (i=1; 1; i++) { // !! laeuft ein byte zu weit !!
      bool cur0 = buf[i] == 0;
      if (i==i_len || cur0 != run0 || cnt==0x3f || (cnt0 && cnt==0xf)) {
        if (run0){
          assert(cnt0==0);
          if (cnt<8 && i!=i_len)
            cnt0 = cnt; //aufbew fuer spaeter
          else {
            if (write1(cnt|0x40)!=1)                goto error;//-cnt&0x3f
          }
        }
        else {
          if (cnt0) {
            if (write1(0x80 | (cnt0<<4) | cnt)!=1)  goto error;//-cnt0xx-cnt
            cnt0 = 0;
          }
          else {
            if (write1(cnt)!=1)                    goto error;//-cnt
          }
          uint8_t ret = write(&buf[i-cnt], cnt);
          if (ret != cnt) { cnt-=ret;                goto error;}//-cnt
        }
        cnt=0;
        if (i==i_len) break;
        run0 = cur0;
      }
      cnt++;
    }
    if (0) {
      error:
      i-=cnt+cnt0;
    }

    close:
    closeTrunc();
    return i;
  }
}

uint8_t RleFile::byte_checksum( uint8_t *p, unsigned int size )
{
        uint32_t csum ;

        csum = 0 ;
        while( size )
        {
                csum += *p++ ;
                size -= 1 ;
        }
        return csum ;
}

uint32_t RleFile::ee32_check_header( struct t_eeprom_header *hptr )
{
        uint8_t csum ;

        csum = byte_checksum( ( uint8_t *) hptr, 7 ) ;
        if ( csum == hptr->hcsum )
        {
                return 1 ;
        }
        return 0 ;
}

// Pass in an even block number, this and the next block will be checked
// to see which is the most recent, the block_no of the most recent
// is returned, with the corresponding data size if required
// and the sequence number if required
uint32_t RleFile::get_current_block_number( uint32_t block_no, uint16_t *p_size)
{
  struct t_eeprom_header b0 ;
  struct t_eeprom_header b1 ;
//  uint32_t sequence_no ;
  uint16_t size ;

  eeprom_read_block( ( uint8_t *)&b0, block_no << 12, sizeof(b0) ) ;          // Sequence # 0
  eeprom_read_block( ( uint8_t *)&b1, (block_no+1) << 12, sizeof(b1) ) ;      // Sequence # 1

  if ( ee32_check_header( &b0 ) == 0 )
  {
    b0.sequence_no = 0 ;
    b0.data_size = 0 ;
    b0.flags = 0 ;
  }

  size = b0.data_size ;
  // sequence_no = b0.sequence_no ;
  if ( ee32_check_header( &b0 ) == 0 )
  {
    if ( ee32_check_header( &b1 ) != 0 )
    {
      size = b1.data_size ;
      // sequence_no = b1.sequence_no ;
      block_no += 1 ;
    }
    else
    {
      size = 0 ;
      // sequence_no = 1 ;
    }
  }
  else
  {
    if ( ee32_check_header( &b1 ) != 0 )
    {
      if ( b1.sequence_no > b0.sequence_no )
      {
        size = b1.data_size ;
        // sequence_no = b1.sequence_no ;
        block_no += 1 ;
      }
    }
  }

  if ( size == 0xFFFF )
  {
    size = 0 ;
  }
  if ( p_size )
  {
    *p_size = size ;
  }

  return block_no ;
}
