#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThLED;                        // thread id
GPIO_InitTypeDef GPIO_InitStruct;
void ThLED (void *argument);                   // thread function
void LED_Initialize();
int Init_ThLED (void) {
 
  tid_ThLED = osThreadNew(ThLED, NULL, NULL);
  if (tid_ThLED == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLED (void *argument) {
  LED_Initialize();
  
  while (1) {
   osThreadFlagsWait(0x01,osFlagsWaitAny,osWaitForever);
    for(int i = 0; i++;i<5){
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
      osDelay(100);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    }
    
    osThreadYield();                            // suspend thread
  }
}

void LED_Initialize(){
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
}