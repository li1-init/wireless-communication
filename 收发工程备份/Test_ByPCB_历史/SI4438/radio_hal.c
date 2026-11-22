/*!
 * File:
 *  radio_hal.c
 *
 * Description:
 *  This file contains RADIO HAL.
 *
 * Silicon Laboratories Confidential
 * Copyright 2011 Silicon Laboratories, Inc.
 */

#include "bsp.h"


void radio_hal_AssertShutdown(void)
{
	SI_SHUTDWN;
}

void radio_hal_DeassertShutdown(void)
{
	SI_ENABLE;
}

void radio_hal_ClearNsel(void)
{
 SI_NSS_0;
}

void radio_hal_SetNsel(void)
{
	SI_NSS_1;
}

uint8_t radio_hal_NirqLevel(void)
{
	return (uint8_t)SI_GPIO1_STATUS;
}

void radio_hal_SpiWriteByte(uint8_t byteToWrite)
{
	SPI_ByteWR(SI4438_SPI,byteToWrite);
}

uint8_t radio_hal_SpiReadByte(void)
{
	return SPI_ByteWR(SI4438_SPI,0xFF);
}

void radio_hal_SpiWriteData(uint8_t byteCount, uint8_t* pData)
{
 uint8_t i;
 for(i=0;i<byteCount;i++)
 	{
		SPI_ByteWR(SI4438_SPI,*pData);
		pData++;
 	}
}

void radio_hal_SpiReadData(uint8_t byteCount, uint8_t* pData)
{
	uint8_t i;
	for(i=0;i<byteCount;i++)
 		{
	 		*pData=SPI_ByteWR(SI4438_SPI,0xFF);
	 		pData++;
 		}
}


uint8_t radio_hal_Gpio0Level(void)
{
	return 0U;
}

uint8_t radio_hal_Gpio1Level(void)
{
	return SI_GPIO1_STATUS;
}

uint8_t radio_hal_Gpio2Level(void)
{
	return 0U;
}

uint8_t radio_hal_Gpio3Level(void)
{
	return SI_GPIO3_STATUS;
}


