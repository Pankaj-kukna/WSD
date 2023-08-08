#include<Wire.h>
#include<ADXL345_WE.h>
const int int2Pin = 2;
volatile bool freeFall = false;

ADXL345_WE myAcc = ADXL345_WE();
void setup(){
  Wire.begin();
  Serial.begin(9600);
  pinMode(int2Pin, INPUT);
  Serial.println("ADXL345_Sketch - Free Fall Interrupt");
  Serial.println();
  if(!myAcc.init()){
    Serial.println("ADXL345 not connected!");
  }

  myAcc.setDataRate(ADXL345_DATA_RATE_3200);
  myAcc.setRange(ADXL345_RANGE_16G);
  myAcc.setFreeFallThresholds(0.4, 100);
 
/*
    ADXL345_ACTIVITY     -   acc. value of included axes are > THRESH_ACT
*/
  myAcc.setInterrupt(ADXL345_FREEFALL, INT_PIN_2);
  
  attachInterrupt(digitalPinToInterrupt(int2Pin), freeFallISR, RISING); 
  freeFall=false; 
}

void loop() {
byte intType = myAcc.readAndClearInterrupts();
if(myAcc.checkInterrupt(intType, ADXL345_FREEFALL)){
Serial.println("FREEFALL confirmed");
}}


void freeFallISR(){
  freeFall = true;
}