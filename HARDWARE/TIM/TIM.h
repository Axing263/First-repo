#ifndef __TIM_H
#define __TIM_H
#include "sys.h"
#include "stdint.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef NULL
#define NULL  0
#endif


/*!
  * @brief 定时器时标信息定义
  *
  */
typedef struct
{
    u8 bTic1msSign ;          /*!< 1ms定时标志 */
    u8 bTic10msSign ;         /*!< 10ms定时标志 */
    u8 bTic20msSign ;         /*!< 20ms定时标志 */
    u8 bTic100msSign ;        /*!< 100ms定时标志 */
	  u8 bTic200msSign ;        /*!< 200ms定时标志 */
    u8 bTic500msSign ;        /*!< 500ms定时标志 */
    u8 bTic1secSign;          /*!< 1s定时标志 */
}Tim_TimeSignType;


typedef struct
{
    u8 sec;
	  u8 min;
	  u8 hour;
} r_work_time;


typedef void (*TimeFun)(void);
extern Tim_TimeSignType g_tTimeSign;

void TIM2_Int_Init(u16 arr,u16 psc); 
void TIM3_Int_Init(u16 arr,u16 psc);


extern void FML_TIME_Init(void);

/* 定时处理函数 ****************************************************************/
extern int FML_TIME_Register(TimeFun pTimeFun, uint32_t time);
extern void FML_TIME_UnRegister(TimeFun pTimeFun);


#endif




