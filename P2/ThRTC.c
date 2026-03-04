#include "RTC.h"
#include "cmsis_os2.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_RTC;                        // thread id
void ThRTC (void *argument); 

 typedef struct{
	char Buf[128]; 			//Buffer que almacena los datos del mensaje
	uint8_t Idx;					//Índice
}MSGQUEUE_OBJ_t;
 MSGQUEUE_OBJ_t msg_enviado;

osMessageQueueId_t mid_RTCQueue;

/*----------------------------------------------------------------------------
  Inicialización del hilo
 *---------------------------------------------------------------------------*/

int Init_ThRTC (void) {
 
  tid_RTC = osThreadNew(ThRTC, NULL, NULL);
  if (tid_RTC == NULL) {
    return(-1);
  }
 
  return(0);
}

int Init_MsgQueue(void){
	mid_RTCQueue = osMessageQueueNew(5 ,sizeof(MSGQUEUE_OBJ_t), NULL);
	if (mid_RTCQueue == NULL) {
   ; // Message Queue object not created, handle failure
  }
	return(0);
}
 

/*----------------------------------------------------------------------------
  Thread 'ThRTC'
 *---------------------------------------------------------------------------*/
void ThRTC (void *argument) {
  
º	Init_MsgQueue();
	
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
  while (1) {
		RTC_CalendarShow(aShowTime, aShowDate);
    
		sprintf(msg_enviado.Buf,"%s       ", aShowTime);
		msg_enviado.Idx = 0;
		osMessageQueuePut(mid_RTCQueue, &msg_enviado, 0U, 0U);
		osDelay(50);
		sprintf(msg_enviado.Buf,"%s       ", aShowDate);
		msg_enviado.Idx = 1;
		osMessageQueuePut(mid_RTCQueue, &msg_enviado, 0U, 0U);
		osDelay(50);
		
    osThreadYield();                            // suspend thread
  }
}
