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

//------------------------------------------------------------------------------
/// \unit
/// !Purpose
/// 
/// Generic Media type, which provides transparent access to all types of
/// memories.
///
/// \note The physical or HW related media operations (physical device
///       connection & protection detecting, PIO configurations and interface
///       driver initialization) are excluded.
/// 
/// !Usage
/// -# Do PIO initialization for peripheral interfaces.
/// -# Initialize peripheral interface driver & device driver.
/// -# Initialize specific media interface and link to this initialized driver.
///
//------------------------------------------------------------------------------

#ifndef _MEDIA_H_
#define _MEDIA_H_

//------------------------------------------------------------------------------
//      Definitions
//------------------------------------------------------------------------------

//! \brief  Operation result code returned by media methods
#define MED_STATUS_SUCCESS      0x00
#define MED_STATUS_ERROR        0x01
#define MED_STATUS_BUSY         0x02
#define MED_STATUS_PROTECTED    0x04

//! \brief Media statuses
#define MED_STATE_READY         0x00    /// Media is ready for access
#define MED_STATE_BUSY          0x01    /// Media is busy

//------------------------------------------------------------------------------
//      Types
//------------------------------------------------------------------------------
typedef struct _Media Media;

typedef void (*MediaCallback)(void *argument,
                              unsigned char status,
                              unsigned int  transferred,
                              unsigned int  remaining);

typedef unsigned char (*Media_write)(Media *media,
                                     unsigned int address,
                                     void *data,
                                     unsigned int length,
                                     MediaCallback callback,
                                     void *argument);

typedef unsigned char (*Media_read)(Media *media,
                                    unsigned int address,
                                    void *data,
                                    unsigned int length,
                                    MediaCallback callback,
                                    void *argument);

typedef unsigned char (*Media_cancelIo)(Media *media);

typedef unsigned char (*Media_lock)(Media        *media,
                                    unsigned int start,
                                    unsigned int end,
                                    unsigned int *pActualStart,
                                    unsigned int *pActualEnd);

typedef unsigned char (*Media_unlock)(Media        *media,
                                      unsigned int start,
                                      unsigned int end,
                                      unsigned int *pActualStart,
                                      unsigned int *pActualEnd);

typedef unsigned char (*Media_ioctl)(Media *media,
                                     unsigned char ctrl,
                                     void *buff);

typedef unsigned char (*Media_flush)(Media *media);

typedef void (*Media_handler)(Media *media);

//! \brief  Media transfer
//! \see    TransferCallback
typedef struct {

    void            *data;      //!< Pointer to the data buffer
    unsigned int    address;    //!< Address where to read/write the data
    unsigned int    length;     //!< Size of the data to read/write
    MediaCallback   callback;   //!< Callback to invoke when the transfer done
    void            *argument;  //!< Callback argument

} MEDTransfer;

//! \brief  Media object
//! \see    MEDTransfer
struct _Media {

  Media_write    write;       //!< Write method
  Media_read     read;        //!< Read method
  Media_cancelIo cancelIo;    //!< Cancel pending IO method
  Media_lock     lock;        //!< lock method if possible
  Media_unlock   unlock;      //!< unlock method if possible
  Media_flush    flush;       //!< Flush method
  Media_handler  handler;     //!< Interrupt handler
  unsigned int   blockSize;   //!< Block size in bytes (1, 512, 1K, 2K ...)
  unsigned int   baseAddress; //!< Base address of media in number of blocks
  unsigned int   size;        //!< Size of media in number of blocks
  MEDTransfer    transfer;    //!< Current transfer operation
  void           *interface;  //!< Pointer to the physical interface used
  unsigned char  bReserved:4,
                 mappedRD:1,  //!< Mapped to memory space to read
                 mappedWR:1,  //!< Mapped to memory space to write
#ifdef __cplusplus
                 protectd:1, //!< Protected media?
#else
                 protected:1, //!< Protected media?
#endif
                 removable:1; //!< Removable/Fixed media?
  unsigned char  state;       //!< Status of media
  unsigned short reserved;
};

/// Available medias.
extern Media medias[];

/// Number of medias which are effectively used.
/// Defined by Media, shared usage by USB MSD & FS ...
extern unsigned int numMedias;

//------------------------------------------------------------------------------
//      Inline Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//! \brief  Writes data on a media
//! \param  media    Pointer to a Media instance
//! \param  address  Address at which to write
//! \param  data     Pointer to the data to write
//! \param  length   Size of the data buffer
//! \param  callback Optional pointer to a callback function to invoke when
//!                   the write operation terminates
//! \param  argument Optional argument for the callback function
//! \return Operation result code
//! \see    TransferCallback
//------------------------------------------------------------------------------
static inline unsigned char MED_Write(Media         *media,
                                      unsigned int  address,
                                      void          *data,                                      
                                      unsigned int  length,
                                      MediaCallback callback,
                                      void          *argument)
{
    return media->write(media, address, data, length, callback, argument);
}

//------------------------------------------------------------------------------
//! \brief  Reads a specified amount of data from a media
//! \param  media    Pointer to a Media instance
//! \param  address  Address of the data to read
//! \param  data     Pointer to the buffer in which to store the retrieved
//!                   data
//! \param  length   Length of the buffer
//! \param  callback Optional pointer to a callback function to invoke when
//!                   the operation is finished
//! \param  argument Optional pointer to an argument for the callback
//! \return Operation result code
//! \see    TransferCallback
//------------------------------------------------------------------------------
static inline unsigned char MED_Read(Media          *media,
                                     unsigned int   address,
                                     void           *data,                                     
                                     unsigned int   length,
                                     MediaCallback  callback,
                                     void           *argument)
{
    return media->read(media, address, data, length, callback, argument);
}

//------------------------------------------------------------------------------
//! \brief  Locks all the regions in the given address range.
//! \param  media    Pointer to a Media instance
/// \param  start  Start address of lock range.
/// \param  end  End address of lock range.
/// \param  pActualStart  Start address of the actual lock range (optional).
/// \param  pActualEnd  End address of the actual lock range (optional).
/// \return 0 if successful; otherwise returns an error code.
//------------------------------------------------------------------------------
static inline unsigned char MED_Lock(Media        *media,
                                     unsigned int start,
                                     unsigned int end,
                                     unsigned int *pActualStart,
                                     unsigned int *pActualEnd)
{
    if( media->lock ) {
        return media->lock(media, start, end, pActualStart, pActualEnd);
    }
    else {
        return MED_STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------------
//! \brief  Unlocks all the regions in the given address range
//! \param  media    Pointer to a Media instance
/// \param start  Start address of unlock range.
/// \param end  End address of unlock range.
/// \param pActualStart  Start address of the actual unlock range (optional).
/// \param pActualEnd  End address of the actual unlock range (optional).
/// \return 0 if successful; otherwise returns an error code.
//------------------------------------------------------------------------------
static inline unsigned char MED_Unlock(Media        *media,
                                       unsigned int start,
                                       unsigned int end,
                                       unsigned int *pActualStart,
                                       unsigned int *pActualEnd)
{
    if( media->unlock ) {
        return media->unlock(media, start, end, pActualStart, pActualEnd);
    }
    else {
        return MED_STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------------
//! \brief  
//! \param  media Pointer to the Media instance to use
//------------------------------------------------------------------------------
static inline unsigned char MED_Flush(Media *media)
{
    if (media->flush) {
    
        return media->flush(media);
    }
    else {

        return MED_STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------------
//! \brief  Invokes the interrupt handler of the specified media
//! \param  media Pointer to the Media instance to use
//------------------------------------------------------------------------------
static inline void MED_Handler(Media *media)
{
    if (media->handler) {
    
        media->handler(media);
    }
}

//------------------------------------------------------------------------------
//      Exported functions
//------------------------------------------------------------------------------

extern void MED_HandleAll(Media *medias, unsigned char numMedias);

#endif // _MEDIA_H_

