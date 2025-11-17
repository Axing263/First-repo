#include "TIM.h"
#include "string.h"

extern uint32_t ML307R_TimCount;
extern uint32_t RS485_TimCount;
extern uint32_t WIFI_TimCount; 
extern uint32_t BEEP_TimCount; 
/**
  * @brief 可注册的定时回调函数最大数目
  */
#define SUPPORT_FUN_MAX_NUM      10

/* Private typedef -----------------------------------------------------------*/

/**
  * @brief TIME 句柄信息结构体定义
  */
typedef struct{
    TimeFun pTimeFun;                   /*!< 回调函数 */
    
    uint32_t ui1msTic;                  /*!< 1ms定时计时 */
    
    uint32_t ui1msMaxTime;              /*!< 回调周期时间 */
} TIME_FunType;

/**
  * @brief TIME 句柄信息结构体定义
  */
typedef struct{
    uint16_t ucSupportCnt;                           /*!< 注册的定时回调函数的数目 */
    
    TIME_FunType tCallFun[SUPPORT_FUN_MAX_NUM];     /*!< 注册的定时回调函数 */
    
    uint32_t ui1msTic;                              /*!< 1ms定时 */
    
    uint32_t uimsDelayTic;                              /*!< 1ms定时 */
} TIME_HandleType;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** 定时标志 */
Tim_TimeSignType g_tTimeSign = {0};

static TIME_HandleType sg_tTIME_Handle = {0};


//通用定时器2中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器2!
void TIM2_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<0;	//TIM2时钟使能    
 	TIM2->ARR=arr;  	//设定计数器自动重装值 
	TIM2->PSC=psc;  	//预分频器	  
	TIM2->DIER|=1<<0;   //允许更新中断	  
	TIM2->CR1|=0x01;    //使能定时器2
  MY_NVIC_Init(1,1,TIM2_IRQn,2);	//抢占1，子优先级3，组2									 
}
	
//定时器2中断服务程序	 
void TIM2_IRQHandler(void)
{ 		    		  			    
	if(TIM2->SR&0X0001)//溢出中断
	{
		
	}				   
	TIM2->SR&=~(1<<0);//清除中断标志位 	    
}


extern u16 flag_time;
//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		
  uint8_t i;	
	if(TIM3->SR&0X0001)//溢出中断
	{
		for (i = 0; i < sg_tTIME_Handle.ucSupportCnt && i < SUPPORT_FUN_MAX_NUM; i++)
        {
            sg_tTIME_Handle.tCallFun[i].ui1msTic++;
            
            if (sg_tTIME_Handle.tCallFun[i].ui1msTic >= sg_tTIME_Handle.tCallFun[i].ui1msMaxTime)
            {
                sg_tTIME_Handle.tCallFun[i].ui1msTic = 0;
                
                if (sg_tTIME_Handle.tCallFun[i].pTimeFun != NULL)
                {
                    sg_tTIME_Handle.tCallFun[i].pTimeFun();
                }
            }
        }
        
        sg_tTIME_Handle.uimsDelayTic++;
        sg_tTIME_Handle.ui1msTic++;
        
        sg_tTIME_Handle.ui1msTic % 1 == 0 ? g_tTimeSign.bTic1msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 10 == 0 ? g_tTimeSign.bTic10msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 20 == 0 ? g_tTimeSign.bTic20msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 100 == 0 ? g_tTimeSign.bTic100msSign = TRUE : 0;
				
				sg_tTIME_Handle.ui1msTic % 200 == 0 ? g_tTimeSign.bTic200msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 500 == 0 ? g_tTimeSign.bTic500msSign = TRUE : 0;
        
        sg_tTIME_Handle.ui1msTic % 1000 == 0 ? (g_tTimeSign.bTic1secSign = TRUE, sg_tTIME_Handle.ui1msTic = 0) : 0;
				
				ML307R_TimCount++;
			if(ML307R_TimCount >= 100000)
				ML307R_TimCount = 0;
				RS485_TimCount++;
			if(RS485_TimCount >= 100000)
				RS485_TimCount = 0;
			  WIFI_TimCount++;
			if(WIFI_TimCount >= 100000)
				WIFI_TimCount = 0;
			
			if(BEEP_TimCount > 0)
			{
				BEEP_ON();
			BEEP_TimCount--;
			}
			else 
			{
				BEEP_OFF();
			}
       		
	}				   
	TIM3->SR&=~(1<<0);//清除中断标志位 	    
}
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为42M
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
	RCC->APB1ENR|=1<<1;	//TIM3时钟使能    
 	TIM3->ARR=arr;  	//设定计数器自动重装值 
	TIM3->PSC=psc;  	//预分频器	  
	TIM3->DIER|=1<<0;   //允许更新中断	  
	TIM3->CR1|=0x01;    //使能定时器3
  MY_NVIC_Init(1,3,TIM3_IRQn,2);	//抢占1，子优先级3，组2									 
}




void FML_TIME_Init(void)
{
    //100Khz的计数频率，计数到100为1ms  
    TIM3_Int_Init(100-1,840-1);  //1ms中断 
    
    memset(&sg_tTIME_Handle, 0, sizeof(sg_tTIME_Handle));
}


/**
  * @brief      注册定时回调函数.
  * @note       注册的定时函数执行时间必须很短
  * @param[in]  pTimeFun 回调函数
  * @param[in]  time     回调周期时间, 单位毫秒
  * @retval     0,成功; -1,失败
  */
int FML_TIME_Register(TimeFun pTimeFun, uint32_t time)
{
    if (sg_tTIME_Handle.ucSupportCnt < SUPPORT_FUN_MAX_NUM)
    {
        sg_tTIME_Handle.tCallFun[sg_tTIME_Handle.ucSupportCnt].ui1msMaxTime = time;
        sg_tTIME_Handle.tCallFun[sg_tTIME_Handle.ucSupportCnt].pTimeFun = pTimeFun;
        sg_tTIME_Handle.ucSupportCnt++;
        return 0;
    }
    
    return -1;
}

/**
  * @brief      注销定时回调函数.
  * @param[in]  pTimeFun 回调函数
  * @retval     None
  */
void FML_TIME_UnRegister(TimeFun pTimeFun)
{
    uint16_t i;
    uint8_t ucSupportCnt = 0;
    TIME_FunType tCallFun[SUPPORT_FUN_MAX_NUM] = {0};
    
    for (i = 0; i < sg_tTIME_Handle.ucSupportCnt && i < SUPPORT_FUN_MAX_NUM; i++)
    {
        if (sg_tTIME_Handle.tCallFun[i].pTimeFun != pTimeFun)
        {
            tCallFun[ucSupportCnt] = sg_tTIME_Handle.tCallFun[i];
            ucSupportCnt++;
        }
    }
    
    for (i = 0; i < SUPPORT_FUN_MAX_NUM; i++)
    {
        sg_tTIME_Handle.tCallFun[i] = tCallFun[i];
    }
    
    sg_tTIME_Handle.ucSupportCnt = ucSupportCnt;
}

