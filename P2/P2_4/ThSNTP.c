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

void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction);

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

    // Realiza la petición SNTP (usa el puerto 123 por defecto)

    netSNTPc_GetTime (NULL, sntp_client_cb);

    // Espera 3 minutos (3 * 60 * 1000 ms)

    osDelay(1800000); 
  }
 }

void sntp_client_cb (uint32_t seconds, uint32_t seconds_fraction) {


	time_t unix_time = (time_t)(seconds-2208988800U);

    // 2. Ajustar Zona Horaria (Ejemplo: UTC-5)
    unix_time -= (5 * 3600); 

    // 3. Convertir a estructura humana usando la librería estándar
    struct tm *ts;
    ts = gmtime(&unix_time); // gmtime es más seguro en embebidos que localtime

    // 4. Configurar el hardware del RTC
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = ts->tm_hour;
    sTime.Minutes = ts->tm_min;
    sTime.Seconds = ts->tm_sec;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN);

    sDate.WeekDay = (ts->tm_wday == 0) ? RTC_WEEKDAY_SUNDAY : ts->tm_wday;
    sDate.Month = ts->tm_mon + 1; // tm_mon es 0-11, RTC espera 1-12
    sDate.Date = ts->tm_mday;
    sDate.Year = (ts->tm_year + 1900) - 2000; // tm_year son años desde 1900
    HAL_RTC_SetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN);
		osThreadFlagsSet(TID_Led,0x02);
}

