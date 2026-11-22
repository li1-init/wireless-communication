/*! @file radio.h
 * @brief This file is contains the public radio interface functions.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef RADIO_H_
#define RADIO_H_

/*****************************************************************************
 *  Global Macros & Definitions
 *****************************************************************************/
	/*! Maximal packet length definition (FIFO size) */
#define RADIO_MAX_PACKET_LENGTH     64u

/*****************************************************************************
 *  Global Typedefs & Enums
 *****************************************************************************/
typedef struct
{
    uint8_t   const *Radio_ConfigurationArray;

    uint8_t   Radio_ChannelNumber;
    uint8_t   Radio_PacketLength;
    uint8_t   Radio_State_After_Power_Up;

    uint16_t  Radio_Delay_Cnt_After_Reset;
		uint8_t   Radio_CustomPayload[RADIO_MAX_PACKET_LENGTH];
} tRadioConfiguration;

/*****************************************************************************
 *  Global Variable Declarations
 *****************************************************************************/
//extern const SEGMENT_VARIABLE_SEGMENT_POINTER(pRadioConfiguration, tRadioConfiguration, SEG_CODE, SEG_CODE);

extern const tRadioConfiguration *pRadioConfiguration;

/*! Si446x configuration array */
extern const uint8_t Radio_Configuration_Data_Array[];

/*****************************************************************************
 *  Global Function Declarations
 *****************************************************************************/
void  vRadio_Init(void);
uint8_t gRadio_CheckTransmitted(void);
void vRadio_StartTx(uint8_t length, uint8_t *pTxPacket, uint8_t channel_number);
void vRadio_StartRX_ConstOn(uint8_t length, uint8_t channel_number);
void vRadio_StartCW_Test(uint8_t power, uint8_t channel_number);
void vRadio_BER_StartRX(void);





#endif /* RADIO_H_ */
