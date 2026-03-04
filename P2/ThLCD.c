#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "main.h"
#include "LCD.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
extern osMessageQueueId_t mid_RTCQueue; 
extern uint16_t positionL1;
extern uint16_t positionL2;
osThreadId_t tid_Thread2;                        // thread id
 
void Thread2 (void *argument);                   // thread function
 
 
int Init_Thread2 (void) {
 
  tid_Thread2 = osThreadNew(Thread2, NULL, NULL);
  if (tid_Thread2 == NULL) {
    return(-1);
  }
 
  return(0);
}
 

typedef struct{
	char Buf[128]; 			//Buffer que almacena los datos del mensaje
	uint8_t Idx;					//Índice
}MSGQUEUE_OBJ_t;

MSGQUEUE_OBJ_t msg_recibido;


osStatus_t status;

void Pintar(MSGQUEUE_OBJ_t msg);

void Thread2 (void *argument) {
  LCD_reset();
	LCD_init();
	LCD_Clean();
  while (1) {
    status = osMessageQueueGet(mid_RTCQueue, &msg_recibido, NULL, osWaitForever);
		if(status == osOK){
			Pintar(msg_recibido);
		}	
		osThreadYield(); 
  }
}

void Pintar(MSGQUEUE_OBJ_t msg){
	int fin = 0;
	positionL1 = 0;
	positionL2 = 0;
	if(msg.Idx == 0){
		for(int i = 0; i<128; i++){
			if(msg.Buf[i]!=0x00 && fin!= 1){
				symbolToLocalBuffer_L1(msg.Buf[i]);
				if(msg.Buf[i+1]==0x00){
					fin = 1;
				}
			}
		}
		LCD_update();
	}else{
		for(int i = 0; i<128; i++){
			if(msg.Buf[i]!=0x00 && fin!= 1){
				symbolToLocalBuffer_L2(msg.Buf[i]);
				if(msg.Buf[i+1]==0x00){
					fin = 1;
				}
			}
		}
		LCD_update();
	}
}
