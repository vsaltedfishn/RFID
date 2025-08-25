
#include <Arduino.h>
#include <SPI.h>
#include <mfrc630.h>
#include <Enerlib.h>
#include <SoftwareSerial.h>


Energy energy;

#define CHIP_SELECT 6
uint8_t gain = 0;
byte TagData[18];

unsigned long pm = 0;
void mfrc630_SPI_transfer(const uint8_t* tx, uint8_t* rx, uint16_t len) {
  for (uint16_t i=0; i < len; i++){
    rx[i] = SPI.transfer(tx[i]);
  }
}
void mfrc630_SPI_select() {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));  // gain control of SPI bus
  digitalWrite(CHIP_SELECT, LOW);
}
void mfrc630_SPI_unselect() {
  digitalWrite(CHIP_SELECT, HIGH);
  SPI.endTransaction();    // release the SPI bus
}
void print_block(uint8_t * block,uint8_t length){
    for (uint8_t i=0; i<length; i++){
        if (block[i] < 16){
        } else {
        }
    }
}




uint8_t preuid[10] = {0}; //Initialized uid: waiting for seting
uint8_t curuid[10] = {0};
int LFlag = 0;
int DFlag = 0;
int FFlag = 0;
void mfrc630_MF_example_dump_arduino() {
  uint16_t atqa = mfrc630_iso14443a_REQA();
 // Serial.println("UID前一步");
  if (atqa != 0) {  // Are there any cards that answered?
  //Serial.println("uid yes");
 
  DFlag = 1;
    uint8_t sak;
    // Select the card and discover its uid.
    uint8_t uid_len = mfrc630_iso14443a_select(curuid, &sak);

    if (uid_len != 0) {  
for(int i = 0; i < 9; i++)
{
if(preuid[i] != curuid[i])
LFlag = 1;
}
     if(LFlag)
     {
analogWrite(A1,255);
      uint8_t FFkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

      mfrc630_cmd_load_key(FFkey);  // load into the key buffer

      // Try to athenticate block 0.
      if (mfrc630_MF_auth(curuid, MFRC630_MF_AUTH_KEY_A, 4)) {

        // Attempt to read the first 4 blocks.
        uint8_t readbuf[16] = {0};
        uint8_t len;
        
          len = mfrc630_MF_read_block(4, readbuf);
for(int i = 0; i <15 ;i++)
          {
            TagData[i] = readbuf[i];
          }
        for(int i = 0; i < 9; i++)
      {
        preuid[i] = curuid[i];          
       }
        FFlag = 1;
        mfrc630_MF_deauth();  // be sure to call this after an authentication!
      }else {}
      } else {}
    } else {}
  } else {}

   LPCD_ON();
}

uint8_t QMin;
uint8_t QMax;
uint8_t IMin;
  uint8_t I = 0;
uint8_t Q = 0;




void LPCD_Calculate1(){//TH决定窗口宽度，一般取1
  mfrc630_AN11145_start_IQ_measurement();
  delay(500);
  uint8_t cI = mfrc630_read_reg(MFRC630_REG_LPCD_I_RESULT);
uint8_t cQ = mfrc630_read_reg(MFRC630_REG_LPCD_Q_RESULT);//demo板无卡30 29

Serial.write(0xFD);
Serial.write(0x00);
Serial.write(0x03);//数据区长度必须对应
Serial.write(0x01);
Serial.write(0x01);
Serial.write(cI);

delay(500);
Serial.write(0xFD);
Serial.write(0x00);
Serial.write(0x03);//数据区长度必须对应
Serial.write(0x01);
Serial.write(0x01);
Serial.write(cQ);


// if(Q>cQ)
Q = cQ;
// if(I>cI)
I = cI;

pt();
delay(500);
// Serial.println(cI);
// Serial.println(cQ);
  mfrc630_AN11145_stop_IQ_measurement();
}


void LPCD_Calculate2(int TH){
//  I = 32;
//  Q = 36;

   I= I;
   Q = Q;
// 
//  I = 32;
//  Q = 27;


//delay(500);
  uint8_t bQMin = Q - TH ;
  uint8_t bQMax = Q + TH ;
  uint8_t bIMin = I - TH ;
  uint8_t bIMax = I + TH ;
  QMin = bQMin | ((bIMax & 0x30) << 2);//9B  00110000
  QMax = bQMax | ((bIMax & 0x0C) << 4);//1F  00001100
  IMin = bIMin | ((bIMax & 0x03) << 6);//1C  00000011
}

int TXH = 1;






void ClearIRQ(){
  mfrc630_write_reg(0x06, 0x7F);
  mfrc630_write_reg(0x07, 0x7F);
}

void LPCD_ON(){
  // LPCD_config
  mfrc630_write_reg(0x3F, QMin);  // Set Qmin register
  mfrc630_write_reg(0x40, QMax);  // Set Qmax register
  mfrc630_write_reg(0x41, IMin);  // Set Imin register
  // mfrc630_write_reg(0x28, 0x89);  // set DrvMode register
  // Execute trimming procedure
  mfrc630_write_reg(0x1F, 0x07);  // Write default. T3 reload value Hi
  mfrc630_write_reg(0x20, 0xF2);  // Write default. T3 reload value Lo
  mfrc630_write_reg(0x24, 0x00);  // Write min. T4 reload value Hi
  mfrc630_write_reg(0x25, 0x10);  // Write min. T4 reload value Lo
  mfrc630_write_reg(0x23, 0xDF);  // Config T4 for AutoLPCD&AutoRestart.Set AutoTrimm bit.Start T4.
  mfrc630_write_reg(0x43, 0x40);  // Clear LPCD result
  mfrc630_write_reg(0x38, 0x52);  // Set Rx_ADCmode bit
  uint8_t gian = mfrc630_read_reg(0x39);
  mfrc630_write_reg(0x39, 0x03);  // Raise receiver gain to maximum
  if(!(mfrc630_read_reg(0x23) & 0x80)){
    delay(5);
  }
  mfrc630_write_reg(0x00, 0x00);
  mfrc630_write_reg(0x02, 0xB0);  // Flush FIFO
  mfrc630_write_reg(0x06, 0x7F);
  mfrc630_write_reg(0x07, 0x7F);
  mfrc630_write_reg(0x08, 0x00);
  mfrc630_write_reg(0x09, 0xE0);
  mfrc630_write_reg(0x00, 0x01);

  if(mfrc630_read_reg(0x07) && 0x20){
  }
pm = millis();
}





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



int ISR = 0;

void readcard()
{
ISR=1;
}



void x0()
{


//analogWrite(A1,255);



  // hx();
  delay(200);
  // hx();
  delay(100);

// int j = 0;
// for (j = 0; j < 18; j++) {
//         if (TagData[j] == 0x00)
//           break;
//       } 

// for(int jj = 0;jj <= j; jj++)
// {
  
// Serial.write(0xFD);

// Serial.write(0x00);

// Serial.write(j+2);//数据区长度必须对应

// Serial.write(0x01);

// Serial.write(0x01);

// Serial.write(TagData[jj]);
// }

analogWrite(A1,0);

}


void pt()
{
//   Serial.print("I: ");
//   Serial.println(I);
// Serial.print("Q: ");
// Serial.println(Q);
// Serial.println("");
int Ia = I;
digital_broadcast(Ia);
delay(1500);
int Qa = Q;
digital_broadcast(Qa);
}



void digital_broadcast(int number)
{
   String number_s=String(number);
   byte number_array[number_s.length()];
   for(int i=0;i<number_s.length();i++)
   {
    number_array[i]=byte(number_s.charAt(i));
   }
   byte final_array[5+number_s.length()];
   final_array[0]=0xFD;
   final_array[1]=0x00;
   final_array[2]=0x02+number_s.length();
   final_array[3]=0x01;
   final_array[4]=0x01;
   for(int i=0;i<number_s.length();i++)
   {
    final_array[i+5]=number_array[i];
   }
   int length=sizeof(final_array)/sizeof(final_array[0]);
   for(int i=0;i<length;i++)
  {
    Serial.write(final_array[i]);
  }
}


int lpcdflag = 1;

void PowerDown_1()
{
  if(lpcdflag)
  energy.PowerDown();

}




void setup(){
  // Start serial communication.
  Serial.begin(115200);

  pinMode(0,INPUT_PULLUP);
  pinMode(A1,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(CHIP_SELECT, OUTPUT);
  pinMode(2, INPUT);
  pinMode(7,OUTPUT);
  digitalWrite(7,LOW);
 


  if(digitalRead(0))
  lpcdflag = 0;
  else
  lpcdflag = 1;


// lpcdflag = 1;


// mySerial.begin(115200);
  SPI.begin();
  mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);

/*************************/
 mfrc630_AN11145_start_IQ_measurement();
  delay(10);
 I = mfrc630_read_reg(MFRC630_REG_LPCD_I_RESULT);
 Q = mfrc630_read_reg(MFRC630_REG_LPCD_Q_RESULT);//demo板无卡30 29

 
delay(100);
  mfrc630_AN11145_stop_IQ_measurement();
/*************************************/

attachInterrupt(digitalPinToInterrupt(2), readcard, CHANGE);

// 0:默认高功耗
if(lpcdflag)
I++;
else 
I=I+2;

delay(100);
LPCD_Calculate2(TXH);
delay(100);




  LPCD_ON();
  ClearIRQ();

  digitalWrite(5,HIGH);
  PowerDown_1();
  



}



void loop(){
  if(digitalRead(2) == 1){
     //Serial.println("Detected Card!");
    mfrc630_write_reg(0x00, 0x1F);
    mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
    mfrc630_MF_example_dump_arduino();
  }

if(DFlag) //接收uid
{

  x0();

DFlag = 0;
if(LFlag)
{
 LFlag = 0;
}
if(FFlag)
{
  FFlag = 0;
//n++;
//Serial.println(n);


}
else if(!FFlag)
{

}
}

 if(millis() - pm > 560 && millis() - pm < 650)
 {
PowerDown_1();
 }

}
