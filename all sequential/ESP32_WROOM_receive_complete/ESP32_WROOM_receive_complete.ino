#include "SerialTransfer.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD.h"                // SD Card ESP32
#include "SPI.h"               // SD Card ESP32
#include "driver/rtc_io.h"

int pictureNumber = 0;

#define SD_CS    14
#define SD_MOSI  15
#define SD_SCK   18
#define SD_MISO  19

SPIClass spi = SPIClass(HSPI);
SerialTransfer myTransfer;

struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;

uint16_t packetCounter=1;
uint16_t bufferPointer=0;
uint8_t tempImageBuffer[40000];

void setup(){
  //Serial.begin(Baud Rate, Data Protocol, Txd pin, Rxd pin);
  Serial.begin(115200);                                                     // Define and start serial monitor
  // 115200,256000,512000,962100
  Serial2.begin(962100,SERIAL_8N1,16,17); //Receiver_Txd_pin, Receiver_Rxd_pin); // Define and start Receiver serial port
  myTransfer.begin(Serial2);
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
 // Serial.println("reached point 0");

}

void loop(){
  if(myTransfer.available()){
    myTransfer.rxObj(ImgMetaData);
/*
    Serial.println(ImgMetaData.counter);
    Serial.println(ImgMetaData.imSize);
    Serial.println(ImgMetaData.numLoops);
    Serial.println(ImgMetaData.sizeLastLoop);
*/
    Serial.println("reached point 1");
        if(ImgMetaData.counter==1){  
          Serial.println("reached point 2");
          copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));   
        }else{if(packetCounter<ImgMetaData.numLoops){   
                  Serial.println(packetCounter);
                  Serial.println(ImgMetaData.counter); 
                  Serial.println();
                 // Serial.println(ImgMetaData.numLoops);     
                  Serial.println("reached point 5");
                  copyToImageBuff(MAX_PACKET_SIZE-sizeof(ImgMetaData));
                }else if(ImgMetaData.counter==packetCounter){
                  Serial.println("reached point 6");
                  copyToImageBuff(ImgMetaData.sizeLastLoop);                 
          }
    } 
 
    if(packetCounter>ImgMetaData.numLoops){  
        Serial.println("reached point 7");
        printBuf(tempImageBuffer);
        pictureNumber += 1;
        // Path where new picture will be saved in SD Card
        String path = "/picture" + String(pictureNumber) + ".jpeg";
        fs::FS &fs = SD;
        Serial.printf("Picture file name: %s\n", path.c_str()); 
        File file = fs.open(path, FILE_WRITE);
        if (!file) {
          Serial.println("Failed to open file in writing mode");
        } else {
          file.write((uint8_t*)tempImageBuffer, ImgMetaData.imSize);  // payload (image), payload length
          Serial.printf("Saved file to path: %s\n", path.c_str());
        }
        file.close();        
            packetCounter=1;
            bufferPointer=0;
      } }
/*
      else if(myTransfer.status < 0) { 
    Serial.println("ERROR: ");
    if(myTransfer.status == -1)
      Serial.println(F("CRC_ERROR"));
    else if(myTransfer.status == -2)
      Serial.println(F("PAYLOAD_ERROR"));
    else if(myTransfer.status == -3)
      Serial.println(F("STOP_BYTE_ERROR"));
  }*/
      }

void copyToImageBuff(uint16_t dataLenght){
  Serial.println("reached point 3");
  for(int y=0;y<dataLenght;y++){
    tempImageBuffer[bufferPointer+y] = myTransfer.packet.rxBuff[y+sizeof(ImgMetaData)];
    //Serial.print(myTransfer.packet.rxBuff[y+sizeof(ImgMetaData)]);
    /*Serial.print(",");
    Serial.print(tempImageBuffer[bufferPointer+y]);
    Serial.print(",");
    Serial.print(y+sizeof(ImgMetaData));
    Serial.print(",");
    Serial.print(bufferPointer+y);
    Serial.println();*/
  //Serial.println("reached point 4");  
  } 
  bufferPointer+=dataLenght;
  packetCounter++;  
  //Serial.println(bufferPointer);  
}

void printBuf(uint8_t Buffer[]){
  Serial.println("Pixel Values: { ");
  for (uint16_t k=0; k<ImgMetaData.imSize; k++){
    Serial.print(Buffer[k]); 
  }
  Serial.println();
}