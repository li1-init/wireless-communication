/*!
 * File:
 *  radio_comm.h
 *
 * Description:
 *  This file contains the RADIO communication layer.
 *
 * Silicon Laboratories Confidential
 * Copyright 2012 Silicon Laboratories, Inc.
 */
#include "bsp.h"

__IO uint8_t ctsWentHigh = 0;

/*!
 * Gets a command response from the radio chip
 *
 * @param byteCount     Number of bytes to get from the radio chip
 * @param pData         Pointer to where to put the data
 *
 * @return CTS value
 */
uint8_t radio_comm_GetResp(uint8_t byteCount, uint8_t* pData)
{
	uint8_t ctsVal=0;
	uint16_t errCnt=RADIO_CTS_TIMEOUT;

  while (errCnt != 0)      //wait until radio IC is ready with the data
  	{
    	radio_hal_ClearNsel();
    	radio_hal_SpiWriteByte(0x44);    //read CMD buffer
    	ctsVal = radio_hal_SpiReadByte();
    	if (ctsVal == 0xFF)
    		{
      		if (byteCount)
      			{
        			radio_hal_SpiReadData(byteCount, pData);
      			}
      		radio_hal_SetNsel();
      		break;
    		}
    	radio_hal_SetNsel();
   	  errCnt--;
  	}

  if (ctsVal == 0xFF)
  	{
    	ctsWentHigh = 1;
  	}

 	 return ctsVal;
}

/*!
 * Sends a command to the radio chip
 *
 * @param byteCount     Number of bytes in the command to send to the radio device
 * @param pData         Pointer to the command to send.
 */
void radio_comm_SendCmd(uint8_t byteCount, uint8_t* pData)
{
    while (!ctsWentHigh)
    	{
        radio_comm_PollCTS();
    	}
    radio_hal_ClearNsel();
    radio_hal_SpiWriteData(byteCount, pData);
    radio_hal_SetNsel();
    ctsWentHigh = 0;
}

/*!
 * Gets a command response from the radio chip
 *
 * @param cmd           Command ID
 * @param pollCts       Set to poll CTS
 * @param byteCount     Number of bytes to get from the radio chip.
 * @param pData         Pointer to where to put the data.
 */
void radio_comm_ReadData(uint8_t cmd, uint8_t pollCts, uint8_t byteCount, uint8_t* pData)
{
    if(pollCts)
    {
        while(!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
    radio_hal_ClearNsel();
    radio_hal_SpiWriteByte(cmd);
    radio_hal_SpiReadData(byteCount, pData);
    radio_hal_SetNsel();
    ctsWentHigh = 0;
}


/*!
 * Gets a command response from the radio chip
 *
 * @param cmd           Command ID
 * @param pollCts       Set to poll CTS
 * @param byteCount     Number of bytes to get from the radio chip
 * @param pData         Pointer to where to put the data
 */
void radio_comm_WriteData(uint8_t cmd, uint8_t pollCts, uint8_t byteCount, uint8_t* pData)
{
    if(pollCts)
    {
        while(!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
    radio_hal_ClearNsel();
    radio_hal_SpiWriteByte(cmd);
    radio_hal_SpiWriteData(byteCount, pData);
    radio_hal_SetNsel();
    ctsWentHigh = 0;
}

/*!
 * Waits for CTS to be high
 *
 * @return CTS value
 */
uint8_t radio_comm_PollCTS(void)
{
#ifdef RADIO_USER_CFG_USE_GPIO1_FOR_CTS
    while(!radio_hal_Gpio1Level());
    ctsWentHigh = 1;
    return 0xFF;
#else
    return radio_comm_GetResp(0,0);
#endif
}

/**
 * Clears the CTS state variable.
 */
void radio_comm_ClearCTS()
{
  ctsWentHigh = 0;
}

uint8_t radio_comm_SendCmdGetResp(uint8_t cmdByteCount, uint8_t* pCmdData, uint8_t respByteCount, uint8_t* pRespData)
{
    radio_comm_SendCmd(cmdByteCount, pCmdData);
    return radio_comm_GetResp(respByteCount, pRespData);
}

