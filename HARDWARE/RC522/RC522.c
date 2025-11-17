#include "RC522.h"
#include "delay.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "RS485.h"

#define MAX_JSON_SIZE 256  

unsigned char RC522_Status;
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
u8 KEY[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8 BUF[4]={0xff,0xff,0xff,0xff};
u16 time_522_flag; 
u32 sn;
char decimal[11];
char block_JOSN[MAX_JSON_SIZE];
uint32_t BEEP_TimCount; 


u8 SPI3_ReadWriteByte(u8 TxData)
{		 			 
	while((SPI3->SR&1<<1)==0);		//等待发送区空 
	SPI3->DR=TxData;	 	  		//发送一个byte  
	while((SPI3->SR&1<<0)==0);		//等待接收完一个byte  
 	return SPI3->DR;          		//返回收到的数据				    
}


void RC522_Init(void)
{
	u16 tempreg=0;
	RCC->AHB1ENR|=1<<0;    	//使能PORTA时钟	
	RCC->AHB1ENR|=1<<1;    	//使能PORTB时钟	
	RCC->AHB1ENR|=1<<2;    	//使能PORTC时钟	
	RCC->APB1ENR|=1<<15;   	//SPI3时钟使能 
	//PB3 SCK  PB5 MOSI PB4 MISO       
	GPIO_Set(GPIOB,PIN3 | PIN5 | PIN4 ,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	
 
  GPIO_AF_Set(GPIOB,3,6);	//PB3,AF6
 	GPIO_AF_Set(GPIOB,4,6);	//PB4,AF6
 	GPIO_AF_Set(GPIOB,5,6);	//PB5,AF6 

	//这里只针对SPI口初始化
	RCC->APB1RSTR|=1<<15;	//复位SPI3
	RCC->APB1RSTR&=~(1<<15);//停止复位SPI3
	tempreg|=0<<10;			//全双工模式	
	tempreg|=1<<9;			//软件nss管理
	tempreg|=1<<8;			 
	tempreg|=1<<2;			//SPI主机  
	tempreg|=0<<11;			//8位数据格式	
	tempreg|=0<<1;			//空闲模式下SCK为低电平  CPOL=0
	tempreg|=0<<0;			//数据采样从第1个时间边沿开始,CPHA=0  
	 	//对SPI3属于APB1的外设.时钟频率最大为42Mhz频率.
	tempreg|=7<<3;			//Fsck=Fpclk1/256
	tempreg|=0<<7;			//MSB First  
	tempreg|=1<<6;			//SPI启动 
	SPI3->CR1=tempreg; 		//设置CR1  
	SPI3->I2SCFGR&=~(1<<11);//选择SPI模式
  SPI3_ReadWriteByte(0xff);
 
 	GPIO_Set(GPIOC,PIN10,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	 //NSS PC10
	GPIO_Set(GPIOC,PIN11,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	 //IRQ PC11
	GPIO_Set(GPIOC,PIN12,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	 //RST PC12
	
	GPIO_Set(GPIOA,PIN3,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	 //BEEP PA3
	
	
	PcdReset(); //复位RC522
	PcdAntennaOff();
	PcdAntennaOn();
	M500PcdConfigISOType('A');
	
}

void BEEP_ON(void)
{
	BEEP = 1;
}

void BEEP_OFF(void)
{
	BEEP = 0;
}

//失能复位
void RC522_Reset_Disable(void)
{
	RC522_RST = 1;
}
//失能片选
void RC522_CS_Disable(void)
{
	RC522_CS=1;
//	GPIO_BOP(GPIOA) = GPIO_PIN_8;
}
//使能复位
void RC522_Reset_Enable(void)
{
	RC522_RST = 0;
	//GPIO_BC(GPIOB) = GPIO_PIN_12;
}
//使能片选
void RC522_CS_Enable(void)
{
	RC522_CS = 0;
//	GPIO_BC(GPIOA) = GPIO_PIN_8;
}

char PcdReset(void)
{
	RC522_Reset_Disable();
	delay_us(1);
	RC522_Reset_Enable();
	delay_us(1);
	RC522_Reset_Disable();
  delay_us(1);
	WriteRawRC(CommandReg,PCD_RESETPHASE);
	WriteRawRC(CommandReg,PCD_RESETPHASE);
	delay_us(1);
	WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
  WriteRawRC(TReloadRegL,30);   
  WriteRawRC(TReloadRegH,0);
  WriteRawRC(TModeReg,0x8D);
  WriteRawRC(TPrescalerReg,0x3E);
	WriteRawRC(TxAutoReg,0x40);//必须要
  return MI_OK;
}

void WriteRawRC(u8   Address, u8   value)
{  
	u8 ucAddr;
	RC522_CS_Enable();
	ucAddr = ((Address<<1)&0x7E);
	SPI3_ReadWriteByte(ucAddr);
	SPI3_ReadWriteByte(value);
	RC522_CS_Disable();
}
//关闭天线
void PcdAntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}
/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn(void)
{
	u8   i;
	i = ReadRawRC(TxControlReg);
	if (!(i & 0x03))
	{
			SetBitMask(TxControlReg, 0x03);
	}
}//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
void SetBitMask(u8   reg,u8   mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
void ClearBitMask(u8 reg,u8 mask)  
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 
//功    能：读RC522寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
u8 ReadRawRC(u8 Address)
{
    u8   ucAddr;
    u8   ucResult=0;
	  RC522_CS_Enable();
    ucAddr = ((Address<<1)&0x7E)|0x80;
	  SPI3_ReadWriteByte(ucAddr);
	  ucResult=SPI3_ReadWriteByte(0XFF);
	  RC522_CS_Disable();
    return ucResult;
}

//设置RC522的工作方式 
char M500PcdConfigISOType(u8   type)
{
   if (type == 'A')                     //ISO14443_A
   { 
       ClearBitMask(Status2Reg,0x08);
       WriteRawRC(ModeReg,0x3D);//3F
       WriteRawRC(RxSelReg,0x86);//84
       WriteRawRC(RFCfgReg,0x78);   //4F
   	   WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
	     WriteRawRC(TReloadRegH,0);
       WriteRawRC(TModeReg,0x8D);
	     WriteRawRC(TPrescalerReg,0x3E);
	     delay_us(1);
       PcdAntennaOn();
   }
   else{ return 1; }
   return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(u8 req_code,u8 *pTagType)
{
	char   status;  
	u8   unLen;
	u8   ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);
	WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = MI_ERR;   }
   
	return status;
}


//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pIn [IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOut [OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(u8   Command, 
                 u8 *pIn , 
                 u8   InLenByte,
                 u8 *pOut , 
                 u8 *pOutLenBit)
{
    char   status = MI_ERR;
    u8   irqEn   = 0x00;
    u8   waitFor = 0x00;
    u8   lastBits;
    u8   n;
    u16   i;
    switch (Command)
    {
        case PCD_AUTHENT:
													irqEn   = 0x12;
													waitFor = 0x10;
													break;
				case PCD_TRANSCEIVE:
													irqEn   = 0x77;
													waitFor = 0x30;
													break;
				default:	break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);	//清所有中断位
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//清FIFO缓存
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pIn [i]);    }
    WriteRawRC(CommandReg, Command);	   
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }	 //开始传送										 
      i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {    
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOut [i] = ReadRawRC(FIFODataReg);    }
            }
        }
        else
        {   status = MI_ERR;   }
        
    }
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}

//用MF522计算CRC16函数
void CalulateCRC(u8 *pIn,u8 len,u8 *pOut )
{
    u8   i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIn +i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}

//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdSelect(u8 *pSnr)
{
    char   status;
    u8   i;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
char PcdAnticoll(u8 *pSnr)
{
    char   status;
    u8   i,snr_check=0;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK             
char PcdAuthState(u8 auth_mode,u8 addr,u8 *pKey,u8 *pSnr)
{
    char   status;
    u8   unLen;
		u8	 ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    memcpy(&ucComMF522Buf[2], pKey, 6); 
    memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          p [OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
char PcdRead(u8   addr,u8 *p )
{
    char   status;
    u8   unLen;
    u8   i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {    *(p +i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          p [IN]：写入的数据，16字节
//返    回: 成功返回MI_OK                 
char PcdWrite(u8   addr,u8 *p )
{
    char   status;
    u8   unLen;
    u8   i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)
        {    
        	ucComMF522Buf[i] = *(p +i);   
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
char PcdHalt(void)
{
    u8   status;
    u8   unLen;
    u8   ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}
extern u8 MCB_Flag;
extern u8 MCB_Current_State; // 添加空开状态变量，0=关，1=开
void SI522_Analysis(void)
{
	RC522_Status=PcdRequest(PICC_REQALL,CT);/*寻卡*/
		if(RC522_Status==MI_OK)	
		{
			RC522_Status=PcdAnticoll(SN);
		}	
		if(RC522_Status==MI_OK)	/*防冲撞*/
		{
			RC522_Status=PcdSelect(SN);
		}				
		if(RC522_Status==MI_OK)//选卡成功
		{
		 RC522_Status =PcdAuthState(0x60,0x03+1*4,KEY,SN);
		}
		if(RC522_Status==MI_OK)//验证成功
		{		

			if(time_522_flag==0 || memcmp(BUF,SN,4)!=0)
			{
				time_522_flag=5000;
				sn =SN[3];
			  sn = (sn<<8)|SN[2];
			  sn = (sn<<8)|SN[1];
			  sn = (sn<<8)|SN[0];
			  decimal[0]=sn%10000000000/1000000000+0x30;
				decimal[1]=sn%1000000000/100000000+0x30;
				decimal[2]=sn%100000000/10000000+0x30;
				decimal[3]=sn%10000000/1000000+0x30;
				decimal[4]=sn%1000000/100000+0x30;
				decimal[5]=sn%100000/10000+0x30;
				decimal[6]=sn%10000/1000+0x30;
				decimal[7]=sn%1000/100+0x30;
				decimal[8]=sn%100/10+0x30;
				decimal[9]=sn%10/1+0x30;
				decimal[10]='\0';		 

        memset(block_JOSN,0,sizeof(block_JOSN));
        memcpy(block_JOSN,"{\"SN\":\"",strlen("{\"SN\":\""));
				strcat(block_JOSN,decimal);
				strcat(block_JOSN,"\"}");
     		
			//	USART0_SendString((void *)block_JOSN);
				memset(BUF,0,4);
				memcpy(BUF,SN,4);
				if (MCB_Current_State == 0)
            {
                 
                MCB_Flag = 1; // 设置为开
                MCB_Current_State = 1; // 更新状态为开
            }
            else
            {
                // 当前是开状态，执行关操作
                MCB_Flag = 2; // 设置为关
                MCB_Current_State = 0; // 更新状态为关
            }
            
            BEEP_TimCount = 100;
 
				
				// 人工报修刷卡结束页面切换逻辑
//				menu.mode = 0;
//				menu.repair = 0;
//				menu_flag = 1;
			}else
			{
			}
		}
}



/*
	将2进制转换为10进制字符串
	binary  要转换的二进制
	return 十进制整数
*/

