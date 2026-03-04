#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
#define RTC_ASYNCH_PREDIV  0x7F   /* LSE as RTC clock */
#define RTC_SYNCH_PREDIV   0x00FF /* LSE as RTC clock */
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_RTC;                        // thread id
 
void ThRTC (void *argument);                   // thread function
 
RTC_HandleTypeDef RtcHandle;

/* Buffers used for displaying Time and Date */
uint8_t aShowTime[50] = {0};
uint8_t aShowDate[50] = {0}; 

 typedef struct{
	char Buf[128]; 			//Buffer que almacena los datos del mensaje
	uint8_t Idx;					//Índice
}MSGQUEUE_OBJ_t;
MSGQUEUE_OBJ_t msg_enviado;




static void RTC_CalendarConfig(void);
static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate);
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc);

void RTC_SetAlarm(void);

osMessageQueueId_t mid_RTCQueue;

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
 
void ThRTC (void *argument) {
  
	Init_MsgQueue();
	
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

static void RTC_CalendarConfig(void)
{
    RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 0x14;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x02;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

	
	HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BCD);
  HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD);

	
  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
}

static void RTC_CalendarShow(uint8_t *showtime, uint8_t *showdate)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  /* Display time Format : hh:mm:ss */
  sprintf((char *)showtime, "%2d:%2d:%2d", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
  /* Display date Format : mm-dd-yy */
  sprintf((char *)showdate, "%2d-%2d-%2d", sdatestructureget.Month, sdatestructureget.Date, 2000 + sdatestructureget.Year);
}

void RTC_SetAlarm(void){
  RTC_TimeTypeDef stime;
  RTC_AlarmTypeDef sAlarm;
  
  HAL_RTC_GetTime(&RtcHandle,&stime, RTC_FORMAT_BIN);
  
  sAlarm.AlarmTime.Hours = stime.Hours;
  sAlarm.AlarmTime.Minutes = (stime.Minutes+1)%60;
  sAlarm.AlarmTime.Seconds = 0;

  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
  sAlarm.AlarmSubSecondMask= RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay= 1;
  sAlarm.Alarm = RTC_ALARM_A;
  
  HAL_RTC_SetAlarm_IT(&RtcHandle,&sAlarm,RTC_FORMAT_BIN); 


}

void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    // 1. Habilitar el reloj del Power Controller (PWR)
    __HAL_RCC_PWR_CLK_ENABLE();

    // 2. Habilitar el acceso al dominio de Backup (RTC)
    HAL_PWR_EnableBkUpAccess();

    // 3. Configurar el oscilador LSE
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Error de configuración
    }

    // 4. Seleccionar LSE como fuente de reloj para el RTC
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        // Error de configuración
    }

    // 5. Habilitar el reloj del periférico RTC
    __HAL_RCC_RTC_ENABLE();
}