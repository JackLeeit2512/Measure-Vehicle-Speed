#include <TrTmonitor.h>

TrTmonitor Monitor(0);

//khai báo các biến để đồng bộ truyền nhận Arduino và App
int16_t goimaytinh1, goimaytinh2; //gửi lên đồng hồ READ1 & READ2 của App
uint8_t LEDgoimaytinh; //gửi hiển thị LED trên App
uint8_t nhanMaytinh1, nhanMaytinh2, nutMaytinh; //WRITE1 & WRITE2 && nút nhấn trên App

//Khai báo các biến bit để dễ xử lý
unsigned char NUT1, NUT2, NUT3, NUT4;
unsigned char LED1, LED2, LED3, LED4;

unsigned char LED10 = 10;

uint32_t demXung, tanSo;
uint32_t tdHienTai, tdTr, chuKy;
uint32_t vanToc, quangDuong;

int16_t doSangLed;
int16_t kqADC6, Qmax;

//-----MOTOR-----------
#define B1 8
#define B2 9
#define B3 6
#define B4 5

#define B1On digitalWrite(B1, HIGH)
#define B2On digitalWrite(B2, HIGH)
#define B3On digitalWrite(B3, HIGH)
#define B4On digitalWrite(B4, HIGH)

#define B1Off digitalWrite(B1, LOW)
#define B2Off digitalWrite(B2, LOW)
#define B3Off digitalWrite(B3, LOW)
#define B4Off digitalWrite(B4, LOW)

void chayThuan(unsigned char _time){
  _time = constrain(_time,5,50);
  B1On; B2Off; B3Off; B4Off; delay(_time);
  B1Off; B2On; B3Off; B4Off; delay(_time);
  B1Off; B2Off; B3On; B4Off; delay(_time);
  B1Off; B2Off; B3Off; B4On; delay(_time);
}
void chayNguoc(unsigned char _time){
  _time = constrain(_time,5,50);
  B1Off; B2Off; B3On; B4Off; delay(_time);
  B1Off; B2On; B3Off; B4Off; delay(_time);
  B1On; B2Off; B3Off; B4Off; delay(_time);  
  B1Off;B2Off; B3Off; B4On;  delay(_time);
}
void dung(unsigned char _time){
  B1Off; B2Off; B3Off; B4Off; delay(_time);delay(_time);delay(_time);delay(_time);
}

signed int viTriSet, viTri; 
char SW1, preSW1, ttSW1=0;
char SW3, preSW3;
//---------------------
void ngatNgoai1(){
  demXung++;
  tdTr = tdHienTai;
  tdHienTai = micros();
  chuKy = tdHienTai - tdTr;
}
void setup() {
  attachInterrupt(digitalPinToInterrupt(3), ngatNgoai1, FALLING);
  pinMode(LED10 , OUTPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);

  pinMode(B1,OUTPUT);
  pinMode(B2,OUTPUT);
  pinMode(B3,OUTPUT);
  pinMode(B4,OUTPUT);

  Monitor.begin(19200);
  
}

void loop() {
  preSW3 = SW3;
  SW3 = digitalRead(7);
  kqADC6 = analogRead(A6);
  Qmax = map(kqADC6, 0, 1023, 40, 400);
//-----------
  if(Qmax <= 60){
    doSangLed = 0;
    analogWrite(LED10, doSangLed);
  }
  else if(Qmax >= 300){
    doSangLed = 255;
    analogWrite(LED10, doSangLed);
  }
  else{
    doSangLed = map(Qmax, 60, 300, 0, 255);
    analogWrite(LED10, doSangLed);
  }
//---------------------
  if(NUT1 == 1){
    demXung = 0;
  }
  
  if(micros()>(tdHienTai + 1000000)){
    tanSo = 0;
    vanToc = 0;
  }
  else{
    tanSo = 1000000/chuKy;
    vanToc = 1080000/chuKy; 
  }
  // bang 0.3*3.6*1000000/chuKy
  
//-------------
  
  viTriSet = vanToc;
  if(vanToc>100){
    viTriSet = 130;
  }

  if(viTri < viTriSet){
    viTri++;
    chayThuan(5);
  }
  else if(viTri > viTriSet){
    viTri--;
    chayNguoc(5);
  }
  else{
    if(NUT1 == 1){
      chayThuan(5);
    }
    else if(NUT2 == 1){
      chayNguoc(5);
    }
    else{
      dung(5);
    }
  }

  preSW1 = SW1;
  SW1 = digitalRead(4);
  if(ttSW1 == 0){
    goimaytinh1 = vanToc;
    goimaytinh2 = quangDuong;
    LED2 =0;
  }
  else{
    goimaytinh1 = viTri;
    goimaytinh2 = Qmax;
    LED2 =1;
  }
  if(SW1 == 0 && preSW1 != 0 && ttSW1 == 0){
      ttSW1 = 1 -ttSW1;
  } 
  else if(SW1 == 0 && preSW1 != 0 && ttSW1 == 1){
    ttSW1 = 1 -ttSW1;
  }
  if(SW3 == 0 && preSW3 != 0){
    demXung = 0;
    quangDuong = 0;
    LED1 = 0;
  }
  else{
    if(quangDuong < Qmax){
      quangDuong = demXung*3/10; 
    }
    else{
      LED1 = 1;
    }
  }
//-------------
  /*goimaytinh1 = doSangLed;
  goimaytinh2 = quangDuong;  */
  // đóng gói các bit vào byte trước khi gửi
  LEDgoimaytinh = LED1 + LED2*2 + LED3*4 + LED4*8; 
  
  //đồng bộ Arduino & App (truyền nhận Arduino & App): cần tối thiểu 20ms
  Monitor.synch(nhanMaytinh1, nhanMaytinh2, nutMaytinh, goimaytinh1, goimaytinh2, LEDgoimaytinh);
  
  // nhan tin hieu tu may tinh
  
  NUT1 = (nutMaytinh&1);
  NUT2 = (nutMaytinh&2)>>1;
  NUT3 = (nutMaytinh&4)>>2;
  NUT4 = (nutMaytinh&8)>>3;

  //delay(20);
}