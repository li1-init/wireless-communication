#include "led.h"
#include "board_config.h"
#include "platform.h"
#include "stm32l1xx.h"

static void LED_OutputInit(void)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_40MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

    gpio.GPIO_Pin = LED_B_CTR | LED_G_CTR | LED_Y_CTR | LED_W_CTR;
    GPIO_Init(GPIOA, &gpio);

    gpio.GPIO_Pin = LED_R_CTR | LED_W1_CTR | LED_W2_CTR |
                    LED_W3_CTR | LED_W4_CTR | LED_W5_CTR;
    GPIO_Init(GPIOB, &gpio);
}

void LED_Init(void)
{
    LED_OutputInit();
    LED_AllOff();
}

void LED_Set(led_color_t color, led_state_t state)
{
    uint8_t on = (state == LED_STATE_ON);

    switch (color)
    {
        case LED_RED:
            if (on) { LED_R_ON; } else { LED_R_OFF; }
            break;
        case LED_GREEN:
            if (on) { LED_G_ON; } else { LED_G_OFF; }
            break;
        case LED_BLUE:
            if (on) { LED_B_ON; } else { LED_B_OFF; }
            break;
        case LED_YELLOW:
            if (on) { LED_Y_ON; } else { LED_Y_OFF; }
            break;
        case LED_WHITE:
            if (on) { LED_W_ON; } else { LED_W_OFF; }
            break;
        default:
            break;
    }
}

void LED_SetPosition(uint8_t position, led_state_t state)
{
    uint8_t on = (state == LED_STATE_ON);

    switch (position)
    {
        case 1: if (on) { LED_W1_ON; } else { LED_W1_OFF; } break;
        case 2: if (on) { LED_W2_ON; } else { LED_W2_OFF; } break;
        case 3: if (on) { LED_W3_ON; } else { LED_W3_OFF; } break;
        case 4: if (on) { LED_W4_ON; } else { LED_W4_OFF; } break;
        case 5: if (on) { LED_W5_ON; } else { LED_W5_OFF; } break;
        default: break;
    }
}

void LED_AllOff(void)
{
    LED_R_OFF;
    LED_G_OFF;
    LED_B_OFF;
    LED_Y_OFF;
    LED_W_OFF;
    LED_W1_OFF;
    LED_W2_OFF;
    LED_W3_OFF;
    LED_W4_OFF;
    LED_W5_OFF;
}
/*******************************************************************************************************************
Function Name:			CodeLED_Display
Brief Comment:			User Defined Beep sound configuration
Input Parameter:		bp_time@ sound time
										tone_cnt@ beep tone  
Return Value:				None
Notes:							tone_cnt could be 1 or 2					
*******************************************************************************************************************/
void CodeLED_Display(uint8_t led_code, uint8_t state)
{
	switch(led_code)
		{
			case 1: 
				if(state)	LED_Set(LED_RED, LED_STATE_ON);
				else			LED_Set(LED_RED, LED_STATE_OFF);	break;
			case 2: 
				if(state)	LED_Set(LED_GREEN, LED_STATE_ON);		
				else			LED_Set(LED_GREEN, LED_STATE_OFF);	break;
			case 3: 
				if(state)	LED_Set(LED_BLUE, LED_STATE_ON);	
				else			LED_Set(LED_BLUE, LED_STATE_OFF);	break;
			case 4: 
				if(state) LED_Set(LED_YELLOW, LED_STATE_ON);
				else			LED_Set(LED_YELLOW, LED_STATE_OFF);	break;
			case 5: 
				if(state) LED_Set(LED_WHITE, LED_STATE_ON);	
				else			LED_Set(LED_WHITE, LED_STATE_OFF);	break;		
			default: 								break;
		}
}
/*******************************************************************************************************************	
Function Name:			CodeLED_Display
Brief Comment:			User Defined Beep sound configuration
Input Parameter:		bp_time@ sound time
										tone_cnt@ beep tone  
Return Value:				None
Notes:							tone_cnt could be 1 or 2					
*******************************************************************************************************************/
void WhiteLED_Display(uint8_t led_w, uint8_t state)
{
	switch(led_w)
		{
			case 1: 
				if(state)	LED_SetPosition(1, LED_STATE_ON);		
				else			LED_SetPosition(1, LED_STATE_OFF); 	break;
			case 2: 
				if(state)	LED_SetPosition(2, LED_STATE_ON);		
				else			LED_SetPosition(2, LED_STATE_OFF); 	break;
			case 3: 
				if(state)	LED_SetPosition(3, LED_STATE_ON);		
				else			LED_SetPosition(3, LED_STATE_OFF); 	break;
			case 4: 
				if(state) LED_SetPosition(4, LED_STATE_ON); 	
				else			LED_SetPosition(4, LED_STATE_OFF);		break;
			case 5: 
				if(state) LED_SetPosition(5, LED_STATE_ON); 	
				else			LED_SetPosition(5, LED_STATE_OFF);		break;		
			default: 									break;
		}
}

void LED_Blink(led_color_t color, uint8_t count, uint16_t interval_ms)
{
    uint8_t i;

    for (i = 0U; i < count; ++i)
    {
        LED_Set(color, LED_STATE_ON);
        Delay_TIM6_1ms(interval_ms);
        LED_Set(color, LED_STATE_OFF);
        Delay_TIM6_1ms(interval_ms);
    }
}
