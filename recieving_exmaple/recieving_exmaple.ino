#include "SerialTransfer.h"
#include "Arduino.h"

SerialTransfer myTransfer;

struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;

void setup()
{
  Serial.begin(115200);
  Serial2.begin(962100); 
  myTransfer.begin(Serial2);
}

void loop()
{
  if (myTransfer.available())
  {  
  Serial.println(ImgMetaData.counter);
  Serial.println(ImgMetaData.imSize);
  Serial.println(ImgMetaData.numLoops);
  Serial.println(ImgMetaData.sizeLastLoop);
  myTransfer.rxObj(ImgMetaData);
  Serial.println(ImgMetaData.counter);
  Serial.println(ImgMetaData.imSize);
  Serial.println(ImgMetaData.numLoops);
  Serial.println(ImgMetaData.sizeLastLoop);
  }
}