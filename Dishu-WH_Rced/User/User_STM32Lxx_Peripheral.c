/****************************************************************************************************************************
File Name:	User_STM32Lxx_Peripheral.c
Author:			Owen.Chen
Date:				Jun 29th, 2015
Comment:		User defined STM32Lxx Peripheral Operation Functions
****************************************************************************************************************************/	
#include "platform.h"
#include "board_config.h"
#include "app_state.h"
#include <string.h>

extern uint8_t __IO 	userkey_irq_flag;								
extern uint8_t __IO	si4438_irq_flag;									
extern uint8_t __IO 	tim7_irq_flag;		

extern uint8_t __IO  manual_mode_flag;								
extern uint8_t __IO  point_mode_flag;								
extern uint8_t __IO  halfauto_mode_flag;		

extern uint8_t __IO  low_valtage_flag;	


/**************************************************	RCC Configuration	*************************************************/
/*******************************************************************************************************************	
Function Name:			RCC_Config
Brief Comment:			User Defined RCC Configuration for STM32L1xx
Input Parameter:		None 
Return Value:				None 
Notes:							Used as RCC Configuration Replacement of Function SetSysClock()					
*******************************************************************************************************************/
void RCC_Config(void)
{
	RCC_DeInit();																								//时钟设置恢复默认
	RCC_HSEConfig(RCC_HSE_ON);																	//开启外部高速时钟HSE
	while(!RCC_WaitForHSEStartUp());														//等待HSE稳定
	RCC_HCLKConfig(RCC_SYSCLK_Div1);														//设置AHB时钟分频因子
	RCC_PCLK1Config(RCC_HCLK_Div1);															//设置APB1时钟分频因子
	RCC_PCLK2Config(RCC_HCLK_Div1);															//设置APB2时钟分频因子
	RCC_PLLConfig(RCC_PLLSource_HSE,RCC_PLLMul_4,RCC_PLLDiv_2);	//设置PLL=HSE*4/2
	RCC_PLLCmd(ENABLE);																					//使能PLL
	while(!RCC_GetFlagStatus(RCC_FLAG_PLLRDY));									//等待PLL稳定
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);									//设置PLL为系统时钟
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS)!=0x0C);				//等待系统时钟设置生效
}
/*******************************************************************************************************************	
Function Name:			RCC_FreqTest
Brief Comment:			Freqency of RCC elements test using MCO functionality
Input Parameter:		RCC_MCOSource@ RCC_MCOSource_ MSI,HSI,LSI,LSE,HSE,PLL,SYSCLK, 
										RCC_MCODiv@ RCC_MCODiv_ 1, 2, 4, 8, or 16 
Return Value:				RCC_MCODiv@ RCC_MCODiv_ 1, 2, 4, 8, or 16 
Notes:							MCO output at PA8 with AF setting					
*******************************************************************************************************************/
void RCC_FreqTest(uint8_t RCC_MCOSource, uint8_t RCC_MCODiv)
{
	GPIO_InitTypeDef	GPIO_InitStructure; 										//定义GPIO初始化结构体变量
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);				//开启GPIO时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	RCC_MCOConfig(RCC_MCOSource, RCC_MCODiv);
}


/********************************************	Low Power Mode Configuration ********************************************/
/*******************************************************************************************************************	
Function Name:			Enter_LPMode
Brief Comment:			Low Power Mode Configuration@ Standby Mode
Input Parameter:		None 
Return Value:				None 
Notes:							Enters stop mode, current consumption of MCU is about 500nA					
*******************************************************************************************************************/
void Enter_StopMode(void)
{
	/* Disable ADC */
	ADC1->CR2 &= (uint32_t)(~ADC_CR2_ADON);	
  /* Disable the SysTick timer */
  SysTick->CTRL &= (~SysTick_CTRL_ENABLE);
	/* Enable Power Interface clock */
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;	
  /* configure MCU to go in stop/standby mode after WFI instruction and not in sleep */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;	
	/* configure MCU to go in stop mode after WFI instruction and not in standby */
  PWR->CR &= (~PWR_CR_PDDS);
  /* Disable VREFINT to save current */
  PWR->CR |= PWR_CR_ULP;
  /* Disable PVDE to save current */
  PWR->CR &= (~PWR_CR_PVDE);
	/* Enable sleep on exit to come back to stop after handling RTC wake up if enabled*/
  //SCB->SCR |= SCB_SCR_SLEEPONEXIT;
	//使能STOP状态下的仿真器连接
	DBGMCU_Config(DBGMCU_STOP, ENABLE); 
	PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
}
/*******************************************************************************************************************	
Function Name:			Enter_StandbyMode
Brief Comment:			Low Power Mode Configuration@ Standby Mode
Input Parameter:		None 
Return Value:				None 
Notes:							Enters standby mode, current consumption of MCU is about 350nA					
*******************************************************************************************************************/
void Enter_StandbyMode(void)
{
	DBGMCU_Config(DBGMCU_STANDBY, ENABLE); //使能standby状态下的仿真器连接
	PWR_WakeUpPinCmd(PWR_WakeUpPin_1, ENABLE);
  EXTI_ClearITPendingBit(EXTI_Line20);	 //清RTC映射的外部唤醒中断
	PWR_ClearFlag(PWR_FLAG_WU);						 //清唤醒标志
	PWR_ClearFlag(PWR_FLAG_SB);						 //清standby标志
	PWR_FastWakeUpCmd(DISABLE);						 //禁止快速唤醒，等待VREFINT就绪后退出standby模式
	PWR_UltraLowPowerCmd(ENABLE);					 //standby模式下关闭VREFINT
	PWR_EnterSTANDBYMode();								 //进入standby模式，MCU功耗350nA
}


/*************************************************	GPIO Configuration ************************************************/
/*******************************************************************************************************************	
Function Name:			GPIOConfig_ActiveMode
Brief Comment:			User Defined GPIO Configuration In Active Mode
Input Parameter:		None 
Return Value:				None 
Notes:							None					
*******************************************************************************************************************/
void GPIOConfig_ActiveMode(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;																//定义GPIO初始化结构体变量

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB
												|RCC_AHBPeriph_GPIOC,ENABLE);								  //开启GPIO时钟

	/******************************************	LED灯 GPIO初始化	*******************************************/
	GPIO_InitStructure.GPIO_Pin=LED_G_CTR|LED_R_CTR;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;												
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											
  GPIO_Init(GPIOB, &GPIO_InitStructure);	 
	LED_G_OFF;LED_R_OFF;

	/*******************************************	BEEP GPIO初始化	*******************************************/
	GPIO_InitStructure.GPIO_Pin=BP_PWM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;												
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource8,GPIO_AF_TIM4);

	/*******************************************  按键 GPIO初始化  ******************************************/
	GPIO_InitStructure.GPIO_Pin=KEY_SW|KEY_USER;																						
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;											
  GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	/******************************************  SI4438 GPIO初始化  *****************************************/	
	GPIO_InitStructure.GPIO_Pin = SI_NSS;																//SI4438 SPI使能
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;												//输出模式
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											//无上下拉模式
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	SI_NSS_1;																														//SI4438 SPI口禁止

	GPIO_InitStructure.GPIO_Pin = SI_SDN;																//SI4438 SDN脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;												//输出模式
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											//无上下拉模式
  GPIO_Init(GPIOA, &GPIO_InitStructure);				
	SI_SHUTDWN;																													//SI4438 休眠	

	GPIO_InitStructure.GPIO_Pin = SI_MOSI|SI_MISO|SI_SCLK;							//SI4438 SPI口, SPI1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;												//复用模式	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=SI_GPIO1;																//SI4438 GPIO0-1引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;													//输人模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											//下拉模式
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=SI_GPIO3;																//SI4438 GPIO0-1引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;													//输人模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											//下拉模式
	GPIO_Init(GPIOA,&GPIO_InitStructure);

}

/*******************************************************************************************************************	
Function Name:			GPIOConfig_PWRON_StopMode
Brief Comment:			User Defined GPIO Configuration In Stop Mode
Input Parameter:		None 
Return Value:				None 
Notes:							None					
*******************************************************************************************************************/
void GPIOConfig_PWROFF_StopMode(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;																//定义GPIO初始化结构体变量

	RCC_AHBPeriphClockCmd(	RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|
													RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOH,ENABLE);								  //开启GPIO时钟

	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All&//(~KB_KEY)	
		//																				&(~SI_MOSI)&(~SI_SCLK)&(~SI_NSS)&(~SI_SDN)
			//																			&(~ST_SWDIO)&(~ST_SWCLK);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin=SI_MOSI|SI_SCLK;													
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;													
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;												
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;											
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	SI_MOSI_1;
	SI_SCLK_1;
	SI_SHUTDWN;
	SI_NSS_1;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;//&(~CH455_SDA)&(~CH455_SCL);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOH, &GPIO_InitStructure);
}

/*******************************************************************************************************************	
Function Name:			GPIOConfig_PWRON_StopMode
Brief Comment:			User Defined GPIO Configuration In Stop Mode
Input Parameter:		None 
Return Value:				None 
Notes:							None					
*******************************************************************************************************************/
void GPIOConfig_PWRON_StopMode(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure; 															//定义GPIO初始化结构体变量
}

/*******************************************************************************************************************	
Function Name:			GPIOConfig_PWRON_StopMode
Brief Comment:			User Defined GPIO Configuration In Stop Mode
Input Parameter:		None 
Return Value:				None 
Notes:							None					
*******************************************************************************************************************/
void GPIOConfig_PWRON_ExitFromStopMode(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;																//定义GPIO初始化结构体变量

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB
												|RCC_AHBPeriph_GPIOC,ENABLE);								  //开启GPIO时钟

}

/*************************************************	NVIC Configuration ************************************************/
/*******************************************************************************************************************	
Function Name:			NVIC_Config
Brief Comment:			User Defined NVIC Configuration
Input Parameter:		None 
Return Value:				None 
Notes:							None
*******************************************************************************************************************/
void NVIC_Config(uint8_t iqr_channel, uint8_t state)
{
	NVIC_InitTypeDef	 NVIC_InitStructure;																	//定义NVIC初始化结构体变量

	switch(iqr_channel)
		{
			case 	SWITCH_KEY_IRQn:
						NVIC_InitStructure.NVIC_IRQChannel=SWITCH_KEY_IRQn; 																							
						NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;					
						NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00; 								
						NVIC_InitStructure.NVIC_IRQChannelCmd=state;
						NVIC_Init(&NVIC_InitStructure);
						EXTI_ClearITPendingBit(EXTI_SWITCH_KEY);
						if(state) EXTI->IMR |= EXTI_SWITCH_KEY;
						else			EXTI->IMR &= ~EXTI_SWITCH_KEY;
						userkey_irq_flag=0; 
			break;

			case SI_GPIO1_IRQn:
						NVIC_InitStructure.NVIC_IRQChannel=SI_GPIO1_IRQn; 																							
						NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;					
						NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; 								
						NVIC_InitStructure.NVIC_IRQChannelCmd=state;
						NVIC_Init(&NVIC_InitStructure);
						EXTI_ClearITPendingBit(EXTI_SI_GPIO1);
						if(state) EXTI->IMR |= EXTI_SI_GPIO1;
						else			EXTI->IMR &= ~EXTI_SI_GPIO1;
						si4438_irq_flag=0; 
			break;

			case TIM7_IRQn:
					if(state==ENABLE)
						NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn;										//设置TIM4中断Channel										
  					NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;			//抢占优先级为1
  					NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;							//响应优先级为0
  					NVIC_InitStructure.NVIC_IRQChannelCmd = state;
						NVIC_Init(&NVIC_InitStructure);

						TIM_ClearITPendingBit(TIM7,TIM_FLAG_Update);
						TIM_ITConfig(TIM7,TIM_IT_Update,state);												//开启TIM7溢出中断
						TIM_Cmd(TIM7, state);
						tim7_irq_flag=0;
			break;
		}
}

/*************************************************	EXTI Configuration ************************************************/
/*******************************************************************************************************************	
Function Name:			EXTI_NVIC_Config
Brief Comment:			User Defined EXTI Configuration
Input Parameter:		None 
Return Value:				None 
Notes:							NVIC_PriorityGroup_2 sets 2bits Preemption Priority and 2bits SubPriority;
										EXTI0~EXTI4 has seperate interrupt channel; 
										EXTI5~EXTI9\EXTI10~EXTI15 share one interrupt channel; 
*******************************************************************************************************************/
void EXTI_Config(void)
{
	EXTI_InitTypeDef	 EXTI_InitStructure;													//定义EXTI初始化结构体变量
	NVIC_InitTypeDef	 NVIC_InitStructure;													//定义NVIC初始化结构体变量

	EXTI_InitStructure.EXTI_Line= EXTI_SWITCH_KEY;									//用户按键中断
  EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling; 					//下降沿触发中断 
  EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI->IMR &= ~ (EXTI_SWITCH_KEY);																//屏蔽用户按键中断

	EXTI_InitStructure.EXTI_Line= EXTI_SI_GPIO1;										//SI448 GPIO1中断
  EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling; 					//下降沿触发中断 
  EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	EXTI->IMR &= ~ (EXTI_SI_GPIO1);																		//屏蔽SI448 GPIO1中断
	
	EXTI_ClearITPendingBit(EXTI_SWITCH_KEY|EXTI_SI_GPIO1);						//清除使能中断的pending

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);						//使能SYSCFG时钟

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);			//连接EXTI0到PA口,用户按键
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2);			//连接EXTI3到PB口,SI_GPIO1
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 									//中断优先级分组值为2 */
}

/*******************************************************************************************************************	
Function Name:			TIM6_Delay_Config
Brief Comment:			User Defined TIM6 Configuration for 1ms precise delay
Input Parameter:		None
Return Value:				None
Notes:							TIM6 userd for 1ms precise delay
										time=((TIM_Prescaler+1)/PCLK2)*(TIM_Period+1)
*******************************************************************************************************************/
void TIM6_Delay_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Prescaler=3200-1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0;	
	TIM_TimeBaseInitStructure.TIM_Period=9;	//time=((TIM_Prescaler+1)/PCLK2)*(TIM_Period+1)=(3200/32000000)*10=1ms
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_GenerateEvent(TIM6,TIM_EventSource_Update);
	TIM_Cmd(TIM6, DISABLE);
}
/*******************************************************************************************************************	
Function Name:			Delay_TIM6_1ms
Brief Comment:			1ms Delay using TIM6
Input Parameter:		time@ time*1 equals actual delay(ms)
Return Value:				None
Notes:							None				
*******************************************************************************************************************/
void Delay_TIM6_1ms(uint32_t time)
{
		while(time--)
		{
			TIM_Cmd(TIM6,ENABLE);	
			while(!TIM_GetFlagStatus(TIM6,TIM_FLAG_Update));	
			TIM_Cmd(TIM6,DISABLE);	
			TIM_ClearITPendingBit(TIM6,TIM_FLAG_Update);
		}
}
/*******************************************************************************************************************	
Function Name:			TIM4_Beep_PWMConfig
Brief Comment:			1ms Delay using TIM6
Input Parameter:		time@ time*1 equals actual delay(ms)
Return Value:				None
Notes:							None				
*******************************************************************************************************************/
void TIM4_Beep_PWMConfig(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);

	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Prescaler=31;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0;	
	TIM_TimeBaseInitStructure.TIM_Period=0x00AC*2-1;		 						
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0x00AC;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
 	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4,ENABLE);

	TIM_Cmd(TIM4, DISABLE);
}
/*******************************************************************************************************************	
Function Name:			TIM7_Irq_Config
Brief Comment:			User Defined TIM7 Configuration for TIM IRQ
Input Parameter:		time@ time to generate interrupt in seconds
Return Value:				None
Notes:							time=((TIM_Prescaler+1)/PCLK2)*(TIM_Period+1)										
*******************************************************************************************************************/
void TIM7_Irq_Config(uint8_t time)		//系统计时定时器，计时单位为秒
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Prescaler=32000-1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0;	
	TIM_TimeBaseInitStructure.TIM_Period=time*1000-1;					
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);

	TIM_ClearITPendingBit(TIM7,TIM_FLAG_Update);
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	TIM_GenerateEvent(TIM7,TIM_EventSource_Update);
	TIM_Cmd(TIM7, DISABLE);
}

/*******************************************************************************************************************	
Function Name:			TIM7_Irq_Config
Brief Comment:			User Defined TIM7 Configuration for TIM IRQ
Input Parameter:		time@ time to generate interrupt in seconds
Return Value:				None
Notes:							time=((TIM_Prescaler+1)/PCLK2)*(TIM_Period+1)										
*******************************************************************************************************************/
void TIM7_SiTrans_Irq_Config(uint16_t time)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Prescaler=32000-1;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=0;	
	TIM_TimeBaseInitStructure.TIM_Period=time-1;					
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStructure);

	TIM_ClearITPendingBit(TIM7,TIM_FLAG_Update);
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
	TIM_GenerateEvent(TIM7,TIM_EventSource_Update);
	TIM_Cmd(TIM7, DISABLE);
}

/**************************************************	ADC Configuration	*************************************************/
/*******************************************************************************************************************	
Function Name:			ADC_Config
Brief Comment:			Configuration of ADC
Input Parameter:		None
Return Value:				None
Notes:							ADC Clock source is HSI, enables HSI first before configure ADC 
*******************************************************************************************************************/
void ADC_Config(uint8_t ADC_Channel)
{
	ADC_InitTypeDef ADC_InitStructure;															//定义ADC初始化结构体变量
	//ADC_CommonInitTypeDef ADC_CommonInitStructure;
	RCC_HSICmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
  ADC_RegularChannelConfig(ADC1, ADC_Channel, 1,ADC_SampleTime_4Cycles);// ADC_SampleTime_384Cycles);
	
	ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);
  /* Enable ADC1 Power Down during Delay */
  ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
	
	ADC_Cmd(ADC1,ENABLE);
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET)
  {
  }

}
/*******************************************************************************************************************	
Function Name:			Battery_Meas
Brief Comment:			Measure battery voltage
Input Parameter:		None
Return Value:				Voltage value of battery
Notes:							Since input voltage should be in 0~Vref, Max value measured is 3.300V
*******************************************************************************************************************/
uint16_t Battery_Meas(void)
{
	uint16_t adc_value;																	//存放电压采样值
	
	ADC_SoftwareStartConv(ADC1);
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
  	{
  	}
	adc_value=ADC_GetConversionValue(ADC1);
	adc_value=(3300*adc_value)/4095;
	
	return adc_value;
}


/************************************************	UART Configuration	*************************************************/
/*******************************************************************************************************************	
Function Name:			UART_Config
Brief Comment:			UART User Configuration
Input Parameter:		USARTx@ UART port need to be configured, could be USART1 or USART2
Return Value:				None 
Notes:							None
*******************************************************************************************************************/
void UART_Config(USART_TypeDef* USARTx)
{
  USART_InitTypeDef USART_InitStructure;
	
	if(USARTx==USART1)
		{  
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
  		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
		}
	else if(USARTx==USART2)
		{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
			GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
		}
	
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTx, &USART_InitStructure);

	USART_ClearFlag(USARTx,USART_FLAG_TC);
	USART_ClearFlag(USARTx,USART_FLAG_RXNE);

  USART_Cmd(USARTx, ENABLE);
	USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
}
/*******************************************************************************************************************	
Function Name:			UART_ByteSend
Brief Comment:			Send One Byte Through UART
Input Parameter:		USARTx@ UART port need to be configured, could be USART1 or USART2
										data@ Data need to be sent
Return Value:				None 
Notes:							None
*******************************************************************************************************************/
void UART_ByteSend(USART_TypeDef* USARTx, uint8_t data)
{
	USART_SendData(USARTx,data);
	while(!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
}
/*******************************************************************************************************************	
Function Name:			UART_StringSend
Brief Comment:			Send String Through UART
Input Parameter:		USARTx@ UART port need to be configured, could be USART1 or USART2
										length@ Length of string need to be sent
										pData@ Pointer of data array used to store string
Return Value:				None 
Notes:							None
*******************************************************************************************************************/
void UART_StringSend(USART_TypeDef* USARTx, uint8_t length, uint8_t *pData)
{
	uint8_t i;
	USART_ClearFlag(USARTx,USART_FLAG_TC);
	for(i=0;i<length;i++)
		{
			USART_SendData(USARTx,*pData++);
			while(!USART_GetFlagStatus(USARTx,USART_FLAG_TXE));
		}	
	while(!USART_GetFlagStatus(USARTx,USART_FLAG_TC));
}
/*******************************************************************************************************************	
Function Name:			UART_ByteRead
Brief Comment:			Receive One Byte Through UART
Input Parameter:		USARTx@ UART port need to be configured, could be USART1 or USART2									
Return Value:				data@ Data received through UART
Notes:							None
*******************************************************************************************************************/
uint8_t UART_ByteRead(USART_TypeDef* USARTx)
{
	uint8_t data;
	while(!USART_GetFlagStatus(USARTx,USART_FLAG_RXNE));
	data=USART_ReceiveData(USARTx);
	USART_ClearFlag(USARTx,USART_FLAG_RXNE);
	return data;
}
/*******************************************************************************************************************	
Function Name:			UART_StringRead
Brief Comment:			Receive String Through UART
Input Parameter:		USARTx@ UART port need to be configured, could be USART1 or USART2
										length@ Length of string need to be received
										pData@ Pointer of data array used to store received string
Return Value:				None 
Notes:							None
*******************************************************************************************************************/
void UART_StringRead(USART_TypeDef* USARTx, uint8_t length, uint8_t *pData)
{
	uint8_t i;
	for(i=0;i<length;i++)
		{
			while(!USART_GetFlagStatus(USARTx,USART_FLAG_RXNE));
			*pData++=USART_ReceiveData(USARTx);
			USART_ClearFlag(USARTx,USART_FLAG_RXNE);
		}	
}

/**************************************************	SPI Configuration	*************************************************/
/*******************************************************************************************************************	
Function Name:			SPIConfig
Brief Comment:			SPI User Configuration
Input Parameter:		SPIx@ SPI port need to be configured, could be SPI1 or SPI2
Return Value:				None 
Notes:							None
*******************************************************************************************************************/	
void SPI_Config(SPI_TypeDef* SPIx)
	{
		SPI_InitTypeDef 	SPI_InitStructure;																//定义SPI初始化结构体变量
		
		if(SPIx==SPI1)
			{
				RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);								//开启SPI1时钟
				GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);
				GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
				GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
			}
		else
			{
				RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);								//开启SPI2时钟
				GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
				GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
				GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
			}
	
		SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
		SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
		SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
		SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
		SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
		SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;			//F=32MHz/2
		SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
		SPI_InitStructure.SPI_CRCPolynomial = 7;
		SPI_Init(SPIx, &SPI_InitStructure);
		SPI_SSOutputCmd(SPIx, DISABLE);
		SPI_Cmd(SPIx, ENABLE);
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY)); 									//等待SPI空闲
	}

/*******************************************************************************************************************	
Function Name:			SPI_ByteWR
Brief Comment:			SPI Byte Write&Read Fuction
Input Parameter:		SPIx@ SPI port need to be configured, could be SPI1 or SPI2
										data@	data need to be sent to slave
Return Value:				data@	data read back from slave  
Notes:							None
*******************************************************************************************************************/	
uint8_t SPI_ByteWR(SPI_TypeDef* SPIx,uint8_t data)
{
	while(!SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE))
	{
	}
		SPI_I2S_SendData(SPIx,data);
	while(!SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE))
	{
	}
		data=SPI_I2S_ReceiveData(SPIx);
	return data;
}


/**************************************************	I2C Configuration	*************************************************/
/*******************************************************************************************************************	
Function Name:			I2C_Config
Brief Comment:			I2C User Configuration
Input Parameter:		I2Cx@ I2C port need to be configured, could be SPI1 or SPI2
Return Value:				I2C_ERROR@ equals '0', I2C configuration falled
										I2C_SUCCESS@ equals '1', I2C configuration succeed
Notes:							Delay needed for waitting bus exit from busy status caused by glitch
*******************************************************************************************************************/
ErrorStatus	I2C_Config(I2C_TypeDef* I2Cx)
{
	GPIO_InitTypeDef	GPIO_InitStructure;																		//定义GPIO初始化结构体变量
	I2C_InitTypeDef		I2C_InitStructure;																		//定义I2C初始化结构体变量
	
	uint32_t op_status=I2C_TIMEOUT;	
	uint8_t	i;

	if(I2Cx==I2C1)																													//I2C1初始化
		{
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;								//PB6 PB7
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 		 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
			GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
			GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
 		 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);									//开启I2C1时钟
			GPIO_PinAFConfig(GPIOB,GPIO_PinSource7,GPIO_AF_I2C1);
			GPIO_PinAFConfig(GPIOB,GPIO_PinSource6,GPIO_AF_I2C1);	
		}
	else if(I2Cx==I2C2)																											//I2C2初始化
		{
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;							//PB10 PB11
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 		 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
			GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;
			GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;
 		 	GPIO_Init(GPIOB, &GPIO_InitStructure);			
			
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);									//开启I2C2时钟
			GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_I2C2);
			GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_I2C2);	
		}

	for(i=0;i<I2C_TIMEOUT;i++)
		{
			I2C_SoftwareResetCmd(I2Cx,ENABLE);																	//软件复位I2C解决glitch引起总线挂死
			Delay_TIM6_1ms(i);																								
			I2C_SoftwareResetCmd(I2Cx,DISABLE);
			if(!(I2C_GetFlagStatus(I2Cx,I2C_FLAG_BUSY))) break;
		}
	
	if(i==I2C_TIMEOUT)	return	I2C_ERROR;
		 	
  I2C_InitStructure.I2C_Mode=I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle=I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1=0;
  I2C_InitStructure.I2C_Ack=I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress=I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed=I2C_SPEED_LOW;
	I2C_Init(I2Cx, &I2C_InitStructure);
	I2C_Cmd(I2Cx,ENABLE);
	
	while(op_status)
	{
		op_status--;
		if(!(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY)))												//等待I2C总线空闲	
		break;
	}
	if(op_status)	return	I2C_SUCCESS;
	else					return	I2C_ERROR;	
}
/******************************************************************************************************************* 
Function Name:			I2C_ByteSend
Brief Comment:			Send one byte through I2C Interface 
Input Parameter:		I2Cx@ I2C Port Number
										addr@ I2C slave address
										data@ Data need to be sent
Return Value: 			None
Notes:							None
*******************************************************************************************************************/
void I2C_ByteSend(I2C_TypeDef* I2Cx,uint8_t addr,	uint8_t data)
{
	I2C_GenerateSTART(I2Cx,ENABLE);																						//主机产生起始位
	while (!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)); 							//等待EV5
		
	I2C_Send7bitAddress(I2Cx,addr,I2C_Direction_Transmitter);									//主机发送从机地址并定义传输方向为发送
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	//等待EV6
				
	I2C_SendData(I2Cx,data); 																									//主机发送一个字节数据
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 					//等待EV8_2
			
	I2C_GenerateSTOP(I2Cx,ENABLE);																						//主机发送停止位
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));														//等待总线释放
}
/******************************************************************************************************************* 
Function Name:			I2C_StringSend
Brief Comment:			String sent through I2C Interface 
Input Parameter:		I2Cx@ I2C Port Number
										addr@ I2C slave address
										length@ Number of byte need to sent
										pData@ Pointer of array to store data needed to be sent
Return Value: 			None
Notes:							None
*******************************************************************************************************************/
void I2C_StringSend(I2C_TypeDef* I2Cx,uint8_t addr,	uint8_t length,uint8_t *pData)
{
	uint8_t i=0;
	I2C_GenerateSTART(I2Cx,ENABLE);
	while (!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)); 							//等待EV5
		
	I2C_Send7bitAddress(I2Cx,addr,I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));	//等待EV6
	
	for(i=0;i<length;i++)
	{
		I2C_SendData(I2Cx,*pData); 
		pData++;
		if(i==(length-1))
			while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 			//等待EV8_2,传输最后一个字节
		else
			while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTING)); 			//等待EV8
	}
	
	I2C_GenerateSTOP(I2Cx,ENABLE);
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
}

/******************************************************************************************************************* 
Function Name:			I2C_StringSend
Brief Comment:			String read through I2C Interface 
Input Parameter:		I2Cx@ I2C Port Number
										addr@ I2C slave address
										length@ Number of byte need to read
										pData@ Pointer of array to store read back data
Return Value: 			None
Notes:							None
*******************************************************************************************************************/
uint8_t I2C_Byte_Read(I2C_TypeDef* I2Cx,uint8_t addr)
{
	uint8_t dat;
	
	I2C_GenerateSTART(I2Cx,ENABLE);
	while (!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)); 								//等待EV5
		
	I2C_Send7bitAddress(I2Cx,addr,I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));				//等待EV6

	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED)); 							//等待EV7
	dat=I2C_ReceiveData(I2Cx); 

				
	I2C_GenerateSTOP(I2Cx,ENABLE);
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);	

	return dat;
}

/******************************************************************************************************************* 
Function Name:			I2C_StringSend
Brief Comment:			String read through I2C Interface 
Input Parameter:		I2Cx@ I2C Port Number
										addr@ I2C slave address
										length@ Number of byte need to read
										pData@ Pointer of array to store read back data
Return Value: 			None
Notes:							None
*******************************************************************************************************************/
void I2C_Read(I2C_TypeDef* I2Cx,uint8_t addr,uint8_t length,uint8_t *pData)
{
	uint8_t i=0;
	
	I2C_GenerateSTART(I2Cx,ENABLE);
	while (!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_MODE_SELECT)); 								//等待EV5
		
	I2C_Send7bitAddress(I2Cx,addr,I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));				//等待EV6

	for(i=0;i<length;i++)
	{
		while(!I2C_CheckEvent(I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED)); 							//等待EV7
		*pData++=I2C_ReceiveData(I2Cx); 
	}
				
	I2C_GenerateSTOP(I2Cx,ENABLE);
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
	I2C_AcknowledgeConfig(I2Cx,ENABLE);	
}


/**************************************************	EEPROM Operation	*************************************************/
/*******************************************************************************************************************	
Function Name:			EEPROM_ByteWR
Brief Comment:			Write single byte to EEPROM
Input Parameter:		addr@ Specifies Address of EEPROM need to be written
										data@ Data need to be written
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void EEPROM_ByteWR(uint32_t addr,uint8_t data)
{
	DATA_EEPROM_Unlock();
	while(!(DATA_EEPROM_ProgramByte(addr,data)==FLASH_COMPLETE));
	DATA_EEPROM_Lock();
}
/*******************************************************************************************************************	
Function Name:			EEPROM_StringWR
Brief Comment:			Write string to EEPROM
Input Parameter:		addr@ Specifies start address of EEPROM need to be written
										length@ Length of data need to be written
										pData@ Pointer of data array need to be written
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void EEPROM_StringWR(uint32_t addr,uint8_t length,uint8_t *pData)
{
	uint8_t i;
	DATA_EEPROM_Unlock();
	for(i=0;i<length;i++)
		{
			while(!(DATA_EEPROM_ProgramByte(addr++,*pData)==FLASH_COMPLETE));
			pData++;
		}
	DATA_EEPROM_Lock();	
}
/*******************************************************************************************************************	
Function Name:			EEPROM_ByteRD
Brief Comment:			Read single byte from EEPROM
Input Parameter:		addr@ Specifies Address of EEPROM need to be read
Return Value:				data@ Read back data
Notes:							None
*******************************************************************************************************************/
uint8_t EEPROM_ByteRD(uint32_t addr)
{
	uint8_t data;
	data=*(uint8_t *)addr;
	return data;
}
/*******************************************************************************************************************	
Function Name:			EEPROM_StringRD
Brief Comment:			Read string from EEPROM
Input Parameter:		addr@ Specifies start address of EEPROM need to be read
										length@ Length of data need to be read
										pData@ Pointer of data array used to store read back data
Return Value:				None
Notes:							None
*******************************************************************************************************************/
void EEPROM_StringRD(uint32_t addr,uint8_t length,uint8_t *pData)
{
	uint8_t i;
	for(i=0;i<length;i++)
		{
			*pData++=*(uint8_t *)addr;
			addr++;
		}
}
/*******************************************************************************************************************	
Function Name:			RTC_LSIWUConfig
Brief Comment:			Using RTC Wakeup function to wake up MCU from low power mode with specified time interval
Input Parameter:		wu_interval@ Specifies time interval to wake up MCU from low power mode
Return Value:				None
Notes:							wu_interval should be 1-28, meaning wake up time interval is between 1s~28s
*******************************************************************************************************************/
void RTC_LSIWUConfig(uint8_t wu_interval)
{
	RTC_InitTypeDef 	RTC_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
  EXTI_InitTypeDef EXTI_InitStructure;

	if(wu_interval>=28) wu_interval=28;
	
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

	/* LSI used as RTC source clock */
	/* The RTC Clock may varies due to LSI frequency dispersion. */   
  /* Enable the LSI OSC */ 
  RCC_LSICmd(ENABLE);

  /* Wait till LSI is ready */  
  while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
   
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();

  /* Calendar Configuration */
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv	=  0x120*wu_interval; /* (37KHz / 128) - 1 = 0x120*/
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure);  

  /* EXTI configuration *******************************************************/
  EXTI_ClearITPendingBit(EXTI_Line20);
  EXTI_InitStructure.EXTI_Line = EXTI_Line20;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  /* Enable the RTC Wakeup Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  

  /* Configure the RTC WakeUp Clock source: CK_SPRE (1Hz) */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
  RTC_SetWakeUpCounter(0x0);

  /* Enable the RTC Wakeup Interrupt */
  RTC_ITConfig(RTC_IT_WUT, ENABLE);
  
  /* Enable Wakeup Counter */
  RTC_WakeUpCmd(ENABLE);
}

void RTC_LSIDisableConfig(void)
{
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

  /* Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);

	RTC_DeInit();

}


/**********************************************	General Purpose Function	*********************************************/
/******************************************************************************************************************* 
Function Name:			Delay_General
Brief Comment:			General Purpose Delay Function
Input Parameter:		delay@ Delay time coefficent
Return Value: 			None
Notes:							None
*******************************************************************************************************************/
void Delay_General(uint32_t delay)
{
	while(delay)
		delay--;
}

/*****************************************	End of STM32L1xx_StdPeriph_Driver.c	***************************************/
