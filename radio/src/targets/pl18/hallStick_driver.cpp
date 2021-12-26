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

DMAFifo<HALLSTICK_BUFF_SIZE> hallDMAFifo __DMA (HALL_DMA_Stream_RX);
Fifo<uint8_t, HALLSTICK_BUFF_SIZE> hallStickTxFifo;
static uint8_t hallStickSendState = HALLSTICK_SEND_STATE_IDLE;
unsigned char HallCmd[264] __DMA;

STRUCT_HALL HallProtocol = { 0 };
STRUCT_HALL HallProtocolTx = { 0 };
signed short hall_raw_values[FLYSKY_HALL_CHANNEL_COUNT];
STRUCT_STICK_CALIBRATION hall_calibration[FLYSKY_HALL_CHANNEL_COUNT] = { {0, 0, 0} };
unsigned short hall_adc_values[FLYSKY_HALL_CHANNEL_COUNT];

/* crc16 implementation according to CCITT standards */
const unsigned short CRC16Table[256]= {
 0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
 0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
 0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
 0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
 0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
 0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
 0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
 0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
 0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
 0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
 0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
 0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
 0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
 0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
 0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
 0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
 0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
 0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
 0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
 0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
 0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
 0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
 0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
 0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
 0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
 0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
 0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
 0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
 0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
 0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
 0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
 0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

//const uint8_t sticks_mapping[4] = { 0 /*STICK1*/,  1/*STICK2*/, 2/*STICK3*/, 3 /*STICK4*/};

unsigned short  calc_crc16(void *pBuffer,unsigned char BufferSize)
{
    unsigned short crc16;
    crc16 = 0xffff;
    while (BufferSize)
    {
        crc16 = (crc16 << 8) ^ CRC16Table[((crc16>>8) ^ (*(unsigned char *)pBuffer)) & 0x00ff];
        pBuffer = (void *)((unsigned char *)pBuffer + 1);
        BufferSize--;
    }
    return crc16;
}

uint16_t get_hall_adc_value(uint8_t ch)
{
  if (ch >= FLYSKY_HALL_CHANNEL_COUNT)
  {
    return 0;
  }

  if (ch < 2)
  {
    return MAX_ADC_CHANNEL_VALUE - hall_adc_values[ch];
  }

  return hall_adc_values[ch];
}


void hall_stick_init(uint32_t baudrate)
{
  if (baudrate == 0)
  {
    USART_DeInit(HALL_SERIAL_USART);
    return;
  }

  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = HALL_SERIAL_RX_DMA_Stream_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(HALL_SERIAL_GPIO, HALL_SERIAL_RX_GPIO_PinSource, HALL_SERIAL_GPIO_AF);
  GPIO_PinAFConfig(HALL_SERIAL_GPIO, HALL_SERIAL_TX_GPIO_PinSource, HALL_SERIAL_GPIO_AF);

  GPIO_InitStructure.GPIO_Pin = HALL_SERIAL_TX_GPIO_PIN | HALL_SERIAL_RX_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(HALL_SERIAL_GPIO, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(HALL_SERIAL_USART, &USART_InitStructure);

  DMA_Cmd(HALL_DMA_Stream_RX, DISABLE);
  USART_DMACmd(HALL_SERIAL_USART, USART_DMAReq_Rx, DISABLE);
  DMA_DeInit(HALL_DMA_Stream_RX);

  DMA_InitTypeDef DMA_InitStructure;
  hallDMAFifo.clear();

  USART_ITConfig(HALL_SERIAL_USART, USART_IT_RXNE, DISABLE);
  USART_ITConfig(HALL_SERIAL_USART, USART_IT_TXE, DISABLE);

  DMA_InitStructure.DMA_Channel = HALL_DMA_Channel;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&HALL_SERIAL_USART->DR);
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(hallDMAFifo.buffer());
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = hallDMAFifo.size();
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(HALL_DMA_Stream_RX, &DMA_InitStructure);
  USART_DMACmd(HALL_SERIAL_USART, USART_DMAReq_Rx, ENABLE);
  USART_Cmd(HALL_SERIAL_USART, ENABLE);
  DMA_Cmd(HALL_DMA_Stream_RX, ENABLE);

  reset_hall_stick();
}

void HallSendBuffer(uint8_t * buffer, uint32_t count)
{
  for(uint32_t idx = 0; buffer != HallCmd && idx < count; idx++)
  {
    HallCmd[idx] = buffer[idx];
  }
  DMA_InitTypeDef DMA_InitStructure;
  DMA_DeInit(HALL_DMA_Stream_TX);
  DMA_InitStructure.DMA_Channel = HALL_DMA_Channel;
  DMA_InitStructure.DMA_PeripheralBaseAddr = CONVERT_PTR_UINT(&HALL_SERIAL_USART->DR);
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_Memory0BaseAddr = CONVERT_PTR_UINT(HallCmd);
  DMA_InitStructure.DMA_BufferSize = count;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(HALL_DMA_Stream_TX, &DMA_InitStructure);
  DMA_Cmd(HALL_DMA_Stream_TX, ENABLE);
  USART_DMACmd(HALL_SERIAL_USART, USART_DMAReq_Tx, ENABLE);
  DMA_ITConfig(HALL_DMA_Stream_TX, DMA_IT_TC, ENABLE);

  /* enable interrupt and set it's priority */
  NVIC_EnableIRQ(HALL_SERIAL_TX_DMA_Stream_IRQn);
  NVIC_SetPriority(HALL_SERIAL_TX_DMA_Stream_IRQn, 7);
}

extern "C" void HALL_TX_DMA_Stream_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_TELEM_DMA);

  if (DMA_GetITStatus(HALL_DMA_Stream_TX, HALL_DMA_TX_FLAG_TC))
  {
    DMA_ClearITPendingBit(HALL_DMA_Stream_TX, HALL_DMA_TX_FLAG_TC);
    HALL_SERIAL_USART->CR1 |= USART_CR1_TCIE;
  }
}

uint8_t HallGetByte(uint8_t * byte)
{
    return hallDMAFifo.pop(*byte);
}


void reset_hall_stick( void )
{
    unsigned short crc16 = 0xffff;

    HallCmd[0] = HALL_PROTOLO_HEAD;
    HallCmd[1] = 0xD1;
    HallCmd[2] = 0x01;
    HallCmd[3] = 0x01;

    crc16 = calc_crc16(HallCmd, 4);

    HallCmd[4] = crc16 & 0xff;
    HallCmd[5] = crc16 >>8 & 0xff;

    HallSendBuffer( HallCmd, 6);
}

void get_hall_config( void )
{
    unsigned short crc16 = 0xffff;

    HallCmd[0] = HALL_PROTOLO_HEAD;
    HallCmd[1] = 0xD1;
    HallCmd[2] = 0x01;
    HallCmd[3] = 0x00;

    crc16 = calc_crc16(HallCmd, 4); // 2B 2C

    HallCmd[4] = crc16 & 0xff;
    HallCmd[5] = crc16 >>8 & 0xff ;

    HallSendBuffer( HallCmd, 6);
}

void get_hall_firmware_info()
{
    unsigned short crc16 = 0xffff;

    HallCmd[0] = HALL_PROTOLO_HEAD;
    HallCmd[1] = 0xA2;
    HallCmd[2] = 0x00;

    crc16 = calc_crc16(HallCmd, 3); // BE 02

    HallCmd[3] = crc16 & 0xff;
    HallCmd[4] = crc16 >>8 & 0xff ;

    HallSendBuffer( HallCmd, 5);
}

void hallStickUpdatefwEnd( void )
{
    unsigned short crc16 = 0xffff;

    HallCmd[0] = HALL_PROTOLO_HEAD;
    HallCmd[1] = 0xA2;
    HallCmd[2] = 0x01;
    HallCmd[3] = 0x07;

    crc16 = calc_crc16(HallCmd, 4);

    HallCmd[4] = crc16 & 0xff;
    HallCmd[5] = crc16 >>8 & 0xff ;

    HallSendBuffer( HallCmd, 6);// 94 DD
}

static int parse_ps_state = 0;
void Parse_Character(STRUCT_HALL *hallBuffer, unsigned char ch)
{
    if (parse_ps_state != 0) return;
    parse_ps_state = 1;

    switch( hallBuffer->status )
    {
        case GET_START:
        {
            if ( HALL_PROTOLO_HEAD == ch )
            {
                hallBuffer->head  = HALL_PROTOLO_HEAD;
                hallBuffer->status = GET_ID;
                hallBuffer->msg_OK = 0;
            }
            break;
        }
        case GET_ID:
        {
            hallBuffer->hallID.ID = ch;
            hallBuffer->status = GET_LENGTH;
            break;
        }
        case GET_LENGTH:
        {
            hallBuffer->length = ch;
            hallBuffer->dataIndex = 0;
            hallBuffer->status = GET_DATA;
            if( 0 == hallBuffer->length )
            {
                hallBuffer->status = GET_CHECKSUM;
                hallBuffer->checkSum=0;
            }
            break;
        }
        case GET_DATA:
        {
            hallBuffer->data[hallBuffer->dataIndex++] = ch;
            if( hallBuffer->dataIndex >= hallBuffer->length)
            {
                hallBuffer->checkSum = 0;
                hallBuffer->dataIndex = 0;
                hallBuffer->status = GET_STATE;
            }
            break;
        }
        case GET_STATE:
        {
            hallBuffer->checkSum = 0;
            hallBuffer->dataIndex = 0;
            hallBuffer->status = GET_CHECKSUM;
        }
        case GET_CHECKSUM:
        {
            hallBuffer->checkSum |= ch << ((hallBuffer->dataIndex++) * 8);
            if( hallBuffer->dataIndex >= 2 )
            {
                hallBuffer->dataIndex = 0;
                hallBuffer->status = CHECKSUM;
            }
            else
            {
                break;
            }
        }
        case CHECKSUM:
        {
            if(hallBuffer->checkSum == calc_crc16( (U8*)&hallBuffer->head, hallBuffer->length + 3 ) )
            {
                hallBuffer->msg_OK = 1;
                goto Label_restart;
            }
            else
            {
                goto Label_error;
            }
        }
    }

    goto exit;

    Label_error:
    Label_restart:
        hallBuffer->status = GET_START;
exit: parse_ps_state = 0;
    return ;
}

#define ERROR_OFFSET      10
void convert_hall_to_adcVaule( void )
{
    uint16_t value;

    for ( uint8_t channel = 0; channel < 4; channel++ )
    {
        if (hall_raw_values[channel] < hall_calibration[channel].mid)
        {
            value = hall_calibration[channel].mid - (hall_calibration[channel].min+ERROR_OFFSET);
            value = ( MIDDLE_ADC_CHANNLE_VALUE * (hall_calibration[channel].mid - hall_raw_values[channel] ) ) / ( value );

            if (value >= MIDDLE_ADC_CHANNLE_VALUE ) {
                value = MIDDLE_ADC_CHANNLE_VALUE;
            }

            hall_adc_values[channel] = MIDDLE_ADC_CHANNLE_VALUE - value;
        }
        else
        {
            value = (hall_calibration[channel].max - ERROR_OFFSET) - hall_calibration[channel].mid;

            value = ( MIDDLE_ADC_CHANNLE_VALUE * (hall_raw_values[channel] - hall_calibration[channel].mid ) ) / (value );

            if (value >= MIDDLE_ADC_CHANNLE_VALUE )
            {
                value = MIDDLE_ADC_CHANNLE_VALUE;
            }

            hall_adc_values[channel] = MIDDLE_ADC_CHANNLE_VALUE + value + 1;
        }
    }
}

uint8_t HallGetByteTx(uint8_t * byte)
{
    return hallStickTxFifo.pop(*byte);
}

bool isHallStickUpdateFirmware( void )
{
    return hallStickSendState == HALLSTICK_STATE_UPDATE_FW;
}

void hallstick_send_by_state( void )
{
    switch ( hallStickSendState )
    {
    case HALLSTICK_STATE_SEND_RESET:
        TRACE("HALLSTICK_STATE_SEND_RESET");
        reset_hall_stick();
        hallStickSendState = HALLSTICK_STATE_GET_FIRMWARE;
        break;

    case HALLSTICK_STATE_GET_FIRMWARE:
        TRACE("HALLSTICK_STATE_GET_FIRMWARE");
        get_hall_firmware_info();
        hallStickSendState = HALLSTICK_STATE_UPDATE_FW;
        break;

    case HALLSTICK_STATE_UPDATE_FW:
        return;

    default: break;
    }
}

void hallstick_wait_send_done(uint32_t timeOut)
{
    static unsigned int startTime = get_tmr10ms();

    while ( hallStickSendState != HALLSTICK_SEND_STATE_IDLE )
    {
        if ( (get_tmr10ms() - startTime) < timeOut )
        {
            break;
        }
    }
}

static uint32_t HallProtocolCount = 0;
bool isHallProtocolTxMsgOK( void )
{
    bool isMsgOK = HallProtocolCount != 0;
    HallProtocolCount = 0;
    return isMsgOK;
}

/* HallStick send main program */
void hallStick_GetTxDataFromUSB( void )
{
    unsigned char abyte;
    uint8_t *pt = (uint8_t *)&HallProtocolTx;

    while( HallGetByteTx(&abyte) )
    {
        Parse_Character(&HallProtocolTx, abyte );

        if ( HallProtocolTx.msg_OK )
        {
            HallProtocolTx.msg_OK = 0;

            pt[HallProtocolTx.length + 3] = HallProtocolTx.checkSum & 0xFF;
            pt[HallProtocolTx.length + 4] = HallProtocolTx.checkSum >> 8;

            //TRACE("USB: %02X %02X %02X ...%04X; CRC:%04X", pt[0], pt[1], pt[2],
            //      HallProtocolTx.checkSum, calc_crc16(pt, HallProtocolTx.length+3));

            switch ( HallProtocolTx.hallID.hall_Id.receiverID )
            {
            case TRANSFER_DIR_TXMCU:
                break;

            case TRANSFER_DIR_HALLSTICK:
                onFlySkyUsbDownloadStart(TRANSFER_DIR_HALLSTICK);

                if ( 0xA2 == HallProtocolTx.hallID.ID )
                {
                    if ( 0 == HallProtocolTx.length ) // 55 A2 00 BE 02
                    {
                        hallStickSendState = HALLSTICK_STATE_SEND_RESET;
                        break;
                    }

                    else if ( 0x01 == HallProtocolTx.length && 0x07 == HallProtocol.data[0] )
                    {
                        hallStickSendState = HALLSTICK_SEND_STATE_IDLE;
                    }
                }
                HallSendBuffer( pt, HallProtocolTx.length + 3 + 2 );
                break;

            case TRANSFER_DIR_RFMODULE:
                onFlySkyUsbDownloadStart(TRANSFER_DIR_RFMODULE);

                if ( 0xAE == HallProtocolTx.hallID.ID && HallProtocolTx.length == 0 )
                {
                    setFlyskyState(INTERNAL_MODULE, FLYSKY_MODULE_STATE_UPDATE_RF_FIRMWARE);
                    break;
                }

                if ( 0x0D == HallProtocolTx.hallID.hall_Id.packetID && HallProtocolTx.data[0] == 1 )
                {
                    onFlySkyGetVersionInfoStart(INTERNAL_MODULE, 1);
                    break;
                }

                //if ( isFlySkyUsbDownload() )
                {
                    intmoduleSendBuffer( pt, HallProtocolTx.length + 3 + 2 );
                }
                break;
            }
        }
    }

    if ( !usbPlugged() )
    {
        onFlySkyUsbDownloadStart(0);
    }
}


/* Run it in 1ms timer routine */
void hall_stick_loop(void)
{
    static uint8_t count = 0;
    static tmr10ms_t lastConfigTime = get_tmr10ms();
    bool log = 0;

    hallStick_GetTxDataFromUSB();

    if(count>10)
    {
        count = 0;
        hallstick_send_by_state();
    }
    count++;
    uint8_t byte;
    while(HallGetByte(&byte))
    {
        HallProtocol.index++;

        Parse_Character(&HallProtocol, byte);

        if ( HallProtocol.msg_OK )
        {
            HallProtocol.msg_OK = 0;
            HallProtocol.stickState = HallProtocol.data[HallProtocol.length - 1];

            switch ( HallProtocol.hallID.hall_Id.receiverID )
            {
            case TRANSFER_DIR_TXMCU:
                if(HallProtocol.hallID.hall_Id.packetID == HALL_RESP_TYPE_CALIB) {
                  memcpy(&hall_calibration, HallProtocol.data, sizeof(hall_calibration));
                }
                else if(HallProtocol.hallID.hall_Id.packetID == HALL_RESP_TYPE_VALUES) {
                  memcpy(hall_raw_values, HallProtocol.data, sizeof(hall_raw_values));
                  convert_hall_to_adcVaule();
                }
                break;
            case TRANSFER_DIR_HOSTPC:
                if (HallProtocol.length == 0x01 && (HallProtocol.data[0] == 0x05 || HallProtocol.data[0] == 0x06) )
                {
                    hallStickSendState = HALLSTICK_SEND_STATE_IDLE;
                }
            case TRANSFER_DIR_HALLSTICK:
                HallProtocolCount++;
                uint8_t *pt = (uint8_t*)&HallProtocol;
                //HallProtocol.head = HALL_PROTOLO_HEAD;
                //TRACE("HALL: %02X %02X %02X ...%04X", pt[0], pt[1], pt[2], HallProtocol.checkSum);
                pt[HallProtocol.length + 3] = HallProtocol.checkSum & 0xFF;
                pt[HallProtocol.length + 4] = HallProtocol.checkSum >> 8;
                usbDownloadTransmit( pt, HallProtocol.length + 5 );
                break;
            }
        }
    }
    //check periodically  if calibration is correct
    if (get_tmr10ms() - lastConfigTime > 200)
    {
        //invalid calibration
        if(hall_calibration[0].max - hall_calibration[0].min < 1024) {
          TRACE("GET HALL CONFIG");
          get_hall_config();
          lastConfigTime = get_tmr10ms();
        }

        if (log)
        {
            TRACE_NOCRLF("Hall(%0d):", FLYSKY_HALL_BAUDRATE);
            for (int idx = 0; idx < HallProtocol.length + 5; idx++)
            {
                TRACE_NOCRLF(" %02X", *((uint8_t*)&HallProtocol + idx));
            }
            TRACE(";");
        }
    }
}
