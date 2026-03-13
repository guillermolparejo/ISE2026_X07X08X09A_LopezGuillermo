#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "RTC.h"
#include <time.h>
#include "rl_net.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
extern osThreadId_t TID_Led;
osThreadId_t tid_ThSNTP;                        // thread id

void ThSNTP (void *argument);                   // thread function
extern uint8_t g_woke_by_button;
void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction);
const NET_ADDR ntp_server = {NET_ADDR_IP4,0,216,239,35,0};
int Init_ThSNTP (void) {

  tid_ThSNTP = osThreadNew(ThSNTP, NULL, NULL);

  if (tid_ThSNTP == NULL) {
    return(-1);
  }
  return(0);
}

 
void ThSNTP (void *argument) {

    osDelay(5000); // Espera inicial de 5s tras el arranque
  while (1) {

    netSNTPc_GetTime((NET_ADDR*)&ntp_server, sntp_client_cb);

    osDelay(180000); 
  }
 }

void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction) {

    struct tm ts;
    uint32_t UTC_seconds = seconds+3600;
  
    ts = *localtime(&UTC_seconds);

    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = ts.tm_hour;
    sTime.Minutes = ts.tm_min;
    sTime.Seconds = ts.tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        return;
    }
    sDate.WeekDay = ts.tm_wday + 1;
    sDate.Month = ts.tm_mon + 1;      
    sDate.Date = ts.tm_mday;
    sDate.Year = ts.tm_year - 100;

    if (HAL_RTC_SetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        return;
    }
    osThreadFlagsSet(TID_Led,0x02);
}

void EXTI15_10_IRQHandler(void){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == GPIO_PIN_13){
	g_woke_by_button=1;
	RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  sdatestructure.Year = 0x00;
  sdatestructure.Month = RTC_MONTH_JANUARY;
  sdatestructure.Date = 0x1;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  

  stimestructure.Hours = 0x00;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;
  stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
  stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

  HAL_RTC_SetDate(&RtcHandle, &sdatestructure, RTC_FORMAT_BCD);
  HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD);
	}
}
