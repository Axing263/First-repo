#include "fontupd.h"
#include "w25q64.h"   
#include "string.h"
#include "delay.h"
#include "dma.h"	

//------------------------------------------------------------------------------------------------------------------------
//字库区域占用的总扇区数大小(4个字库+unigbk表+字库信息=6302984字节,约占1539个W25QXX扇区,一个扇区4K字节)
#define FONTSECSIZE	 	1539
//字库存放起始地址 
#define FONTINFOADDR 	1024*1024*1 					//Apollo STM32开发板是从1M地址以后开始存放字库
														//前面5M被fatfs占用了.
														//1M以后紧跟4个字库+UNIGBK.BIN,总大小6.01M,被字库占用了,不能动!
														//31.01M以后,用户可以自由使用.
//定义各个字库的大小
#define UNIGBK         171*1024        //171KB
#define GBK12_FONSIZE  562*1024        //562KB
#define GBK16_FONSIZE  749*1024		     //749KB
#define GBK24_FONSIZE  1684*1024       //1684KB
#define GBK32_FONSIZE  2993*1024       //2993KB
//------------------------------------------------------------------------------------------------------------------------
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;


//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
u8 font_init(void)
{		
	u8 t=0;
	W25QXX_Init();  
	printf("开始检查字库...\r\n");
	while(t<10)//连续读取10次,都是错误,说明确实是有问题,得更新字库了
	{
		t++;
		W25QXX_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//读出ftinfo结构体数据
		if(ftinfo.fontok==0XAA)break;
		delay_ms(20);
	}
	if(ftinfo.fontok!=0XAA)
	{
		printf("检查完成，字库丢失...\r\n");
		return 1;
	}
	printf("检查完成，字库完好...\r\n");
	return 0;		    
}

//显示当前字体更新进度
//x,y:坐标
//size:字体大小
//fsize:整个文件大小
//pos:当前文件指针位置
u32 fupd_prog(u16 x,u16 y,u8 size,u32 fsize,u32 pos)
{
	float prog;
	u8 t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
	if(t!=prog)
	{
		//LCD_ShowString(x+3*size/2,y,240,320,size,"%");		
		t=prog;
		if(t>100)t=100;
	//	LCD_ShowNum(x,y,t,3,size);//显示数值
	}
	return 0;					    
} 

//更新某一个
//x,y:坐标
//size:字体大小
//fx:更新的内容 0,ungbk;1,gbk12;2,gbk16;3,gbk24;
//返回值:0,成功;其他,失败.
u8 updata_fontx(u16 x,u16 y,u8 size,u8 fx)
{
	u32 flashaddr=0;								    
	u32 offx=0;      //接收到的文件大小
	u32 fsize=0;	 //总文件大小
	u16 over_len =0; //最后数据长度
	
	switch(fx)
	{
		case 0:												//更新UNIGBK.BIN
			ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);	//信息头之后，紧跟UNIGBK转换码表
			fsize=ftinfo.ugbksize=UNIGBK;					//UNIGBK大小
			flashaddr=ftinfo.ugbkaddr;						//UNIGBK的起始地址
			printf("Please send UNIGBK.bin\r\n");
			break;
		case 1:
			ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;	//UNIGBK之后，紧跟GBK12字库
			fsize=ftinfo.gbk12size=GBK12_FONSIZE;			//GBK12字库大小
			flashaddr=ftinfo.f12addr;						//GBK12的起始地址
			printf("Please send GBK12.FON\r\n");
			break;
		case 2:
			ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;	//GBK12之后，紧跟GBK16字库
			fsize=ftinfo.gbk16size=GBK16_FONSIZE;			//GBK16字库大小
			flashaddr=ftinfo.f16addr;						//GBK16的起始地址
			printf("Please send GBK16.FON\r\n");
			break;
		case 3:
			ftinfo.f24addr=ftinfo.f16addr+ftinfo.gbk16size;	//GBK16之后，紧跟GBK24字库
			fsize=ftinfo.gbk24size=GBK24_FONSIZE;			//GBK24字库大小
			flashaddr=ftinfo.f24addr;						//GBK24的起始地址
			printf("Please send GBK24.FON\r\n");
			break;
		case 4:
			ftinfo.f32addr=ftinfo.f24addr+ftinfo.gbk24size;	//GBK24之后，紧跟GBK32字库
			fsize=ftinfo.gbk32size=GBK32_FONSIZE;			//GBK32字库大小
			flashaddr=ftinfo.f32addr;						//GBK32的起始地址
			printf("Please send GBK32.FON\r\n");
			break;
	} 
	fupd_prog(x,y,size,fsize,offx);	 			//进度显示
	GBK_BUF_Flag	= 2; //清除标志等待下一次DMA中断
	GBK_OVER_Flag	= 0; //停止计时
	printf("请发送文件...\r\n");
	do
	{
		if(GBK_OVER_Flag)GBK_OVER_Flag++;	//不为0开始计时
		if(GBK_BUF_Flag!=2)//判断是否进入DMA中断
		{		
			GBK_OVER_Flag=1; //开始计时
			if(GBK_BUF_Flag==0) //当前目标在存储器0
			{
				W25QXX_Write(Usart1_Rece_Buf1,offx+flashaddr,Usart1_DMA_Len);		//开始写入Usart6_DMA_Len个数据  
			}
			else 
			if(GBK_BUF_Flag==1) //当前目标在存储器0
			{
				W25QXX_Write(Usart1_Rece_Buf0,offx+flashaddr,Usart1_DMA_Len);		//开始写入Usart6_DMA_Len个数据  
			}
			offx+=Usart1_DMA_Len; 	//加上已写完的数据量		
			fupd_prog(x,y,size,fsize,offx);	 			//进度显示
			GBK_BUF_Flag=2; 		//清除标志等待下一次DMA中断
		}		
		delay_us(100); //延时100us	
	}
	while(GBK_OVER_Flag<=WATE_TIME); //判断超时？
		
	over_len = Usart1_DMA_Len-DMA2_Stream5->NDTR; //最后数据长度	
	if(DMA2_Stream5->CR&(1<<19)) //得到CT当前目标存储器
	{
	   W25QXX_Write(Usart1_Rece_Buf1,offx+flashaddr,over_len);//将DMA最后的一帧数据写入FLASH
	}
	else
	{
	   W25QXX_Write(Usart1_Rece_Buf0,offx+flashaddr,over_len);//将DMA最后的一帧数据写入FLASH
	}
	offx+=over_len; //加上已写完的数据量
	fupd_prog(x,y,size,fsize,offx);	 			//进度显示
	//重新开启一次DMA传输
	MYDMA_REST_Enable(DMA2_Stream5,(u32)&USART1->DR,(u32)Usart1_Rece_Buf0,Usart1_DMA_Len);  //重置DMA
	printf("已写完数据,本次接收到%d字节！！！\r\n",offx);
	printf(" \r\n");
	delay_ms(100); //延时100ms
	if((offx > (fsize-1024))&(offx < fsize)) //误差在正负1K之内都认为是更新成功
		return 0; //更新成功
	else
		printf("更新失败,重新开始!\r\n");
		return 1; //更新失败
} 

//更新字体文件,UNIGBK,GBK12,GBK16,GBK24一起更新
//x,y:提示信息的显示地址
//size:字体大小
//提示信息字体大小										  
//返回值:0,更新成功;
//		 其他,错误代码.	  
u8 update_font(u16 x,u16 y,u8 size)
{	
	u16 i,j;
	u8 flag=0; //状态
	 
cl:	//更新失败就跳转到这重新更新
	printf("正在擦除扇区...\r\n");
	for(i=0;i<FONTSECSIZE;i++)	//先擦除字库区域,提高写入速度
	{
		fupd_prog(x+20*size/2,y,size,FONTSECSIZE,i);	//进度显示%
		W25QXX_Read((u8*)Usart1_Rece_Buf1,((FONTINFOADDR/4096)+i)*4096,4096);//读出整个扇区的内容(借用一下DMA缓冲区)
		for(j=0;j<4096;j++)//校验数据
		{
			if(Usart1_Rece_Buf1[j]!=0XFF)break;//需要擦除  	  
		}
		if(j!=4096)W25QXX_Erase_Sector((FONTINFOADDR/4096)+i);	//需要擦除的扇区
	}
	delay_ms(100); //延时100ms
		
	printf("1.下一个，请发送UNIGBK.BIN文件...\r\n");
	flag=updata_fontx(x+20*size/2,y,size,0);	//更新UNIGBK.BIN
	if(flag==1)goto cl; //更新不成功就重新开始

	printf("2.下一个，请发送GBK12.BIN文件...\r\n");
	flag=updata_fontx(x+20*size/2,y,size,1);	//更新GBK12.FON
	if(flag==1)goto cl; //更新不成功就重新开始
  
	printf("3.下一个，请发送GBK16.BIN文件...\r\n");
	flag=updata_fontx(x+20*size/2,y,size,2);	//更新GBK16.FON
	if(flag==1)goto cl; //更新不成功就重新开始

	printf("4.下一个，请发送GBK24.BIN文件...\r\n");
	flag=updata_fontx(x+20*size/2,y,size,3);	//更新GBK24.FON
	if(flag==1)goto cl; //更新不成功就重新开始

	printf("5.下一个，请发送GBK32.BIN文件...\r\n");
	flag=updata_fontx(x+20*size/2,y,size,4);	//更新GBK32.FON
	if(flag==1)goto cl; //更新不成功就重新开始
		
	//全部更新好了
	ftinfo.fontok=0XAA; //写更新完成标志
	W25QXX_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//保存字库信息
	printf("恭喜您！字库已更新成功!\r\n");
  return 0;
}



























