/*! @file radio.c
 * @brief This file contains functions to interface with the radio chip.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#include "bsp.h"
#include "app_protocol.h"


/*****************************************************************************
 *  Local Macros & Definitions
 *****************************************************************************/

/*****************************************************************************
 *  Global Variables
 *****************************************************************************/
const uint8_t Radio_Configuration_Data_Array[]=RADIO_CONFIGURATION_DATA_ARRAY;
const tRadioConfiguration RadioConfiguration=RADIO_CONFIGURATION_DATA;
const tRadioConfiguration *pRadioConfiguration=&RadioConfiguration;
volatile uint8_t fixRadioPacket[APP_RADIO_PACKET_SIZE];

/*****************************************************************************
 *  Local Function Declarations
 *****************************************************************************/
void vRadio_PowerUp(void);
/*!
 *  Power up the Radio.
 *
 *  @note
 *
 */
void vRadio_PowerUp(void)
{
	uint16_t wDelay=0;
  /* Hardware reset the chip */
  si446x_reset();

  /* Wait until reset timeout or Reset IT signal */
  for (; wDelay < pRadioConfiguration->Radio_Delay_Cnt_After_Reset; wDelay++);
}

/*!
 *  Radio Initialization.
 *
 *  @author Sz. Papp
 *
 *  @note
 *
 */
void vRadio_Init(void)
{
  uint16_t wDelay;

  /* Power Up the radio chip */
  vRadio_PowerUp();

  /* Load radio configuration */
  while (SI446X_SUCCESS != si446x_configuration_init(pRadioConfiguration->Radio_ConfigurationArray))
  {
    for (wDelay = 0x7FFF; wDelay--; ) ;
   /* Power Up the radio chip */
    vRadio_PowerUp();
  }

  // Read ITs, clear pending ones
  si446x_get_int_status(0u, 0u, 0u);
}

/*******************************************************************************************************************	
Function Name:			gRadio_CheckTransmitted(void)
Brief Comment:			Check transmitting state of one packege
Input Parameter:		None
Return Value:				transmit state@ 1= transmitting finished
Notes:							None
*******************************************************************************************************************/
uint8_t gRadio_CheckTransmitted(void)
{
  si446x_get_int_status(0u, 0u, 0u);															//读取中断状态
  if(Si446xCmd.GET_INT_STATUS.PH_PEND&
		 SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)	//等待发送完成，PACKET_SENT_PEND置位
  	{
      return 1;																															
    }
    return 0;
}
/*******************************************************************************************************************	
Function Name:			vRadio_StartTxvRadio_StartTx(uint8_t length, uint8_t *pPacket)
Brief Comment:			Start packet transmitting process
Input Parameter:		None
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void vRadio_StartTx(uint8_t length, uint8_t *pTxPacket,uint8_t channel_number)
{
	si446x_get_int_status(0u, 0u, 0u);												//清除中断

  si446x_fifo_info(SI446X_CMD_FIFO_INFO_ARG_FIFO_TX_BIT);		//复位TX FIFO
	si446x_write_tx_fifo(length, pTxPacket);									//写待发送数据包至TX FIFO

	si446x_pkt_f1_length_config(length);											//置Packet Filed1包长度

  si446x_start_tx(channel_number,0x30,00);									//启动发射,发送后待机
	while(!gRadio_CheckTransmitted());												//等待发送完成
	//while(SI_GPIO1_STATUS);		
}

/*******************************************************************************************************************	
Function Name:			vRadio_StartTxvRadio_StartTx(uint8_t length, uint8_t *pPacket)
Brief Comment:			Start packet transmitting process
Input Parameter:		None
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void vRadio_StartRX_ConstOn(uint8_t length,uint8_t channel_number)
{
	si446x_change_state(SI446X_STATE_RX);
	si446x_get_int_status(0u, 0u, 0u);
	
  si446x_fifo_info(SI446X_CMD_FIFO_INFO_ARG_FIFO_RX_BIT);
	si446x_pkt_f1_length_config(length);											//置Packet Filed1包长度

  si446x_start_rx(	channel_number,0,0,
										SI446X_STATE_RX,												//preamble超时
										SI446X_STATE_READY,											//包接收正常，CRC校验通过
										SI446X_STATE_RX													//CRC校验失败
																		);
}

/*******************************************************************************************************************	
Function Name:			vRadio_StartCW_Test(void)
Brief Comment:			Set Radio Chip into CW transmitting state
Input Parameter:		power@ setting of output power level
Return Value:				None
Notes:							MODEM_MOD_TYPE(0x2000)=0x00 sets radio into cw state
*******************************************************************************************************************/
void vRadio_StartCW_Test(uint8_t power, uint8_t channel_number)
{
	vRadio_Init();															//初始化芯片
	si446x_pa_pwr(power);												//设置芯片发发射功率，0x7F=20dBm
	
	si446x_set_property(0x20,0x01,0x00,0x00);		//设置property 0x2000为0x00,进入CW状态(AN625 Page97)
	si446x_change_state(SI446X_STATE_TUNE_TX);	//设置芯片进入TX TUNE状态
  si446x_get_int_status(0u, 0u, 0u);					//清楚中断标志

	si446x_start_tx(channel_number, 0u, 0u);								//启动发射过程
	while(1);
}

void vRadio_BER_StartRX(void)
{
	si446x_gpio_pin_cfg(17,20,17,20,0,0,0);						//配置GPIO0、GPIO2为RX CLK，GPIO1、GPIO3为RX DAT

	si446x_set_property(0x10,0x01,0x01,0x88);					//禁止同步字搜索超时，AN625 Page66,'PREAMBLE_CONFIG_STD_1'

	si446x_set_property(0x12,0x01,0x06,0x40);					//禁止接收模式PH，AN625 Page73,'PKT_CONFIG1'
	

  si446x_get_int_status(0u, 0u, 0u);

  /* Start Receiving packet, channel 0, START immediately, Packet off  */
  si446x_start_rx(0, 0u, 0u,
                  SI446X_CMD_START_RX_ARG_NEXT_STATE1_RXTIMEOUT_STATE_ENUM_NOCHANGE,
                  SI446X_CMD_START_RX_ARG_NEXT_STATE2_RXVALID_STATE_ENUM_RX,
                  SI446X_CMD_START_RX_ARG_NEXT_STATE3_RXINVALID_STATE_ENUM_RX );
}




