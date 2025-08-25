
#include <SoftwareSerial.h>
#include <TaskScheduler.h>
#include <SPI.h>
#include <MFRC522.h>
unsigned char ACDConfigRegK_Val ;
unsigned char ACDConfigRegC_Val ;
#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          6          // Configurable, see typical pin layout above
#define IRQ_PIN         2          // Configurable, depends on hardware

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

volatile bool bNewInt = false;
byte regVal = 0x7F;
void activateRec(MFRC522 mfrc522);
void clearInt(MFRC522 mfrc522);






int i = 0;
int ii = 0;
byte DetectData[18];
byte TagData[18];
int c = 0;
int d = 0;
int j = 0;
unsigned long dpm = 0;  // 记录上一次更新的时间
unsigned long dcm = 0;
unsigned long de = 0;
unsigned long pm = 0;  // 记录上一次更新的时间
unsigned long cm = 0;
unsigned long dianji = 0;
int jiance;
int p = 0;




void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}









void SI522A_SPI_LL_WriteRawRC(unsigned char RegAddr,unsigned char value)
{
	digitalWrite(SS_PIN, HIGH);  
	RegAddr = (RegAddr & 0x3f) << 1;                 //code the first byte	
	digitalWrite(SS_PIN, LOW);  
	//SPI1_ReadWriteByte(RegAddr);	//write address first
	//SPI1_ReadWriteByte(value);				//write value then 	
SPI.transfer(RegAddr);
SPI.transfer(value);
	digitalWrite(SS_PIN, HIGH);  
}




unsigned char SI522A_SPI_LL_ReadRawRC(unsigned char RegAddr)
{
	uint8_t RegVal=0;
	RegAddr = (RegAddr & 0x3f) << 1 | 0x80;   //code the first byte	
	digitalWrite(SS_PIN, LOW);  
	SPI.transfer(RegAddr);	//write address first
	RegVal = SPI.transfer(0x00);	
	digitalWrite(SS_PIN, HIGH);  
	return RegVal;
}







void I_SI522A_IO_Write(unsigned char RegAddr,unsigned char value)
{
    SI522A_SPI_LL_WriteRawRC(RegAddr,value);
}

unsigned char I_SI522A_IO_Read(unsigned char RegAddr)
{
    return SI522A_SPI_LL_ReadRawRC(RegAddr);
}




void I_SI522A_ClearBitMask(unsigned char reg,unsigned char mask)  
{
	char tmp = 0x00;
	tmp = I_SI522A_IO_Read(reg);
	I_SI522A_IO_Write(reg, tmp & ~mask);  // clear bit mask
} 


void I_SI522A_SetBitMask(unsigned char reg,unsigned char mask)  
{
	char tmp = 0x00;
	tmp = I_SI522A_IO_Read(reg);
	I_SI522A_IO_Write(reg,tmp | mask);  // set bit mask
}



void PCD_SI522A_TypeA_Init1(void)
{
	//I_SI522A_IO_Init();	// Initializes the interface with Si522	


/*

     pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // 默认拉高片选引脚

  // 初始化SPI通信
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16); // 设置SPI时钟频率
  SPI.setDataMode(SPI_MODE0);           // 设置SPI模式（Mode 0）
  SPI.setBitOrder(MSBFIRST);

*/

	//delay(500);
	

  
	I_SI522A_ClearBitMask(0x08, 0x08);  
	// Reset baud rates
	I_SI522A_IO_Write(0x12, 0x00);
	I_SI522A_IO_Write(0x13, 0x00);
	// Reset ModWidthReg
	I_SI522A_IO_Write(0x24, 0x26);
	// RxGain:110,43dB by default;
	I_SI522A_IO_Write(0x26, 0x78);
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	I_SI522A_IO_Write(0x2A, 0x80);// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	I_SI522A_IO_Write(0x2B, 0xa9);// TPreScaler = TModeReg[3..0]:TPrescalerReg
	I_SI522A_IO_Write(0x2C, 0x03); // Reload timer 
	I_SI522A_IO_Write(0x2D, 0xe8); // Reload timer 
	I_SI522A_IO_Write(0x15, 0x40);	// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	I_SI522A_IO_Write(0x11, 0x3D);	// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	I_SI522A_IO_Write(0x01, 0x00);  // Turn on the analog part of receiver   
  I_SI522A_IO_Write(0x14,I_SI522A_IO_Read(0x14) | 0x03);	//Tx1RFEn=1  Tx2RFEn=1
		delay(1);      //这里加一个小延时，会好一点！


}



void PCD_ACD_AutoCalc(void)
{
	unsigned char temp; 
	unsigned char temp_Compare=0; 
	unsigned char VCON_TR[8]={ 0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};//acd灵敏度调节
	unsigned char TR_Compare[4]={ 0x00, 0x00, 0x00, 0x00};
	ACDConfigRegC_Val = 0x7f;
	unsigned char	ACDConfigRegK_RealVal = 0;
	I_SI522A_IO_Write(0x14, 0x83);	//打开天线
	I_SI522A_SetBitMask(0x01, 0x06);	//开启ADC_EXCUTE
	delayMicroseconds(200);
	
	for(int i=7; i>0; i--)
	{	
		I_SI522A_IO_Write(0x20, (0x0a << 2) | 0x40);		
		I_SI522A_IO_Write(0x0F, VCON_TR[i]);
		
		I_SI522A_IO_Write(0x20, (0x06 << 2) | 0x40);
		temp_Compare = I_SI522A_IO_Read(0x0F);
		for(int m=0;m<10;m++)
		{
			I_SI522A_IO_Write(0x20, (0x06 << 2) | 0x40);		
			temp = I_SI522A_IO_Read(0x0F);
			
			if(	temp	==	0) 	break;          //处在接近的VCON值附近值，如果偶合出现0值，均有概率误触发，应舍弃该值。
				
			temp_Compare=(temp_Compare+temp)/2;		
			delayMicroseconds(1);
		}		
		
		if(temp_Compare == 0 || temp_Compare == 0x7f) //比较当前值和所存值
		{

		}
		else
		{
			if(temp_Compare < ACDConfigRegC_Val)
			{
				ACDConfigRegC_Val = temp_Compare;
				ACDConfigRegK_Val = VCON_TR[i];
			}
		}
	}
	ACDConfigRegK_RealVal	=	ACDConfigRegK_Val;     //取得最接近的参考电压VCON
	
	
	for(int j=0; j<4; j++)
	{
		I_SI522A_IO_Write(0x20, (0x0a << 2) | 0x40);		
		I_SI522A_IO_Write(0x0F, j*32+ACDConfigRegK_Val);
		
		I_SI522A_IO_Write(0x20, (0x06 << 2) | 0x40);
		temp_Compare = I_SI522A_IO_Read(0x0F);
		for(int n=0;n<10;n++)
		{
			I_SI522A_IO_Write(0x20, (0x06 << 2) | 0x40);		
			temp = I_SI522A_IO_Read(0x0F);
			temp_Compare=(temp_Compare+temp)/2;		
			delayMicroseconds(1);
		}		
		TR_Compare[j] = temp_Compare;
	}//再调TR的档位，将采集值填入TR_Compare[]

	for(int z=0; z<3; z++)                   //TR有四档可调，但是最大档的时候，电源有抖动，可能会导致ADC的值抖动较大，造成误触发
	{
		if(TR_Compare[z] ==0x7F)         
		{
			
		}
		else
		{
			ACDConfigRegC_Val = TR_Compare[z];//最终选择的配置
			ACDConfigRegK_Val = ACDConfigRegK_RealVal + z*32;
		}
	}//再选出一个非7f大值
	
	
	I_SI522A_IO_Write(0x20, (0x0a << 2) | 0x40);
	printf("\r\n ACDConfigRegK_Val:%02x ",ACDConfigRegK_Val);	
	
	I_SI522A_SetBitMask(0x01, 0x06);		//关闭ADC_EXCUTE
}



void PCD_ACD_Init1(void)
{
	I_SI522A_IO_Write(0x05, 0x60);	//清中断，该处不清中断，进入ACD模式后会异常产生有卡中断。
	I_SI522A_IO_Write(0x20, (0x09 << 2) | 0x40);		
	I_SI522A_IO_Write(0x0F, 0x55);	//Clear ACC_IRQ
	I_SI522A_IO_Write(0x20, (0x00 << 2) | 0x40);          //设置轮询时间
	I_SI522A_IO_Write(0x0F, 0x00 );
	I_SI522A_IO_Write(0x20, (0x01 << 2) | 0x40);					//设置相对模式或者绝对模式
	I_SI522A_IO_Write(0x0F, 0xA8 );
	I_SI522A_IO_Write(0x20, (0x02 << 2) | 0x40);					//设置无卡场强值


	I_SI522A_IO_Write(0x0F, ACDConfigRegC_Val );

	I_SI522A_IO_Write(0x20, (0x03 << 2) | 0x40);					//设置灵敏度，一般建议为4，在调试时，可以适当降低验证该值，验证ACD功能
	I_SI522A_IO_Write(0x0F, 0x02 );
	I_SI522A_IO_Write(0x20, (0x07 << 2) | 0x40);					//设置看门狗定时器时间
	I_SI522A_IO_Write(0x0F, 0x26 );
	I_SI522A_IO_Write(0x20, (0x08 << 2) | 0x40);         
	I_SI522A_IO_Write(0x0F, 0x00 );	
	I_SI522A_IO_Write(0x20, (0x0a << 2) | 0x40);					//设置ADC的基准电压和放大增益

	I_SI522A_IO_Write(0x0F, ACDConfigRegK_Val );

	I_SI522A_IO_Write(0x20, (0x0c << 2) | 0x40);					//设置监测ACD功能是否产生场强，意外产生可能导致读卡芯片复位或者寄存器丢失
	I_SI522A_IO_Write(0x0F, 0x00 );


  I_SI522A_IO_Write(0x20, (0x0e << 2) | 0x40);					//设置ACD模式下相关功能的标志位传导到IRQ引脚
	I_SI522A_IO_Write(0x0F, 0x00 );
	I_SI522A_IO_Write(0x02, 0x80);												//ComIEnReg，DivIEnReg   设置IRQ选择下降沿
	I_SI522A_IO_Write(0x03, 0xC0);												
	//I_SI522A_IO_Write(0x20, (0x09 << 2) | 0x40);				//设置监测ACD功能下的重要寄存器的配置值，寄存器丢失后会立即产生中断
	//I_SI522A_IO_Write(0x0F, 0x55 );								// 写非0x55的值即开启功能，写0x55清除使能停止功能。
	I_SI522A_IO_Write(0x01, 0xb0);	//进入ACD
}





unsigned long lastInterruptTime = 0;  // 记录最后一次中断的时间
int acdflag = 0;



void ACD_init_Fun1(void)
{

lastInterruptTime = millis();
acdflag = 1;
//Serial.println("待机一次");
	PCD_SI522A_TypeA_Init1();	//Reader模式的初始化
	
	PCD_ACD_AutoCalc(); //自动获取阈值
	
	PCD_ACD_Init1();   //初始化ACD配置寄存器，并且进入ACD模式
}





void ACD_init_Fun2(void)
{

lastInterruptTime = millis();
acdflag = 1;
//Serial.println("待机一次");
	PCD_SI522A_TypeA_Init2();	//Reader模式的初始化
	
	//PCD_ACD_AutoCalc(); //自动获取阈值
	
	PCD_ACD_Init2();   //初始化ACD配置寄存器，并且进入ACD模式
}

void PCD_ACD_Init2(void)
{
	I_SI522A_IO_Write(0x05, 0x60);	//清中断，该处不清中断，进入ACD模式后会异常产生有卡中断。
	I_SI522A_IO_Write(0x20, (0x09 << 2) | 0x40);		
	I_SI522A_IO_Write(0x0F, 0x55);	//Clear ACC_IRQ
	I_SI522A_IO_Write(0x20, (0x00 << 2) | 0x40);          //设置轮询时间
	I_SI522A_IO_Write(0x0F, 0x00 );
	I_SI522A_IO_Write(0x20, (0x01 << 2) | 0x40);					//设置相对模式或者绝对模式
	I_SI522A_IO_Write(0x0F, 0xA8 );
	//I_SI522A_IO_Write(0x20, (0x02 << 2) | 0x40);					//设置无卡场强值


	//I_SI522A_IO_Write(0x0F, ACDConfigRegC_Val );

	//I_SI522A_IO_Write(0x20, (0x03 << 2) | 0x40);					//设置灵敏度，一般建议为4，在调试时，可以适当降低验证该值，验证ACD功能
	//I_SI522A_IO_Write(0x0F, 0x04 );
	//I_SI522A_IO_Write(0x20, (0x07 << 2) | 0x40);					//设置看门狗定时器时间
	//I_SI522A_IO_Write(0x0F, 0x26 );
	//I_SI522A_IO_Write(0x20, (0x08 << 2) | 0x40);         
	//I_SI522A_IO_Write(0x0F, 0x00 );	
	I_SI522A_IO_Write(0x20, (0x0a << 2) | 0x40);					//设置ADC的基准电压和放大增益

	I_SI522A_IO_Write(0x0F, ACDConfigRegK_Val );

	//I_SI522A_IO_Write(0x20, (0x0c << 2) | 0x40);					//设置监测ACD功能是否产生场强，意外产生可能导致读卡芯片复位或者寄存器丢失
	//I_SI522A_IO_Write(0x0F, 0x01 );


	//I_SI522A_IO_Write(0x20, (0x0e << 2) | 0x40);					//设置ACD模式下相关功能的标志位传导到IRQ引脚
	//I_SI522A_IO_Write(0x0F, 0x00 );
	I_SI522A_IO_Write(0x02, 0x80);												//ComIEnReg，DivIEnReg   设置IRQ选择下降沿
	I_SI522A_IO_Write(0x03, 0xC0);												
	//I_SI522A_IO_Write(0x20, (0x09 << 2) | 0x40);				//设置监测ACD功能下的重要寄存器的配置值，寄存器丢失后会立即产生中断
	//I_SI522A_IO_Write(0x0F, 0x55 );								// 写非0x55的值即开启功能，写0x55清除使能停止功能。
	I_SI522A_IO_Write(0x01, 0xb0);	//进入ACD
}


void PCD_SI522A_TypeA_Init2(void)
{
	//I_SI522A_IO_Init();	// Initializes the interface with Si522	


/*

     pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // 默认拉高片选引脚

  // 初始化SPI通信
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16); // 设置SPI时钟频率
  SPI.setDataMode(SPI_MODE0);           // 设置SPI模式（Mode 0）
  SPI.setBitOrder(MSBFIRST);

*/

	//delay(500);
	

  
	//I_SI522A_ClearBitMask(0x08, 0x08);  
	// Reset baud rates
	//I_SI522A_IO_Write(0x12, 0x00);
	//I_SI522A_IO_Write(0x13, 0x00);
	// Reset ModWidthReg
	//I_SI522A_IO_Write(0x24, 0x26);
	// RxGain:110,43dB by default;
	//I_SI522A_IO_Write(0x26, 0x68);
	// When communicating with a PICC we need a timeout if something goes wrong.
	// f_timer = 13.56 MHz / (2*TPreScaler+1) where TPreScaler = [TPrescaler_Hi:TPrescaler_Lo].
	// TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	//I_SI522A_IO_Write(0x2A, 0x80);// TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	//I_SI522A_IO_Write(0x2B, 0xa9);// TPreScaler = TModeReg[3..0]:TPrescalerReg
	//I_SI522A_IO_Write(0x2C, 0x03); // Reload timer 
	//I_SI522A_IO_Write(0x2D, 0xe8); // Reload timer 
	//I_SI522A_IO_Write(0x15, 0x40);	// Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting
	//I_SI522A_IO_Write(0x11, 0x3D);	// Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4)
	I_SI522A_IO_Write(0x01, 0x00);  // Turn on the analog part of receiver   
  I_SI522A_IO_Write(0x14,I_SI522A_IO_Read(0x14) | 0x03);	//Tx1RFEn=1  Tx2RFEn=1
		//delay(1);      //这里加一个小延时，会好一点！


}
int m = 0;


int chuankouacd = 0;
int chuankoudk = 0;

void readCard() {

if(acdflag)
{
  i= 0;
//Serial.print("l");
chuankouacd = 1;

}

else{
    //m++;  // 每次中断触发时，增加 m
    i = 1;
    //Serial.print("dk");
chuankoudk = 1;

}
    
}








/*

void t1Callback()
{

  de = (cm-pm);
  if(p == 0)
  {
if(Serial.read() == 0x4F){
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x01);

Serial.write(0x88);
  
  }
  }
  
}

Task t1(1, TASK_FOREVER, &t1Callback);
Scheduler runner;



*/
void huanxing()//	0xFD 0x00 0x01 0xFF
{
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x01);

Serial.write(0xFF);

}


void shuimian()//0xFD 0x00 0x01 0x88
{
  if(Serial.read() == 0x4F){
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x01);

Serial.write(0x88);
  }
}

void shuimian1()
{
 if(Serial.read() == 0x4F){

Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x0B);

Serial.write(0x01);

Serial.write(0x01);

Serial.write(0xD3);

Serial.write(0xEE);

Serial.write(0xD2);

Serial.write(0xF4);

Serial.write(0xCC);

//	0xD3 0xEE 0xD2 0xF4 0xCC 0xEC 0xCF 0xC2
Serial.write(0xEC);

Serial.write(0xCF);

Serial.write(0xC2);


 }
}
void chaxun()//0xFD 0x00 0x01 0x21
{
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x01);

Serial.write(0x21);

}
void hx()
{
  while(1)
      {
        chaxun();
        if (Serial.read() != 0x4E)
        {
        huanxing();
        break;
        }
      }
}



void x0() {
  //Serial.print("e1");
  if (TagData[0] == 0xCD && TagData[1] == 0xBB && TagData[2] == 0xC6 &&TagData[3] == 0xC6 && TagData[4] == 0xB5 && (d !=2))
  {
    d = 2;
    p = 1;
    if(de >= 1456)    // 如果距离上一个过了0.5s  
 { 
 int ww = 0;
      //Serial.print("2");
      analogWrite(A1,255);



hx();
      



/******************************************************/

    /*  delay(100);
      delay(100);
      huanxing();*/


/*******************实测无需如此长的延迟*******************/



      delay(100);
      hx();
      delay(100);
/*
  while(1)
      {
        chaxun();
        if (Serial.read() != 0x4E)
        {
        huanxing();
        break;
        }
      }
  //Serial.print("2");
  /*while(1)
      {
        delay(10);
        chaxun();
        if (Serial.read() == 0x4F)
        {
        huanxing();
        break;
        }
      }*/
      //analogWrite(A1,HIGH);/*


      /*
      while(1)
      {
        chaxun();
        if (Serial.read() != 0x4E)
        {
        huanxing();
        break;
        }
      }*/
     //delay(200);
      //digitalWrite(4,LOW);
    for (int y = 0; y < 18; ++y)
      DetectData[y] = TagData[y];
    jiance = 1;
    dpm = millis();
    

//Serial.println("11");




Serial.write(0xFD);
Serial.write(0x00);
Serial.write(18);//数据区长度必须对应
Serial.write(0x01);
Serial.write(0x01);
Serial.write(0xCD);
Serial.write(0xBB);
Serial.write(0xC6);
Serial.write(0xC6);
Serial.write(0xB5);
Serial.write(0xDA);
Serial.write(0xC8);
Serial.write(0xFD);
Serial.write(0xB5);
Serial.write(0xC0);
Serial.write(0xB7);
Serial.write(0xE2);
Serial.write(0xCB);
Serial.write(0xF8);
Serial.write(0xCF);
Serial.write(0xDF);






   // }  //从第11个开始为字
    memset(TagData, 0, sizeof(TagData));
    p = 0;
 }
 
else if(de < 1456)
{
  //Serial.print("3");
  jiance = 2;
 
//delay(1456 - de);
hx();
digitalWrite(A1,0);
delay(100);
digitalWrite(A1,255);



hx();

 delay(100);
    dpm = millis();
    jiance = 1;
    for (int y = 0; y < 18; ++y)
      DetectData[y] = TagData[y];
    /*
    for (int j = 11; j <43 ; j++) {
      Serial.write(TagData[j]);
      */


      
Serial.write(0xFD);
Serial.write(0x00);
Serial.write(18);//数据区长度必须对应
Serial.write(0x01);
Serial.write(0x01);
Serial.write(0xCD);
Serial.write(0xBB);
Serial.write(0xC6);
Serial.write(0xC6);
Serial.write(0xB5);
Serial.write(0xDA);
Serial.write(0xC8);
Serial.write(0xFD);
Serial.write(0xB5);
Serial.write(0xC0);
Serial.write(0xB7);
Serial.write(0xE2);
Serial.write(0xCB);
Serial.write(0xF8);
Serial.write(0xCF);
Serial.write(0xDF);





    //}  //从第11个开始为字
    memset(TagData, 0, sizeof(TagData));
    p = 0;
  }
  }
else if(TagData[0] != 0xCD || TagData[1] != 0xBB || TagData[2] != 0xC6 ||TagData[3] != 0xC6 || TagData[4] != 0xB5|| TagData[5]!= 0xDA)
  {
    d = 1;
    p = 1;
    c = 1;
    if (de >= 1456)  // 如果距离上一个过了0.5s
    {
      
      int ww = 0;
      //Serial.print("2");
      analogWrite(A1,255);


      hx();


delay(100);
hx();

      /*************************************************/
      /*delay(100);
      delay(100);
      huanxing();*/

/*********************实测无需如此长的延迟*****************************************/





      delay(100);
    
      dpm = millis();
      jiance = 1;
      //Serial.print("ee");
      for (j = 0; j < 18; j++) {
        if (TagData[j] == 0x00)
          break;
      } 

for(int jj = 0;jj <= j; jj++)
{
  if(ww == 0){
  ww = 1;
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(j+2);//数据区长度必须对应

Serial.write(0x01);

Serial.write(0x01);
}
Serial.write(TagData[jj]);
}
      memset(TagData, 0, sizeof(TagData));
      p = 0;
    }

    else if (de < 1456) {
      //Serial.print("3");
      int qq =0;
      jiance = 2;
     hx();
      analogWrite(A1,0);
      delay(100);
     // huanxing();
      analogWrite(A1,255);




      hx();




       delay(100);
      /*·
      while(1)
      {
        chaxun();
        if (Serial.read() == 0x4F)
        {
          delay(200);
        huanxing();
        break;
        }
      }
      */



/*
      huanxing();
      delay(200);

*/



      dpm = millis();
      jiance = 1;

      for (j = 0; j < 18; j++) {
        if (TagData[j] == 0x00)
          break;
      }


for(int jj = 0;jj <= j; jj++)
{
  if(qq == 0){
  qq = 1;
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(j+2);//数据区长度必须对应

Serial.write(0x01);

Serial.write(0x01);
}
Serial.write(TagData[jj]);
}





      memset(TagData, 0, sizeof(TagData));
      p = 0;
    }
  
}
}


void deng() {  // 如果 过了0.5s 且 没到下一个打卡 (jiance == 1)关灯
  if ((de >= 6000) && (jiance == 1))
    analogWrite(A1,0);
}
//如果 过了0.5s 且 没到下一个打卡 (jiance == 1);关灯










void setup()
{	
//runner.addTask(t1);
  //t1.enable();

pinMode(10,OUTPUT);
digitalWrite(10,LOW);

	// System Initialization
	//pinMode(3,OUTPUT);
pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH); // 默认拉高片选引脚

//pinMode(IRQ_PIN, OUTPUT);
 // digitalWrite(IRQ_PIN, HIGH); 
pinMode(A1,OUTPUT);
pinMode(5,OUTPUT);


//pinMode(4,OUTPUT);

  //7 9 10 全为 GND
  // 初始化SPI通信
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16); // 设置SPI时钟频率
  SPI.setDataMode(SPI_MODE0);           // 设置SPI模式（Mode 0）
 SPI.setBitOrder(MSBFIRST);//高位优先
	uint8_t aaa=0;
	Serial.begin(9600);	// Initializes the uart 115200 bps
	//LED_Init();					// Initializes LED
	//EXTIX_Init();				// Initializes external interrupt,do not enable
pinMode(IRQ_PIN,INPUT_PULLUP);
memset(TagData, 0, sizeof(TagData));
memset(DetectData, 0, sizeof(DetectData));

/*
  CLKPR = 0x80; // enable change of CLKPR (4 CLPS bits must be 0)
   CLKPR = 0x03; // set the CLKDIV to 8 (0b0011 = div by 8)
   delayMicroseconds(1000);
   */






attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);
for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }
ACD_init_Fun1();
//ACD_Fun();
delay(1000);
i = 0;
m = 0;
acdflag = 0;
delay(500);
digitalWrite(5,HIGH);
}













void dump_byte_array(byte *TagData) {
  int y = 0;

for(byte l =0; l < 18 ; l++)
{
if(DetectData[l] != TagData[l])
y = 1;
}


for(int ll =0; ll<18;++ll)
{
  DetectData[ll] = TagData[ll];
}


if(y)
{
  y=0;

//Serial.print("EE");
/*********************************************************************/
x0();

/*
  for(int k = 0;k < 18;++k)
  {
    Serial.print(TagData[k],HEX);
    Serial.print(" ");
  }
*/
/********************************************************************/






}



else;
//ACD_init_Fun1();



}




unsigned long tianxianc = 0;
unsigned long tianxianp= 0;
int txflag = 1;
int zhenkongqi = 0;
int zh = 0;


void loop(){

if(millis() - lastInterruptTime >= 1)       //
acdflag = 0;

  //runner.execute();
if(chuankouacd)
{
  //Serial.println("chuankouacd");
  chuankouacd = 0;
}
if(chuankoudk)
{
  //Serial.println("chuankoudk");
  chuankoudk = 0;
}




  dcm = millis();
  cm = millis();
//if((millis() - tianxianp) >= 700 && (millis() - tianxianp) <= 701)  //自毁程序     只执行一次,根据晶振频率选择范围
if(millis() - tianxianp >= 1000)
{
  analogWrite(A1,0);
  if(zh)
  {
    zh = 0;
  i = 0;
 zhenkongqi = 0;
  txflag = 1;
  //Serial.println("自会一次");
  ACD_init_Fun1();
  }
}



 de = (cm-dpm);
  if(p == 0)
  {
if(Serial.read() == 0x4F){
  p = 1;
Serial.write(0xFD);

Serial.write(0x00);

Serial.write(0x01);

Serial.write(0x88);
  
  }
  }
  




//Serial.println(tianxianc - tianxianp);
//Serial.println(de);










if(i || zhenkongqi)//中断触发  readcard里面设定好了一定为读卡产生中断
{


//Serial.print("IRQ");
  if(txflag)       //txflag用来只触发一次初始化
  {

zh = 1;
tianxianp = millis();

//Serial.println("打开天线");

//zhenkongqi = 1;
    txflag =0;
 // mfrc522.PCD_Init(); //初始化天线  *1
  //Serial.println("初始化天线");
  mfrc522.PCD_Init(); 
  activateRec(mfrc522); //打开接收    *1
  //Serial.print("初始化接受");
  mfrc522.PICC_ReadCardSerial();   //读卡模式    *1
  //Serial.println("读卡模式");
 
  }

//Serial.print(millis() - tianxianp);

 if(millis() - tianxianp >= 600)
 {
  if(zh)
  {
  i = 0;
 zhenkongqi = 0;
  txflag = 1;
  zh = 1;
  //Serial.println("自会一次内部");
  //ACD_init_Fun();
  }
  return ;
}

//Serial.println("第一次通过前");

if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

//Serial.println("第一次通过");
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
//Serial.println("第二次通过");



byte sector = 1;           // Sector 1
    byte startBlockAddr = 4;   // Start reading from block 4 (sector 1)
    //byte TagData[18];
    byte size = sizeof(TagData);

    MFRC522::StatusCode status;

    // Authenticate using key A
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, startBlockAddr + 3, &key, &(mfrc522.uid));
    
   if (status != MFRC522::STATUS_OK) {
mfrc522.PICC_HaltA(); // Halt the card and allow retry
        mfrc522.PCD_StopCrypto1(); // Stop the encryption
        return ;
   }


//Serial.print("第三次通过");
status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(4, TagData, &size);


if (status != MFRC522::STATUS_OK) {
            mfrc522.PICC_HaltA(); // Halt the card and allow retry
            mfrc522.PCD_StopCrypto1(); // Stop the encryption
       return ;
        }



/////////////////////////////////////////////////////////////////////////////////////////////////////
//读卡成功后的工作
zh = 0;
i = 0;
zhenkongqi = 0;
txflag = 1;


///////////////////////////////////////////////////////////////////////////////////////////////////////


/***********提前结束真空期**************/

ACD_init_Fun1();    //读卡成功后关闭天线进入acd
//Serial.println("yesss");
dump_byte_array(TagData);
//读卡成功，提前关闭接收中断，直到下一次读卡中断


 
/*******************防抖动******************/







}

de = (cm - dpm);





}



///  txflag acdflag   谁最后结束？是读卡 还是  超过真空期？///


//方案一   只设定一个初始真空期（在每一段中断域内）    读卡成功，提前结束真空期    //////
//方案二   每次终端都有一个真空期（很短），最后一次   触发中断 后 经过这个很短的真空期，结束接收  进入ACD   //////


//超级大问题   待机之后 读卡  发出一个l  以后天线敞开无法读卡/////






//  1MHZ问题：  IRQ第一次通过循环时，内部自会 会 bug////


//1MHZ要单独加官方的init   16MHZ不需要////


//  最后一点 ： 最多读9个字    后期可能要改

/******************************************完成版**********************************/



/***************场地实验完美*********************************************/



/***************************代码从来没有过问题   所有问题都是接触不良！！》》************/////////////

//亮灯灭灯没有读  常开