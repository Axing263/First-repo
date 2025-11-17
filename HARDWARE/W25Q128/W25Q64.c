#include "W25Q64.h" 
#include "spi.h"
#include "delay.h"	   
 
 
u16 W25QXX_TYPE=0;	//默认是W25Q64

//4Kbytes为一个Sector
//16个扇区为1个Block
//W25Q64
//容量为16M字节,共有128个Block,4096个Sector 
													 
//初始化SPI FLASH的IO口
void W25QXX_Init(void)
{ 
	RCC->AHB1ENR|=1<<0;     //使能PORTA时钟 
	GPIO_Set(GPIOA,PIN4,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PA4推挽输出
	W25QXX_CS=1;			//SPI FLASH不选中
	SPI1_Init();		   			//初始化SPI
	SPI1_SetSpeed(SPI_SPEED_2);		//设置为21M时钟,高速模式 
	W25QXX_TYPE=W25QXX_ReadID();	//读取FLASH ID.
}  

//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	W25QXX_CS=0;                            //使能器件   
	SPI1_ReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
	byte=SPI1_ReadWriteByte(0Xff);             //读取一个字节  
	W25QXX_CS=1;                            //取消片选     
	return byte;   
} 
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void W25QXX_Write_SR(u8 sr)   
{   
	W25QXX_CS=0;                            //使能器件   
	SPI1_ReadWriteByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令    
	SPI1_ReadWriteByte(sr);               //写入一个字节  
	W25QXX_CS=1;                            //取消片选     	      
}   
//W25QXX写使能	
//将WEL置位   
void W25QXX_Write_Enable(void)   
{
	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_WriteEnable);      //发送写使能  
	W25QXX_CS=1;                            //取消片选     	      
} 
//W25QXX写禁止	
//将WEL清零  
void W25QXX_Write_Disable(void)   
{  
	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_WriteDisable);     //发送写禁止指令    
	W25QXX_CS=1;                            //取消片选     	      
} 		
//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128 	  
u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	W25QXX_CS=0;				    
	SPI1_ReadWriteByte(0x90);//发送读取ID命令	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	 			   
	Temp|=SPI1_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI1_ReadWriteByte(0xFF);	 
	W25QXX_CS=1;	
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPI1_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPI1_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI1_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	  { 
    pBuffer[i]=SPI1_ReadWriteByte(0XFF);   //循环读数  
    }
	W25QXX_CS=1;  				    	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	  u16 i;  
    W25QXX_Write_Enable();                  //SET WEL 
	  W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_PageProgram);      //发送写页命令   
    SPI1_ReadWriteByte((u8)((WriteAddr)>>16)); //发送24bit地址    
    SPI1_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI1_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//循环写数  
		W25QXX_CS=1;                            //取消片选 
		W25QXX_Wait_Busy();					   //等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)						
//NumByteToWrite:要写入的字节数(最大65535)   
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
 	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;//扇区地址  
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QXX_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 
}
//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();                  //SET WEL 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_ChipErase);        //发送片擦除命令  
	W25QXX_CS=1;                            //取消片选     	      
	W25QXX_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{    
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  //SET WEL 	 
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((u8)Dst_Addr);  
	W25QXX_CS=1;                            //取消片选     	      
    W25QXX_Wait_Busy();   				   //等待擦除完成
}  
//等待空闲
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
	W25QXX_CS=1;                            //取消片选     	      
    delay_us(3);                               //等待TPD  
}   
//唤醒
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                            //使能器件   
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
	W25QXX_CS=1;                            //取消片选     	      
    delay_us(3);                               //等待TRES1
}   





u8 is_all_FF(u8 *array, size_t length) 
{
    for (size_t i = 0; i < length; i++) 
	{
        if (array[i] != 0xFF) 
				{
            return 0;
        }
   }
    return 1;
}



_Set_Type Set_Type;
u32 Time_Interval;
u16 Current_Set;
u16 Voltage_Set;
u16 Current_Alarm;
u16 Voltage_Alarm;



void Read_Set(void)
{
	u8 Read_Buf[255];
	W25QXX_Read(Read_Buf,0,160);  
  
	memset(Set_Type.ver,0,10);
	memset(Set_Type.port,0,10);
	memset(Set_Type.ip,0,30);
	memset(Set_Type.interval,0,4);
	memset(Set_Type.votage_set,0,2);
	memset(Set_Type.current_set,0,2);
	memset(Set_Type.ssid,0,20);
	memset(Set_Type.pwd,0,20);
	/*OTA更新用的定义
	memset(Set_Type.flilesize,0,4);
	memset(Set_Type.url,0,50);
	memset(Set_Type.flag,0,1);
  */
	memcpy(Set_Type.ver,Read_Buf,10);             //版本号          ASCII              0     10个字节
	memcpy(Set_Type.interval,Read_Buf+10,4);      //上传间隔时间    HEX    高位在前    10    4个字节 
	memcpy(Set_Type.port,Read_Buf+18,10);         //上传服务器端口  ASCII              18    10个字节
	memcpy(Set_Type.ip,Read_Buf+28,30);           //上传服务器IP    ASCII              28    30个字节
	memcpy(Set_Type.votage_set,Read_Buf+109,2);	  //焊机工作电流    HEX                109   2个字节
	memcpy(Set_Type.current_set,Read_Buf+111,2);	//焊接工作电压    HEX                111   2个字节
	
	memcpy(Set_Type.votage_alarm,Read_Buf+113,2);	  //焊机预警电压    HEX              113   2个字节
	memcpy(Set_Type.current_alarm,Read_Buf+115,2);	//焊接预警电流    HEX              115   2个字节
	/*OTA更新用的定义
	memcpy(Set_Type.url,Read_Buf+58,50);          //OTA更新地址     ASCII              58    50个字节
	memcpy(Set_Type.flag,Read_Buf+108,1);         //更新标志位      ASCII              108   1个字节
  memcpy(Set_Type.flilesize,Read_Buf+14,4);	    //更新文件大小    HEX                14    4个字节 
	*/
  memcpy(Set_Type.ssid,Read_Buf+121,20);	//WIFI名字    ASCII               121   20个字节
	memcpy(Set_Type.pwd,Read_Buf+141,20);	//WIFI密码    ASCII               141   20个字节
	
	if(is_all_FF(Set_Type.interval,4)==0)   
  Time_Interval=Set_Type.interval[0]*256*256*256+Set_Type.interval[1]*256*256+Set_Type.interval[2]*256+Set_Type.interval[3];
	else 
	Time_Interval=10;
 
	if(is_all_FF(Set_Type.current_set,2)==0)   
	Current_Set=Set_Type.current_set[0]*256+Set_Type.current_set[1];
	else
	Current_Set=10;
	
	if(is_all_FF(Set_Type.votage_set,2)==0)   
	Voltage_Set=Set_Type.votage_set[0]*256+Set_Type.votage_set[1];
	else
	Voltage_Set=10;
		
	if(is_all_FF(Set_Type.votage_alarm,2)==0)   
	Voltage_Alarm=Set_Type.votage_alarm[0]*256+Set_Type.votage_alarm[1];
	else
	Voltage_Alarm=100;
	
	
	if(is_all_FF(Set_Type.current_alarm,2)==0)   
	Current_Alarm=Set_Type.current_alarm[0]*256+Set_Type.current_alarm[1];
	else
	Current_Alarm=500;
	
	if(is_all_FF(Set_Type.port,10)==1)
  {
		memset(Set_Type.port,0,10);
		memcpy(Set_Type.port,"1883",4);
	}		
  //memcpy(Set_Type.port,"1883",4);
	
	if(is_all_FF(Set_Type.ip,30)==1)
	{
		memset(Set_Type.ip,0,30);
		//memcpy(Set_Type.ip,"10.168.88.171",strlen("10.168.88.171"));
		//memcpy(Set_Type.ip,"10.168.89.68",strlen("10.168.89.68"));
	 // memcpy(Set_Type.ip,"120.26.4.156",strlen("120.26.4.156"));
		memcpy(Set_Type.ip,"36.212.11.228",strlen("36.212.11.228"));
	}	
//	memcpy(Set_Type.ip,"36.212.11.228",strlen("36.212.11.228"));
	
	 // memcpy(Set_Type.ip,"120.26.4.156",strlen("120.26.4.156"));
	if(is_all_FF(Set_Type.ver,10)==1)
	{
		memset(Set_Type.ver,0,10);
		memcpy(Set_Type.ver,"1.0.0",strlen("1.0.0"));
	}	
}



 	
	
 

 






