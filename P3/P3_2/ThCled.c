#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_ThCled;                        // thread id
extern GPIO_InitTypeDef GPIO_InitStruct;
void ThCled (void *argument);                   // thread function
 
int Init_ThCled (void) {
 
  tid_ThCled = osThreadNew(ThCled, NULL, NULL);
  if (tid_ThCled == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThCled (void *argument) {
 
  while (1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // <-- AJUSTA al pin real del LED verde de tu Nucleo-144
    osDelay(100);
  }
}
