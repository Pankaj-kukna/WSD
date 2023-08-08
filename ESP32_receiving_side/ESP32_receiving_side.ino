#define DEBUG_ESP              //comment out to deactivate debug console

#ifdef DEBUG_ESP
  #define pDBGln(x) Serial.println(x)
  #define pDBG(x)   Serial.print(x)
#else 
  #define pDBG(...)
  #define pDBGln(...)
#endif

#include "Arduino.h"
#include "FS.h"  
#include "SD.h"
#include "SPI.h"              // SD Card ESP32
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include "SerialTransfer.h"

SerialTransfer myTransfer;
HardwareSerial Comm(1);

#define EEPROM_SIZE 1
int pictureNumber = 0;

struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;

uint16_t packetCounter=1;
uint16_t bufferPointer=0;

uint8_t tempImageBuffer[32000];

void setup(){
  //Serial.begin(Baud Rate, Data Protocol, Txd pin, Rxd pin);
  Serial.begin(921600);                                                     // Define and start serial monitor
  // 115200,256000,512000,962100
  //Serial2.begin(512000, SERIAL_8N1,32,33); //Receiver_Txd_pin, Receiver_Rxd_pin); // Define and start Receiver serial port
  //myTransfer.begin(Serial2);  
  Comm.begin(512000, SERIAL_8N1,32,33);  
  myTransfer.begin(Comm);

if(SD.begin(5)){
  pDBGln("Starting SD Card");}
else{
  pDBGln("Card Mount Failed");
  return;
   }
uint8_t cardType = SD.cardType();

if(cardType == CARD_NONE){
  pDBGln("No SD card attached");
  return;
}
}

void loop(){
  if(myTransfer.available())  {
    myTransfer.packet.rxObj(ImgMetaData, sizeof(ImgMetaData));
    pDBG("Struct Data: ");
    pDBG(ImgMetaData.counter);
    pDBG(", ");
    pDBG(ImgMetaData.imSize);
    pDBG(", ");  
    pDBG(ImgMetaData.numLoops);
    pDBG(", ");
    pDBG(ImgMetaData.sizeLastLoop);
    pDBG(", PacketCounter_recieve: ");
    pDBGln(packetCounter);  

    if(ImgMetaData.counter==1){  
      copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));
    }else{
      if(ImgMetaData.counter==packetCounter){  
        if(packetCounter<ImgMetaData.numLoops){        
          copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));
        }else if(ImgMetaData.counter==packetCounter){
          copyToImageBuff(ImgMetaData.sizeLastLoop);     
        }
      }
    }
 
    if(packetCounter>ImgMetaData.numLoops){  
      pDBG("ERROR: ");
      SD_upload();     
      packetCounter=1;
      bufferPointer=0;
      delay(2000);
      //while(1){}
    }  

  }else if(myTransfer.status < 0) { 
    pDBG("ERROR: ");
    if(myTransfer.status == -1)
      pDBGln(F("CRC_ERROR"));
    else if(myTransfer.status == -2)
      pDBGln(F("PAYLOAD_ERROR"));
    else if(myTransfer.status == -3)
      pDBGln(F("STOP_BYTE_ERROR"));
  }
}
  

void copyToImageBuff(uint16_t dataLenght){
   pDBGln(F("Sent:"));

   pDBG("copt to image buffer opened");
  for(int y=0;y<dataLenght;y++){
  tempImageBuffer[bufferPointer+y] = myTransfer.packet.rxBuff[y+sizeof(ImgMetaData)];
  } 
  bufferPointer+=dataLenght;
  packetCounter++;  

  pDBG("dataLenght: ");
  pDBG(dataLenght);  
  pDBG(", bufferPointer: ");
  pDBGln(bufferPointer);
}

void printBuf(char localBuff){
  pDBG(F("Pixel Values: { "));
  for (uint16_t k=0; k<sizeof(myTransfer.packet.rxBuff); k++){
    pDBG(myTransfer.packet.rxBuff[k]);
    if (k < (sizeof(myTransfer.packet.rxBuff) - 1))
      pDBG(F(", "));
    else
      pDBGln(F(" }"));
  }
}

void SD_upload(){
  pDBG("Uploading via SD: "); 
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";

  fs::FS &fs = SD; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    pDBGln("Failed to open file in writing mode");
  } 
  else {
    file.write(tempImageBuffer, ImgMetaData.imSize); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();

  pDBGln("Done... Saving to sdcard...");
  delay(100);
}