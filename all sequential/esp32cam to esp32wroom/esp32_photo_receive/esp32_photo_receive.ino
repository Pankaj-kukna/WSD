/*Receives picture via serial */

//#define DEBUG_ESP              //comment out to deactivate debug console

#ifdef DEBUG_ESP
  #define pDBGln(x) Serial.println(x)
  #define pDBG(x)   Serial.print(x)
#else 
  #define pDBG(...)
  #define pDBGln(...)
#endif

#include "SerialTransfer.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD.h"                // SD Card ESP32
#include "SPI.h"               // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>  // read and write from flash memory

#define SD_CS    14
#define SD_MOSI  15
#define SD_SCK   18
#define SD_MISO  19

SPIClass spi = SPIClass(HSPI);

// define the number of bytes you want to access
#define EEPROM_SIZE 1
// picture name
String pic_name;
int pictureNumber = 0;

SerialTransfer myTransfer;
struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;
uint16_t packetCounter=1;
uint16_t bufferPointer=0;
char tempImageBuffer[32000];

void setup(){
  //Serial.begin(Baud Rate, Data Protocol, Txd pin, Rxd pin);
  Serial.begin(115200);                                                     // Define and start serial monitor
  // 115200,256000,512000,962100
  Serial2.begin(962100, SERIAL_8N1); //Receiver_Txd_pin, Receiver_Rxd_pin); // Define and start Receiver serial port
  myTransfer.begin(Serial2);
 spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
   spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  Serial.print("MOSI: ");
  Serial.println(SD_MOSI);
  Serial.print("MISO: ");
  Serial.println(SD_MISO);
  Serial.print("SCK: ");
  Serial.println(SD_SCK);
  Serial.print("SS: ");
  Serial.println(SD_CS);  
  delay(1000);
    //Serial.println("Starting SD Card");
  if (!SD.begin(SD_CS,spi,80000000)) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD Card attached");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

}

void loop(){
  if(myTransfer.available())  {
    myTransfer.rxObj(ImgMetaData, sizeof(ImgMetaData));
    pDBG("Struct Data: ");
    pDBG(ImgMetaData.counter);
    pDBG(", ");
    pDBG(ImgMetaData.imSize);
    pDBG(", ");  
    pDBG(ImgMetaData.numLoops);
    pDBG(", ");
    pDBG(ImgMetaData.sizeLastLoop);
    pDBG(", PacketCounter: ");
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
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) + ".jpg";

  fs::FS &fs = SD;
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write((unsigned char*)tempImageBuffer, ImgMetaData.imSize);  // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close(); 
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
