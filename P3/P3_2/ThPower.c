#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 uint8_t g_woke_by_button = 0;
osThreadId_t tid_ThPower;                        // thread id
extern GPIO_InitTypeDef GPIO_InitStruct;
void ThPower (void *argument);                   // thread function
static volatile uint8_t slept_once = 0;
int Init_ThPower (void) {
 
  tid_ThPower = osThreadNew(ThPower, NULL, NULL);
  if (tid_ThPower == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThPower (void *argument) {
   g_woke_by_button = 0;
  osDelay(15000);

  /* Marca “voy a dormir” */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

  /* Suspende tick para que SysTick no despierte al core */
  HAL_SuspendTick();

  /* Entra en Sleep (WFI) */
  __HAL_RCC_PWR_CLK_ENABLE();
  while (g_woke_by_button == 0) {
    __WFI();
  }

  /* Aquí sigues cuando una interrupción (botón) te despierte */
  HAL_ResumeTick();

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
  slept_once = 1;
}
