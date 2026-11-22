/*!
 * File:
 *  si446x_api_lib.c
 *
 * Description:
 *  This file contains the Si446x API library.
 *
 * Silicon Laboratories Confidential
 * Copyright 2011 Silicon Laboratories, Inc.
 */

#include "bsp.h"
#include <stdarg.h>

//SEGMENT_VARIABLE( Si446xCmd, union si446x_cmd_reply_union, SEG_XDATA );
//SEGMENT_VARIABLE( Pro2Cmd[16], uint8_t, SEG_XDATA );

union si446x_cmd_reply_union	Si446xCmd;
uint8_t	Pro2Cmd[16];
uint8_t Si446xPatchCommands[][8]={SI446X_PATCH_CMDS};

/*!
 * This functions is used to reset the si446x radio by applying shutdown and
 * releasing it.  After this function @ref si446x_boot should be called.  You
 * can check if POR has completed by waiting 4 ms or by polling GPIO 0, 2, or 3.
 * When these GPIOs are high, it is safe to call @ref si446x_boot.
 */
void si446x_reset(void)
{
    uint8_t loopCount;

    /* Put radio in shutdown, wait then release */
    radio_hal_AssertShutdown();
    //! @todo this needs to be a better delay function.
    //for (loopCount = 255; loopCount != 0; loopCount--);
    radio_hal_DeassertShutdown();
    //for (loopCount = 255; loopCount != 0; loopCount--);
    while(!SI_GPIO1_STATUS);
    radio_comm_ClearCTS();
}

/*!
 * This function is used to initialize after power-up the radio chip.
 * Before this function @si446x_reset should be called.
 */
void si446x_power_up(uint8_t BOOT_OPTIONS, uint8_t XTAL_OPTIONS, uint32_t XO_FREQ)
{
    Pro2Cmd[0] = SI446X_CMD_ID_POWER_UP;
    Pro2Cmd[1] = BOOT_OPTIONS;
    Pro2Cmd[2] = XTAL_OPTIONS;
    Pro2Cmd[3] = (uint8_t)(XO_FREQ >> 24);
    Pro2Cmd[4] = (uint8_t)(XO_FREQ >> 16);
    Pro2Cmd[5] = (uint8_t)(XO_FREQ >> 8);
    Pro2Cmd[6] = (uint8_t)(XO_FREQ);

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_POWER_UP, Pro2Cmd );
}

/*!
 * This function is used to load all properties and commands with a list of NULL terminated commands.
 * Before this function @si446x_reset should be called.
 */
uint8_t si446x_configuration_init(const uint8_t* pSetPropCmd)
{
  uint8_t col,numOfBytes;

  /* While cycle as far as the pointer points to a command */
  while (*pSetPropCmd != 0x00)
  {
    /* Commands structure in the array:
     * --------------------------------
     * LEN | <LEN length of data>
     */

    numOfBytes = *pSetPropCmd++;

    if (numOfBytes > 16u)
    {
      /* Number of command bytes exceeds maximal allowable length */
      return SI446X_COMMAND_ERROR;
    }

    for (col = 0u; col < numOfBytes; col++)
    {
      Pro2Cmd[col] = *pSetPropCmd;
      pSetPropCmd++;
    }

    if (radio_comm_SendCmdGetResp(numOfBytes, Pro2Cmd, 0, 0) != 0xFF)
    {
      /* Timeout occured */
      return SI446X_CTS_TIMEOUT;
    }

    if (radio_hal_NirqLevel() == 0)
    {
      /* Get and clear all interrupts.  An error has occured... */
      si446x_get_int_status(0, 0, 0);
      if (Si446xCmd.GET_INT_STATUS.CHIP_PEND & SI446X_CMD_GET_CHIP_STATUS_REP_CHIP_PEND_CMD_ERROR_PEND_MASK)
      {
        return SI446X_COMMAND_ERROR;
      }
    }
  }

  return SI446X_SUCCESS;
}

/*! This function sends the PART_INFO command to the radio and receives the answer
 *  into @Si446xCmd union.
 */
void si446x_part_info(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_PART_INFO;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_PART_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_PART_INFO,
                              Pro2Cmd );

    Si446xCmd.PART_INFO.CHIPREV         = Pro2Cmd[0];
    Si446xCmd.PART_INFO.PART            = ((uint16_t)Pro2Cmd[1] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.PART           |= (uint16_t)Pro2Cmd[2] & 0x00FF;
    Si446xCmd.PART_INFO.PBUILD          = Pro2Cmd[3];
    Si446xCmd.PART_INFO.ID              = ((uint16_t)Pro2Cmd[4] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.ID             |= (uint16_t)Pro2Cmd[5] & 0x00FF;
    Si446xCmd.PART_INFO.CUSTOMER        = Pro2Cmd[6];
    Si446xCmd.PART_INFO.ROMID           = Pro2Cmd[7];
}

/*! Sends START_TX command to the radio.
 *
 * @param CHANNEL   Channel number.
 * @param CONDITION Start TX condition.
 * @param TX_LEN    Payload length (exclude the PH generated CRC).
 */
void si446x_start_tx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t TX_LEN)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = (uint8_t)(TX_LEN >> 8);
    Pro2Cmd[4] = (uint8_t)(TX_LEN);
    Pro2Cmd[5] = 0x00;

    // Don't repeat the packet, 
    // ie. transmit the packet only once
    Pro2Cmd[6] = 0x00;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_START_TX, Pro2Cmd );
}

/*!
 * Sends START_RX command to the radio.
 *
 * @param CHANNEL     Channel number.
 * @param CONDITION   Start RX condition.
 * @param RX_LEN      Payload length (exclude the PH generated CRC).
 * @param NEXT_STATE1 Next state when Preamble Timeout occurs.
 * @param NEXT_STATE2 Next state when a valid packet received.
 * @param NEXT_STATE3 Next state when invalid packet received (e.g. CRC error).
 */
void si446x_start_rx(uint8_t CHANNEL, uint8_t CONDITION, uint16_t RX_LEN, uint8_t NEXT_STATE1, uint8_t NEXT_STATE2, uint8_t NEXT_STATE3)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = (uint8_t)(RX_LEN >> 8);
    Pro2Cmd[4] = (uint8_t)(RX_LEN);
    Pro2Cmd[5] = NEXT_STATE1;
    Pro2Cmd[6] = NEXT_STATE2;
    Pro2Cmd[7] = NEXT_STATE3;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_START_RX, Pro2Cmd );
}

/*!
 * Get the Interrupt status/pending flags form the radio and clear flags if requested.
 *
 * @param PH_CLR_PEND     Packet Handler pending flags clear.
 * @param MODEM_CLR_PEND  Modem Status pending flags clear.
 * @param CHIP_CLR_PEND   Chip State pending flags clear.
 */
void si446x_get_int_status(uint8_t PH_CLR_PEND, uint8_t MODEM_CLR_PEND, uint8_t CHIP_CLR_PEND)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_INT_STATUS;
    Pro2Cmd[1] = PH_CLR_PEND;
    Pro2Cmd[2] = MODEM_CLR_PEND;
    Pro2Cmd[3] = CHIP_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_INT_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_INT_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_INT_STATUS.INT_PEND       = Pro2Cmd[0];
    Si446xCmd.GET_INT_STATUS.INT_STATUS     = Pro2Cmd[1];
    Si446xCmd.GET_INT_STATUS.PH_PEND        = Pro2Cmd[2];
    Si446xCmd.GET_INT_STATUS.PH_STATUS      = Pro2Cmd[3];
    Si446xCmd.GET_INT_STATUS.MODEM_PEND     = Pro2Cmd[4];
    Si446xCmd.GET_INT_STATUS.MODEM_STATUS   = Pro2Cmd[5];
    Si446xCmd.GET_INT_STATUS.CHIP_PEND      = Pro2Cmd[6];
    Si446xCmd.GET_INT_STATUS.CHIP_STATUS    = Pro2Cmd[7];
}

/*!
 * Send GPIO pin config command to the radio and reads the answer into
 * @Si446xCmd union.
 *
 * @param GPIO0       GPIO0 configuration.
 * @param GPIO1       GPIO1 configuration.
 * @param GPIO2       GPIO2 configuration.
 * @param GPIO3       GPIO3 configuration.
 * @param NIRQ        NIRQ configuration.
 * @param SDO         SDO configuration.
 * @param GEN_CONFIG  General pin configuration.
 */
void si446x_gpio_pin_cfg(uint8_t GPIO0, uint8_t GPIO1, uint8_t GPIO2, uint8_t GPIO3, uint8_t NIRQ, uint8_t SDO, uint8_t GEN_CONFIG)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GPIO_PIN_CFG;
    Pro2Cmd[1] = GPIO0;
    Pro2Cmd[2] = GPIO1;
    Pro2Cmd[3] = GPIO2;
    Pro2Cmd[4] = GPIO3;
    Pro2Cmd[5] = NIRQ;
    Pro2Cmd[6] = SDO;
    Pro2Cmd[7] = GEN_CONFIG;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GPIO_PIN_CFG,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GPIO_PIN_CFG,
                              Pro2Cmd );

    Si446xCmd.GPIO_PIN_CFG.GPIO[0]        = Pro2Cmd[0];
    Si446xCmd.GPIO_PIN_CFG.GPIO[1]        = Pro2Cmd[1];
    Si446xCmd.GPIO_PIN_CFG.GPIO[2]        = Pro2Cmd[2];
    Si446xCmd.GPIO_PIN_CFG.GPIO[3]        = Pro2Cmd[3];
    Si446xCmd.GPIO_PIN_CFG.NIRQ         = Pro2Cmd[4];
    Si446xCmd.GPIO_PIN_CFG.SDO          = Pro2Cmd[5];
    Si446xCmd.GPIO_PIN_CFG.GEN_CONFIG   = Pro2Cmd[6];
}

/*******************************************************************************************************************	
Function Name:			si446x_set_property( uint8_t GROUP, uint8_t NUM_PROPS, uint8_t START_PROP, ... )
Brief Comment:			Set property values for the radio
Input Parameter:		GROUP@ Property group number
										NUM_PROPS@ Number of properties to be written
										START_PROP@ Starting sub-property number
Return Value:				Property value will be load into Si446xCmd.GET_PROPERTY
Notes:							Command ID=0x11(SET_PROPERTY)
*******************************************************************************************************************/
void si446x_set_property(uint8_t GROUP,uint8_t NUM_PROPS,int START_PROP, ... )
{
  va_list argList;
  uint8_t cmdIndex;

  Pro2Cmd[0]=SI446X_CMD_ID_SET_PROPERTY;
  Pro2Cmd[1]=GROUP;
  Pro2Cmd[2]=NUM_PROPS;
  Pro2Cmd[3]=(uint8_t)START_PROP;

  va_start(argList,START_PROP);
  cmdIndex=4;
  while(NUM_PROPS--)
    {
      Pro2Cmd[cmdIndex]=(uint8_t)va_arg(argList,int);
      cmdIndex++;
    }
  va_end(argList);

  radio_comm_SendCmd(cmdIndex,Pro2Cmd);
}

/*******************************************************************************************************************	
Function Name:			si446x_get_property(uint8_t GROUP, uint8_t NUM_PROPS, uint8_t START_PROP)
Brief Comment:			Get property values from the radio,Reads them into Si446xCmd union
Input Parameter:		GROUP@ Property group number
										NUM_PROPS@ Number of properties to be read
										START_PROP@ Starting sub-property number
Return Value:				None
Notes:							Command ID=0x11(GET_PROPERTY)
*******************************************************************************************************************/
void si446x_get_property(uint8_t GROUP, uint8_t NUM_PROPS, uint8_t START_PROP)
{
  Pro2Cmd[0]=SI446X_CMD_ID_GET_PROPERTY;
  Pro2Cmd[1]=GROUP;
  Pro2Cmd[2]=NUM_PROPS;
  Pro2Cmd[3]=START_PROP;

  radio_comm_SendCmdGetResp(SI446X_CMD_ARG_COUNT_GET_PROPERTY,
                            Pro2Cmd,
                            Pro2Cmd[2],
                            Pro2Cmd);

  Si446xCmd.GET_PROPERTY.DATA[0]=Pro2Cmd[0];
  Si446xCmd.GET_PROPERTY.DATA[1]=Pro2Cmd[1];
  Si446xCmd.GET_PROPERTY.DATA[2]=Pro2Cmd[2];
  Si446xCmd.GET_PROPERTY.DATA[3]=Pro2Cmd[3];
  Si446xCmd.GET_PROPERTY.DATA[4]=Pro2Cmd[4];
  Si446xCmd.GET_PROPERTY.DATA[5]=Pro2Cmd[5];
  Si446xCmd.GET_PROPERTY.DATA[6]=Pro2Cmd[6];
  Si446xCmd.GET_PROPERTY.DATA[7]=Pro2Cmd[7];
  Si446xCmd.GET_PROPERTY.DATA[8]=Pro2Cmd[8];
  Si446xCmd.GET_PROPERTY.DATA[9]=Pro2Cmd[9];
  Si446xCmd.GET_PROPERTY.DATA[10]=Pro2Cmd[10];
  Si446xCmd.GET_PROPERTY.DATA[11]=Pro2Cmd[11];
  Si446xCmd.GET_PROPERTY.DATA[12]=Pro2Cmd[12];
  Si446xCmd.GET_PROPERTY.DATA[13]=Pro2Cmd[13];
  Si446xCmd.GET_PROPERTY.DATA[14]=Pro2Cmd[14];
  Si446xCmd.GET_PROPERTY.DATA[15]=Pro2Cmd[15];
}


/*******************************************************************************************************************	
Function Name:			si446x_change_state(uint8_t NEXT_STATE1)
Brief Comment:			Update state machine entries
Input Parameter:		NEXT_STATE1@ New state to be updated
Return Value:				None 
Notes:							Command ID=0x34(CHANGE_STATE)
										See AN625 Page43 for state details
*******************************************************************************************************************/
void si446x_change_state(uint8_t NEXT_STATE1)
{
  Pro2Cmd[0]=SI446X_CMD_ID_CHANGE_STATE;
  Pro2Cmd[1]=NEXT_STATE1;

  radio_comm_SendCmd(SI446X_CMD_ARG_COUNT_CHANGE_STATE, Pro2Cmd);
}

/*******************************************************************************************************************	
Function Name:			si446x_nop(void)
Brief Comment:			No operation command
Input Parameter:		None
Return Value:				None 
Notes:							Command ID=0x00(NOP)
*******************************************************************************************************************/
void si446x_nop(void)
{
	Pro2Cmd[0] = SI446X_CMD_ID_NOP;
  radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_NOP, Pro2Cmd );
}

/*******************************************************************************************************************	
Function Name:			si446x_fifo_info(uint8_t FIFO)
Brief Comment:			Provides access to transmit and receive fifo counts and reset
Input Parameter:		FIFO@ 0x01 for TX FIFO reset, 0x02 for RX FIFO reset
Return Value:				Pro2Cmd[0]@ RX FIFO count
										Pro2Cmd[1]@ Amount of space currently available in TX FIFO 
Notes:							Command ID=0x15(FIFO_INFO)
*******************************************************************************************************************/
void si446x_fifo_info(uint8_t FIFO)
{
  Pro2Cmd[0]=SI446X_CMD_ID_FIFO_INFO;
  Pro2Cmd[1]=FIFO;

  radio_comm_SendCmdGetResp(SI446X_CMD_ARG_COUNT_FIFO_INFO,
                             Pro2Cmd,
                             SI446X_CMD_REPLY_COUNT_FIFO_INFO,
                             Pro2Cmd);

  Si446xCmd.FIFO_INFO.RX_FIFO_COUNT=Pro2Cmd[0];
  Si446xCmd.FIFO_INFO.TX_FIFO_SPACE=Pro2Cmd[1];
}

/*******************************************************************************************************************	
Function Name:			si446x_write_tx_fifo(uint8_t numBytes, uint8_t* pTxData)
Brief Comment:			Writes data array to TX FIFO
Input Parameter:		numBytes@ Length of data array need to be loaded to TX FIFO 
										pTxData@ Pointer of data array need to be loaded to TX FIFO
Return Value:				None
Notes:							Command ID=0x66(WRITE_TX_FIFO)
*******************************************************************************************************************/
void si446x_write_tx_fifo(uint8_t numBytes, uint8_t* pTxData)
{
  radio_comm_WriteData(SI446X_CMD_ID_WRITE_TX_FIFO,0,numBytes,pTxData);
}

/*******************************************************************************************************************	
Function Name:			si446x_read_rx_fifo(uint8_t numBytes, uint8_t* pTxData)
Brief Comment:			Read data array from RX FIFO
Input Parameter:		numBytes@ Length of data array need to be read from RX FIFO 
										pTxData@ Pointer of data array to store data read for RX FIFO
Return Value:				None
Notes:							Command ID=0x77(READ_RX_FIFO)
*******************************************************************************************************************/
void si446x_read_rx_fifo(uint8_t numBytes, uint8_t* pRxData)
{
  radio_comm_ReadData(SI446X_CMD_ID_READ_RX_FIFO,0,numBytes,pRxData);
}

/* Full driver support functions */

/*!
 * Sends the FUNC_INFO command to the radio, then reads the resonse into @Si446xCmd union.
 */
void si446x_func_info(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_FUNC_INFO;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_FUNC_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_FUNC_INFO,
                              Pro2Cmd );

    Si446xCmd.FUNC_INFO.REVEXT          = Pro2Cmd[0];
    Si446xCmd.FUNC_INFO.REVBRANCH       = Pro2Cmd[1];
    Si446xCmd.FUNC_INFO.REVINT          = Pro2Cmd[2];
    Si446xCmd.FUNC_INFO.FUNC            = Pro2Cmd[5];
}

/*!
 * Reads the Fast Response Registers starting with A register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_a_read(uint8_t respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_A_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_A_READ.FRR_A_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_A_READ.FRR_B_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_A_READ.FRR_C_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_A_READ.FRR_D_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with B register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_b_read(uint8_t respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_B_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_B_READ.FRR_B_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_B_READ.FRR_C_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_B_READ.FRR_D_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_B_READ.FRR_A_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with C register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_c_read(uint8_t respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_C_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_C_READ.FRR_C_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_C_READ.FRR_D_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_C_READ.FRR_A_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_C_READ.FRR_B_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the Fast Response Registers starting with D register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
void si446x_frr_d_read(uint8_t respByteCount)
{
    radio_comm_ReadData(SI446X_CMD_ID_FRR_D_READ,
                            0,
                        respByteCount,
                        Pro2Cmd);

    Si446xCmd.FRR_D_READ.FRR_D_VALUE = Pro2Cmd[0];
    Si446xCmd.FRR_D_READ.FRR_A_VALUE = Pro2Cmd[1];
    Si446xCmd.FRR_D_READ.FRR_B_VALUE = Pro2Cmd[2];
    Si446xCmd.FRR_D_READ.FRR_C_VALUE = Pro2Cmd[3];
}

/*!
 * Reads the ADC values from the radio into @Si446xCmd union.
 *
 * @param ADC_EN  ADC enable parameter.
 */
void si446x_get_adc_reading(uint8_t ADC_EN)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_ADC_READING;
    Pro2Cmd[1] = ADC_EN;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_ADC_READING,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_ADC_READING,
                              Pro2Cmd );

    Si446xCmd.GET_ADC_READING.GPIO_ADC         = ((uint16_t)Pro2Cmd[0] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.GPIO_ADC        |=  (uint16_t)Pro2Cmd[1] & 0x00FF;
    Si446xCmd.GET_ADC_READING.BATTERY_ADC      = ((uint16_t)Pro2Cmd[2] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.BATTERY_ADC     |=  (uint16_t)Pro2Cmd[3] & 0x00FF;
    Si446xCmd.GET_ADC_READING.TEMP_ADC         = ((uint16_t)Pro2Cmd[4] << 8) & 0xFF00;
    Si446xCmd.GET_ADC_READING.TEMP_ADC        |=  (uint16_t)Pro2Cmd[5] & 0x00FF;
}

/*!
 * Receives information from the radio of the current packet. Optionally can be used to modify
 * the Packet Handler properties during packet reception.
 *
 * @param FIELD_NUMBER_MASK Packet Field number mask value.
 * @param LEN               Length value.
 * @param DIFF_LEN          Difference length.
 */
void si446x_get_packet_info(uint8_t FIELD_NUMBER_MASK, uint16_t LEN, int16_t DIFF_LEN )
{
    Pro2Cmd[0] = SI446X_CMD_ID_PACKET_INFO;
    Pro2Cmd[1] = FIELD_NUMBER_MASK;
    Pro2Cmd[2] = (uint8_t)(LEN >> 8);
    Pro2Cmd[3] = (uint8_t)(LEN);
    // the different of the byte, althrough it is signed, but to command hander
    // it can treat it as unsigned
    Pro2Cmd[4] = (uint8_t)((uint16_t)DIFF_LEN >> 8);
    Pro2Cmd[5] = (uint8_t)(DIFF_LEN);

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_PACKET_INFO,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_PACKET_INFO,
                              Pro2Cmd );

    Si446xCmd.PACKET_INFO.LENGTH = ((uint16_t)Pro2Cmd[0] << 8) & 0xFF00;
    Si446xCmd.PACKET_INFO.LENGTH |= (uint16_t)Pro2Cmd[1] & 0x00FF;
}

/*!
 * Gets the Packet Handler status flags. Optionally clears them.
 *
 * @param PH_CLR_PEND Flags to clear.
 */
void si446x_get_ph_status(uint8_t PH_CLR_PEND)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_PH_STATUS;
    Pro2Cmd[1] = PH_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_PH_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_PH_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_PH_STATUS.PH_PEND        = Pro2Cmd[0];
    Si446xCmd.GET_PH_STATUS.PH_STATUS      = Pro2Cmd[1];
}

/*!
 * Gets the Modem status flags. Optionally clears them.
 *
 * @param MODEM_CLR_PEND Flags to clear.
 */
void si446x_get_modem_status( uint8_t MODEM_CLR_PEND )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    Pro2Cmd[1] = MODEM_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = Pro2Cmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = Pro2Cmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = Pro2Cmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = Pro2Cmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = Pro2Cmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = Pro2Cmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((uint16_t)Pro2Cmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (uint16_t)Pro2Cmd[7] & 0x00FF;
}

/*!
 * Gets the Chip status flags. Optionally clears them.
 *
 * @param CHIP_CLR_PEND Flags to clear.
 */
void si446x_get_chip_status( uint8_t CHIP_CLR_PEND )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_CHIP_STATUS;
    Pro2Cmd[1] = CHIP_CLR_PEND;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_GET_CHIP_STATUS,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_CHIP_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_CHIP_STATUS.CHIP_PEND         = Pro2Cmd[0];
    Si446xCmd.GET_CHIP_STATUS.CHIP_STATUS       = Pro2Cmd[1];
    Si446xCmd.GET_CHIP_STATUS.CMD_ERR_STATUS    = Pro2Cmd[2];
}

/*!
 * Performs image rejection calibration. Completion can be monitored by polling CTS or waiting for CHIP_READY interrupt source.
 *
 * @param SEARCHING_STEP_SIZE
 * @param SEARCHING_RSSI_AVG
 * @param RX_CHAIN_SETTING1
 * @param RX_CHAIN_SETTING2
 */
void si446x_ircal(uint8_t SEARCHING_STEP_SIZE, uint8_t SEARCHING_RSSI_AVG, uint8_t RX_CHAIN_SETTING1, uint8_t RX_CHAIN_SETTING2)
{
    Pro2Cmd[0] = SI446X_CMD_ID_IRCAL;
    Pro2Cmd[1] = SEARCHING_STEP_SIZE;
    Pro2Cmd[2] = SEARCHING_RSSI_AVG;
    Pro2Cmd[3] = RX_CHAIN_SETTING1;
    Pro2Cmd[4] = RX_CHAIN_SETTING2;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_IRCAL, Pro2Cmd);
}


/*!
 * Image rejection calibration. Forces a specific value for IR calibration, and reads back calibration values from previous calibrations
 *
 * @param IRCAL_AMP
 * @param IRCAL_PH
 */
void si446x_ircal_manual(uint8_t IRCAL_AMP, uint8_t IRCAL_PH)
{
    Pro2Cmd[0] = SI446X_CMD_ID_IRCAL_MANUAL;
    Pro2Cmd[1] = IRCAL_AMP;
    Pro2Cmd[2] = IRCAL_PH;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_IRCAL_MANUAL,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_IRCAL_MANUAL,
                              Pro2Cmd );

    Si446xCmd.IRCAL_MANUAL.IRCAL_AMP_REPLY   = Pro2Cmd[0];
    Si446xCmd.IRCAL_MANUAL.IRCAL_PH_REPLY    = Pro2Cmd[1];
}

/*!
 * Requests the current state of the device and lists pending TX and RX requests
 */
void si446x_request_device_state(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd );

    Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE       = Pro2Cmd[0];
    Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL  = Pro2Cmd[1];
}

/*!
 * While in TX state this will hop to the frequency specified by the parameters
 *
 * @param INTE      New INTE register value.
 * @param FRAC2     New FRAC2 register value.
 * @param FRAC1     New FRAC1 register value.
 * @param FRAC0     New FRAC0 register value.
 * @param VCO_CNT1  New VCO_CNT1 register value.
 * @param VCO_CNT0  New VCO_CNT0 register value.
 * @param PLL_SETTLE_TIME1  New PLL_SETTLE_TIME1 register value.
 * @param PLL_SETTLE_TIME0  New PLL_SETTLE_TIME0 register value.
 */
void si446x_tx_hop(uint8_t INTE, uint8_t FRAC2, uint8_t FRAC1, uint8_t FRAC0, uint8_t VCO_CNT1, uint8_t VCO_CNT0, uint8_t PLL_SETTLE_TIME1, uint8_t PLL_SETTLE_TIME0)
{
    Pro2Cmd[0] = SI446X_CMD_ID_TX_HOP;
    Pro2Cmd[1] = INTE;
    Pro2Cmd[2] = FRAC2;
    Pro2Cmd[3] = FRAC1;
    Pro2Cmd[4] = FRAC0;
    Pro2Cmd[5] = VCO_CNT1;
    Pro2Cmd[6] = VCO_CNT0;
    Pro2Cmd[7] = PLL_SETTLE_TIME1;
    Pro2Cmd[8] = PLL_SETTLE_TIME0;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_TX_HOP, Pro2Cmd );
}

/*!
 * While in RX state this will hop to the frequency specified by the parameters and start searching for a preamble.
 *
 * @param INTE      New INTE register value.
 * @param FRAC2     New FRAC2 register value.
 * @param FRAC1     New FRAC1 register value.
 * @param FRAC0     New FRAC0 register value.
 * @param VCO_CNT1  New VCO_CNT1 register value.
 * @param VCO_CNT0  New VCO_CNT0 register value.
 */
void si446x_rx_hop(uint8_t INTE, uint8_t FRAC2, uint8_t FRAC1, uint8_t FRAC0, uint8_t VCO_CNT1, uint8_t VCO_CNT0)
{
    Pro2Cmd[0] = SI446X_CMD_ID_RX_HOP;
    Pro2Cmd[1] = INTE;
    Pro2Cmd[2] = FRAC2;
    Pro2Cmd[3] = FRAC1;
    Pro2Cmd[4] = FRAC0;
    Pro2Cmd[5] = VCO_CNT1;
    Pro2Cmd[6] = VCO_CNT0;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_RX_HOP, Pro2Cmd );
}

/*! Sends START_TX command ID to the radio with no input parameters
 *
 */
void si446x_start_tx_fast( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;

    radio_comm_SendCmd( 1, Pro2Cmd );
}

/*!
 * Sends START_RX command ID to the radio with no input parameters
 *
 */
void si446x_start_rx_fast( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;

    radio_comm_SendCmd( 1, Pro2Cmd );
}

/*!
 * Clear all Interrupt status/pending flags. Does NOT read back interrupt flags
 *
 */
void si446x_get_int_status_fast_clear( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_INT_STATUS;

    radio_comm_SendCmd( 1, Pro2Cmd );
}

/*!
 * Clear and read all Interrupt status/pending flags
 *
 */
void si446x_get_int_status_fast_clear_read(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_INT_STATUS;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_INT_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_INT_STATUS.INT_PEND       = Pro2Cmd[0];
    Si446xCmd.GET_INT_STATUS.INT_STATUS     = Pro2Cmd[1];
    Si446xCmd.GET_INT_STATUS.PH_PEND        = Pro2Cmd[2];
    Si446xCmd.GET_INT_STATUS.PH_STATUS      = Pro2Cmd[3];
    Si446xCmd.GET_INT_STATUS.MODEM_PEND     = Pro2Cmd[4];
    Si446xCmd.GET_INT_STATUS.MODEM_STATUS   = Pro2Cmd[5];
    Si446xCmd.GET_INT_STATUS.CHIP_PEND      = Pro2Cmd[6];
    Si446xCmd.GET_INT_STATUS.CHIP_STATUS    = Pro2Cmd[7];
}

/*!
 * Reads back current GPIO pin configuration. Does NOT configure GPIO pins
  *
 */
void si446x_gpio_pin_cfg_fast( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GPIO_PIN_CFG;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GPIO_PIN_CFG,
                              Pro2Cmd );

    Si446xCmd.GPIO_PIN_CFG.GPIO[0]        = Pro2Cmd[0];
    Si446xCmd.GPIO_PIN_CFG.GPIO[1]        = Pro2Cmd[1];
    Si446xCmd.GPIO_PIN_CFG.GPIO[2]        = Pro2Cmd[2];
    Si446xCmd.GPIO_PIN_CFG.GPIO[3]        = Pro2Cmd[3];
    Si446xCmd.GPIO_PIN_CFG.NIRQ         = Pro2Cmd[4];
    Si446xCmd.GPIO_PIN_CFG.SDO          = Pro2Cmd[5];
    Si446xCmd.GPIO_PIN_CFG.GEN_CONFIG   = Pro2Cmd[6];
}


/*!
 * Clear all Packet Handler status flags. Does NOT read back interrupt flags
 *
 */
void si446x_get_ph_status_fast_clear( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_PH_STATUS;
    Pro2Cmd[1] = 0;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Clear and read all Packet Handler status flags.
 *
 */
void si446x_get_ph_status_fast_clear_read(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_PH_STATUS;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_PH_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_PH_STATUS.PH_PEND        = Pro2Cmd[0];
    Si446xCmd.GET_PH_STATUS.PH_STATUS      = Pro2Cmd[1];
}

/*!
 * Clear all Modem status flags. Does NOT read back interrupt flags
 *
 */
void si446x_get_modem_status_fast_clear( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    Pro2Cmd[1] = 0;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Clear and read all Modem status flags.
 *
 */
void si446x_get_modem_status_fast_clear_read( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = Pro2Cmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = Pro2Cmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = Pro2Cmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = Pro2Cmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = Pro2Cmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = Pro2Cmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET = ((uint16_t)Pro2Cmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (uint16_t)Pro2Cmd[7] & 0x00FF;
}

/*!
 * Clear all Chip status flags. Does NOT read back interrupt flags
 *
 */
void si446x_get_chip_status_fast_clear( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_CHIP_STATUS;
    Pro2Cmd[1] = 0;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Clear and read all Chip status flags.
 *
 */
void si446x_get_chip_status_fast_clear_read( void )
{
    Pro2Cmd[0] = SI446X_CMD_ID_GET_CHIP_STATUS;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_GET_CHIP_STATUS,
                              Pro2Cmd );

    Si446xCmd.GET_CHIP_STATUS.CHIP_PEND         = Pro2Cmd[0];
    Si446xCmd.GET_CHIP_STATUS.CHIP_STATUS       = Pro2Cmd[1];
    Si446xCmd.GET_CHIP_STATUS.CMD_ERR_STATUS    = Pro2Cmd[2];
}

/*!
 * Resets the RX/TX FIFO. Does not read back anything from TX/RX FIFO
 *
 */
void si446x_fifo_info_fast_reset(uint8_t FIFO)
{
    Pro2Cmd[0] = SI446X_CMD_ID_FIFO_INFO;
    Pro2Cmd[1] = FIFO;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Reads RX/TX FIFO count space. Does NOT reset RX/TX FIFO
 *
 */
void si446x_fifo_info_fast_read(void)
{
    Pro2Cmd[0] = SI446X_CMD_ID_FIFO_INFO;

    radio_comm_SendCmdGetResp( 1,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_FIFO_INFO,
                              Pro2Cmd );

    Si446xCmd.FIFO_INFO.RX_FIFO_COUNT   = Pro2Cmd[0];
    Si446xCmd.FIFO_INFO.TX_FIFO_SPACE   = Pro2Cmd[1];
}
/*******************************************************************************************************************	
Function Name:			si446x_pa_pwr(uint8_t pwr_lvl)
Brief Comment:			Define output power level of radio chip
Input Parameter:		pwr_lvl@ power level of radio chip
Return Value:				None
Notes:							PA_PWR_LVL(0x2201), pwr_lvl=0x7F is the max power, 20dBm
*******************************************************************************************************************/
void si446x_pa_pwr(uint8_t pwr_lvl)
{
	si446x_set_property(0x22,0x01,0x01,pwr_lvl);			//设置功率等级(AN625 Page 106)
}

/*******************************************************************************************************************	
Function Name:			si446x_sync_config(uint8_t sync_length, uint8_t *pSync_Array)
Brief Comment:			User defined sync word configuration
Input Parameter:		sync_length@ length of re-defined sync word
										pSync_Array@ Poniter of data array used to re-define sync word
Return Value:				None
Notes:							SYNC_CONFIG(0x1100-0x1104), define sync word length and content of sync word
*******************************************************************************************************************/
void si446x_sync_config(uint8_t sync_length, uint8_t *pSync_Array)
{
	uint8_t i;
	si446x_set_property(0x11,0x01,0x00,(sync_length-1));			//设置同步字长度(AN625 Page 71)
	for(i=0;i<sync_length;i++)
		{
			si446x_set_property(0x11,0x01,(0x01+i),*pSync_Array);	//设置同步字内容
			pSync_Array++;
		}
}
/*******************************************************************************************************************	
Function Name:			si446x_preamble_config(uint8_t preamble_length, uint8_t preamble_rx_thresh, uint8_t preamble_rx_timeout)
Brief Comment:			User defined preamble configuration
Input Parameter:		preamble_length@ length of TX preamble
										preamble_rx_thresh@ min preable bits receiver needs to generate PREAMBLE_DETECT_PEND
										preamble_rx_timeout@ preable timeout value in nibbles
Return Value:				None
Notes:							PREAMBLE_TX_LENGTH(0x1000)
*******************************************************************************************************************/
void si446x_preamble_config(uint8_t preamble_length, uint8_t preamble_rx_thresh, uint8_t preamble_rx_timeout)
{
	si446x_set_property(0x10,0x01,0x00,preamble_length);		//设置发射前导长度(AN625 Page 66)
	si446x_set_property(0x10,0x01,0x01,preamble_rx_thresh);	//接收机需要侦测的前导最少位数，默认值为0x14,
																													//接收机接收满最少位前导后产生PREAMBLE_DETECT_PEND
	si446x_set_property(0x10,0x01,0x03,preamble_rx_timeout);//接收机侦测前导超时时间，默认值为0x0F,即15*4=60bits
																													//高四位计算为15*[7:4]*4(AN625 Page 67)
																													//超时后产生INVALID_PREAMBLE_PEND
}
/*******************************************************************************************************************	
Function Name:			si446x_pkt_f1_length_config(uint8_t pkt_length)
Brief Comment:			Configure filed 1 length of the packet
Input Parameter:		pkt_length@ filed 1 length of the packet
Return Value:				None
Notes:							PREAMBLE_TX_LENGTH(0x1000)
*******************************************************************************************************************/
void si446x_pkt_f1_length_config(uint8_t pkt_length)
{
	si446x_set_property(0x12,0x01,0x0E,pkt_length);		//配置Packet Filed 1长度
}
/*******************************************************************************************************************	
Function Name:			si446x_datarate_config(uint32_t dr)
Brief Comment:			Configure Radio data rate
Input Parameter:		dr@ data rate in bps, eg. 9.6Kbps=9600
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void si446x_datarate_config(uint32_t dr)
{
	uint8_t dr_23_16,dr_15_8,dr_7_0;
	
	dr=(dr*40);
	dr_23_16=(uint8_t)((dr&0xFF0000)>>16);
	dr_15_8=(uint8_t)((dr&0xFF00)>>8);
	dr_7_0=(uint8_t)(dr&0xFF);
	
	si446x_set_property(0x20,0x01,0x03,dr_23_16);		//配置'MODEM_DATA_RATE_2'
	si446x_set_property(0x20,0x01,0x04,dr_15_8);		//配置'MODEM_DATA_RATE_1'
	si446x_set_property(0x20,0x01,0x05,dr_7_0);			//配置'MODEM_DATA_RATE_0'
}
/*******************************************************************************************************************	
Function Name:			si446x_freqdev_config(uint32_t dev)
Brief Comment:			Configure Radio frequency deviation
Input Parameter:		dev@ frequency deviation,	caculation unkonwn yet!
Return Value:				None
Notes:							19.2KHz=0x53E
*******************************************************************************************************************/
void si446x_freqdev_config(uint32_t dev)
{
	uint8_t dev_17,dev_15_8,dev_7_0;
	
	dev_17=(uint8_t)((dev&0x10000)>>16);
	dev_15_8=(uint8_t)((dev&0xFF00)>>8);
	dev_7_0=(uint8_t)(dev&0xFF);
	
	si446x_set_property(0x20,0x01,0x0A,dev_17);		//配置'MODEM_FREQ_DEV_2'
	si446x_set_property(0x20,0x01,0x0B,dev_15_8);	//配置'MODEM_FREQ_DEV_1'
	si446x_set_property(0x20,0x01,0x0C,dev_7_0);	//配置'MODEM_FREQ_DEV_0'
}
/*******************************************************************************************************************	
Function Name:			si446x_irq_config(uint8_t int_global, uint8_t int_ph, uint8_t int_modem, uint8_t int_chip)
Brief Comment:			Configure Interrupt of radio chip
Input Parameter:		int_global@ radio chip global chip interrupt configure
										int_ph@ radio chip package handler interrupt configure
										int_modem@ radio chip modem interrupt configure
										int_chip@ chip interrupt configure
Return Value:				None
Notes:							19.2KHz=0x53E
*******************************************************************************************************************/
void si446x_irq_config(uint8_t int_global, uint8_t int_ph, uint8_t int_modem, uint8_t int_chip)
{	
	si446x_set_property(0x01,0x01,0x00,int_global);		//配置'INT_CTL_ENABLE',全局中断配置
	si446x_set_property(0x01,0x01,0x01,int_ph);				//配置'INT_CTL_PH_ENABLE'，PH中断配置
	si446x_set_property(0x01,0x01,0x02,int_modem);		//配置'INT_CTL_MODEM_ENABLE'，MODEM中断配置
	si446x_set_property(0x01,0x01,0x03,int_chip);			//配置'INT_CTL_CHIP_ENABLE'，CHIP中断配置
}
/*******************************************************************************************************************	
Function Name:			si446x_wutstate_config
Brief Comment:			Configure WUT function of Si4438
Input Parameter:		wut_state@ enable or disable functionality of Si4438
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void si446x_wutstate_config(uint8_t wut_state)
{	
	if(wut_state==1)
		{
			si446x_set_property(0x00,0x01,0x01,0x01);		//配置'GLOBAL_CLK_CFG',使能CLK_32K_SEL为内部RC Oscillator
			si446x_set_property(0x00,0x01,0x04,0x42);		//配置'GLOBAL_WUT_CONFIG ',使能RC_LDC, ENABLE_WUT			
		}

	else
		{
			si446x_set_property(0x00,0x01,0x01,0x00);		//配置'GLOBAL_CLK_CFG',禁止CLK_32K_SEL
			si446x_set_property(0x00,0x01,0x04,0x00);		//配置'GLOBAL_WUT_CONFIG ',禁止RC_LDC, ENABLE_WUT
		}
}
/*******************************************************************************************************************	
Function Name:			si446x_wutperiod_config
Brief Comment:			Configure WUT of Si4438
Input Parameter:		wut_period@ Sleep Time of Si4438
										ldc_period@ Active time during LDC Period
Return Value:				None
Notes:							wut_period=WUT_M*(4*2^WUT_R/32768)
										ldc_period=WUT_LDC*(4*2^WUT_R/32768)
										WUT_R=3, max wut_period=63999.023ms
														 min ldc_period=0.977ms
														 max ldc_period=249.023ms
*******************************************************************************************************************/
void si446x_wutperiod_config(uint16_t wut_period, uint16_t ldc_period)
{
	uint16_t wut_m;
	uint8_t wut_ldc;	

	wut_m=wut_period*32768/1000/(4*8);
	wut_ldc=(uint8_t)(ldc_period*32768/1000/(4*8));
/*
	si446x_set_property(0x00,0x01,0x05,((uint8_t)((wut_m&0xFF00)>>8)));	//写property 0x0005, WUT_M[15:8]
	si446x_set_property(0x00,0x01,0x06,((uint8_t)(wut_m&0xFF)));					//写property 0x0006, WUT_M[7:0]
	si446x_set_property(0x00,0x01,0x07,0x63);													//在检测到前导或同步字后等待2*LDC,LDC超时后进入sleep,WUT_R=3
	si446x_set_property(0x00,0x01,0x08,wut_ldc);											//写property 0x0008, WUT_LDC
*/
	si446x_set_property(0x00,0x01,0x05,0xA0);	//写property 0x0005, WUT_M[15:8]
	si446x_set_property(0x00,0x01,0x06,0x00);					//写property 0x0006, WUT_M[7:0]
	si446x_set_property(0x00,0x01,0x07,0x21);													//在检测到前导或同步字后等待2*LDC,LDC超时后进入sleep,WUT_R=3
	si446x_set_property(0x00,0x01,0x08,0x9C);											//写property 0x0008, WUT_LDC

}







