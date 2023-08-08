#include <Wire.h>
#include <ADXL345_WE.h>

#include <EEPROM.h>
#define EEPROM_SIZE 1

#include <FS.h> 
#include <SD.h>
#include <SPI.h>
//spi bus pin definition
  #define VSPI_MISO   19
  #define VSPI_MOSI   23
  #define VSPI_SCLK   18
  #define VSPI_SS     5

  #define HSPI_MISO   12
  #define HSPI_MOSI   13
  #define HSPI_SCLK   14
  #define HSPI_SS     15

static const int spiClk = 2000000; // 2 MHz
//uninitalised pointers to SPI objects
SPIClass * vspi = NULL;
SPIClass * hspi = NULL;

#define BUTTON_PIN_BITMASK 0x10//2^4 in hex define pin 4 as interrupt pin
RTC_DATA_ATTR int bootCount = 0;// variable store in rtc memory

ADXL345_WE myAcc = ADXL345_WE(); // constructor

void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(4, INPUT_PULLDOWN);// internal pull down used to reduce component count
  ++bootCount;// number of time the esp32 has booted
  Serial.println("Boot number: " + String(bootCount));
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4,1);// single external interrupt initialized, active high

  //Serial.println("ADXL345_Sketch - Activity Interrupts");
  // if (!myAcc.init()) {Serial.println("ADXL345 not connected!");}// adxl345 initiallization and error handling
  myAcc.init();
  myAcc.setDataRate(ADXL345_DATA_RATE_3200);// samping rate
  myAcc.setRange(ADXL345_RANGE_16G);// range settings
  myAcc.setFreeFallThresholds(0.4, 100);//freefall senstivity & other argument
  myAcc.setActivityParameters(ADXL345_AC_MODE, ADXL345_XYZ, 6);// activity sensitivity and other arguments
  myAcc.setInterrupt(ADXL345_ACTIVITY, 2); //initialization activity interrupt
  myAcc.setInterrupt(ADXL345_FREEFALL, 2);// initialization frefall interrupt
  byte intType = myAcc.readAndClearInterrupts();

  //initialise two instances of the SPIClass attached to VSPI and HSPI respectively
  vspi = new SPIClass(VSPI);
  hspi = new SPIClass(HSPI);

  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS); //SCLK, MISO, MOSI, SS
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_SS); //SCLK, MISO, MOSI, SS

pinMode(VSPI_SS, OUTPUT);
pinMode(HSPI_SS, OUTPUT);


//code will come here

  Serial.println("Going to sleep now");
  delay(2000);
  
}
void loop() {
    /*  if(myAcc.checkInterrupt(intType, ADXL345_FREEFALL)){
        Serial.println("FREEFALL confirmed");}

      if(myAcc.checkInterrupt(intType, ADXL345_ACTIVITY)){
        Serial.println("Activity confirmed");
        }
       myAcc.readAndClearInterrupts();*/
       esp_deep_sleep_start();
              }