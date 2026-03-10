/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    HTTP_Server.c
 * Purpose: HTTP Server example
 *----------------------------------------------------------------------------*/

#include <stdio.h>

#include "main.h"

#include "rl_net.h"                     // Keil.MDK-Pro::Network:CORE

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "adc.h"                  // ::Board Support:A/D Converter
#include "LCD.h"
#include "RTC.h"
 #define MSGQUEUE_OBJECTS 16

// Main stack size must be multiple of 8 Bytes

#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};


uint16_t AD_in          (uint32_t ch);
extern uint8_t  get_button     (void);
extern void     netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len);

extern osThreadId_t TID_Display;
extern osThreadId_t TID_Led;

ADC_HandleTypeDef adchandle; //handler definition

char lcd_text[2][20+1] = { "LCD line 1",
                           "LCD line 2" };

static GPIO_InitTypeDef GPIO_InitStruct;
MSGQUEUE_OBJ_t_LCD msg_enviado;
MSGQUEUE_OBJ_t_LCD msg_recibido;
bool LEDrun = true;
float adc_value;
													 	 
/* Thread IDs */
osThreadId_t TID_Display;
osThreadId_t TID_Led;
													 
void LED_Initialize();
void ADC_Initialize();
void RTC_Init(void);
													 
void BlinkLed (void *arg);
void Display (void *arg);

osStatus_t status;

__NO_RETURN void app_main (void *arg);



/*----------------------------------------------------------------------------
  LECTURA DEL ADC
 *---------------------------------------------------------------------------*/
uint16_t AD_in (uint32_t ch) {
  adc_value = 0;
	if(ch == 0){
			adc_value = ADC_getVoltage(&adchandle, 10)*4096/3.3; 
	}
	
  return (uint16_t)adc_value ;
}



/* IP address change notification */
void netDHCP_Notify (uint32_t if_num, uint8_t option, const uint8_t *val, uint32_t len) {

  (void)if_num;
  (void)val;
  (void)len;

  if (option == NET_DHCP_OPTION_IP_ADDRESS) {
    /* IP address change, trigger LCD update */
    osThreadFlagsSet (TID_Display, 0x01);
  }
}




/*----------------------------------------------------------------------------
  Thread 'Display': LCD display handler
 *---------------------------------------------------------------------------*/
void Display (void *arg) {
  
  LCD_reset();
	LCD_init();
	LCD_Clean();

  while (1) {
    status = osMessageQueueGet(mid_MsgQueue_LCD, &msg_recibido, NULL, osWaitForever);
		if(status == osOK){
			Pintar(msg_recibido);
		}	
		osThreadYield(); 
  }
}


/*----------------------------------------------------------------------------
  Thread 'BlinkLed': Blink the LEDs on an eval board
 *---------------------------------------------------------------------------*/

 void LED_SetOut(uint8_t val){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, (val & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (val & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14,(val & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
 
void BlinkLed (void *arg) {
  uint32_t flags;
  uint32_t cnt = 0;

  (void)arg;

  LEDrun = false;
  while(1) {
		
		flags = osThreadFlagsWait(0x01 | 0x02| 0x04, osFlagsWaitAny, osWaitForever); 

    if (flags) { 
        if (flags & 0x01) {
					
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
					
					for(int i = 0; i < 25; i++){
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
						osDelay(100);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
						osDelay(100);
					}
        }
        if (flags & 0x02) {
					
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
					
          for(int i = 0; i < 20; i++){
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
						osDelay(100);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
						osDelay(100);
					}
        }
				if (flags & 0x04) {
					
			if (LEDrun == true) {
				if(cnt == 0){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
						cnt++;
				}else if(cnt == 1){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
						cnt++;
				}else if(cnt == 2){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_SET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
						cnt++;
				}else if(cnt == 3){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_SET);
						cnt=0;
				}
				osDelay (100);
				osThreadFlagsSet(TID_Led,0x04);
				}
			}
    }
  }
}



/*----------------------------------------------------------------------------
  Main Thread 'main': Run Network
 *---------------------------------------------------------------------------*/
__NO_RETURN void app_main (void *arg) {
  (void)arg;
	
	
  LED_Initialize();
	ADC_Initialize();
	RTC_Init();
	netInitialize ();
	Init_ThSNTP();

  TID_Display = osThreadNew (Display,  NULL, NULL);
  TID_Led     = osThreadNew (BlinkLed, NULL, NULL);
	Init_Thread();
	while (1) {
		
		RTC_CalendarShow(aShowTime, aShowDate);
  }
}



//Inicialización de los LEDS y del ADC

void LED_Initialize(){
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	
	
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	
	//Pulsador Azul
	
	GPIO_InitStruct.Pin= GPIO_PIN_13;
  GPIO_InitStruct.Mode= GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull= GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void ADC_Initialize(){
	ADC1_pins_F429ZI_config(); //specific PINS configuration
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
}

//Inicialización RTC
void RTC_Init(void){
	
	HAL_RTC_MspInit(&RtcHandle);

	
	RtcHandle.Instance = RTC; 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  __HAL_RTC_RESET_HANDLE_STATE(&RtcHandle);
	HAL_RTC_Init(&RtcHandle);
  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_CalendarConfig();
  }
  else
  {
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
    {
      /* Turn on LED2: Power on reset occurred */
   
    }
    /* Check if Pin Reset flag is set */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
      /* Turn on LED1: External reset occurred */

    }
    /* Clear source Reset Flag */
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }
	HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
	RTC_SetAlarm();
}

/*----------------------------------------------------------------------------
  Thread 'PROVEEDOR LCD'
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thread;        // thread id

osMessageQueueId_t mid_MsgQueue; 

int Init_MsgQueue(void){
	mid_MsgQueue_LCD = osMessageQueueNew(MSGQUEUE_OBJECTS ,sizeof(MSGQUEUE_OBJ_t_LCD), NULL);
	if (mid_MsgQueue_LCD == NULL) {
   ; // Message Queue object not created, handle failure
  }
	return(0);
}

void Thread (void *argument);                   // thread function
 
int Init_Thread (void) {
 
  tid_Thread = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread (void *argument) {
 Init_MsgQueue();
  while (1) {
		osThreadFlagsWait(0x01U, osFlagsWaitAny, osWaitForever);
		sprintf(msg_enviado.Buf, lcd_text[0]);
		msg_enviado.Idx = 0;
		osMessageQueuePut(mid_MsgQueue_LCD, &msg_enviado, 0U, 0U);
		sprintf(msg_enviado.Buf, lcd_text[1]);
		msg_enviado.Idx = 1;
		osMessageQueuePut(mid_MsgQueue_LCD, &msg_enviado, 0U, 0U);
		
    osThreadYield();                            // suspend thread
  }
}