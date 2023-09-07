#include "esp_camera.h"
#include "Arduino.h"
#include "driver/rtc_io.h"
#include "SerialTransfer.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

SerialTransfer myTransfer;
HardwareSerial Comm(2);

struct img_meta_data{
  uint16_t counter;
  uint16_t imSize;
  uint16_t numLoops;
  uint16_t sizeLastLoop;
} ImgMetaData;
 
const uint16_t PIXELS_PER_PACKET = MAX_PACKET_SIZE - sizeof(ImgMetaData);

void setup(){
  Serial.begin(115200);                     // Define and start serial monitor
  // 115200,256000,512000,962100
  Comm.begin(962100, SERIAL_8N1,13,14);     //, Comm_Txd_pin, Comm_Rxd_pin); // Define and start Comm serial port
  myTransfer.begin(Comm);
 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
 
 if(psramFound()){
   config.frame_size = FRAMESIZE_XGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
   config.jpeg_quality = 10;
   config.fb_count = 2;
 } else {
   config.frame_size = FRAMESIZE_SVGA;
   config.jpeg_quality = 12;
   config.fb_count = 1;
 }
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK){
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
void loop(){
  uint16_t startIndex = 0;
  camera_fb_t * fb = NULL;
  //Take Picture with Camera
  fb = esp_camera_fb_get(); 
  ImgMetaData.imSize       = fb->len;                             //sizeof(myFb);
  ImgMetaData.numLoops     = (fb->len / PIXELS_PER_PACKET) + 1;   //(sizeof(myFb)/PIXELS_PER_PACKET) + 1; 
  ImgMetaData.sizeLastLoop = fb->len % PIXELS_PER_PACKET;         //(sizeof(myFb)%PIXELS_PER_PACKET);

  //Serial.println(ImgMetaData.counter);
  //Serial.println(ImgMetaData.imSize);
  //Serial.println(ImgMetaData.numLoops);
  //Serial.println(ImgMetaData.sizeLastLoop);

  for(ImgMetaData.counter=1; ImgMetaData.counter<=ImgMetaData.numLoops; ImgMetaData.counter++){
    myTransfer.txObj(ImgMetaData);
    Serial.println(ImgMetaData.counter);
    stuffPixels(fb->buf, startIndex, sizeof(ImgMetaData));    
    startIndex += PIXELS_PER_PACKET;    
    delay(10);
  }
  Serial.println("reached point 1");
  printBuf(fb->buf,ImgMetaData.imSize);
  esp_camera_fb_return(fb);   //clear camera memory
  delay(15000); 
  }

void stuffPixels(const uint8_t *pixelBuff, uint16_t bufStartIndex, uint16_t txStartIndex){
  uint16_t txi = txStartIndex;
  uint16_t i=bufStartIndex;
  for (i; i<(bufStartIndex + 246); i++) {
    myTransfer.packet.txBuff[txi] = pixelBuff[i];
   /* Serial.print(myTransfer.packet.txBuff[txi]);
    Serial.print(",");
    Serial.print(pixelBuff[i]);
    Serial.print(",");
    Serial.print(txi);
    Serial.print(",");
    Serial.print(i);
    Serial.println();*/
    txi++;
    }
     myTransfer.sendDatum(myTransfer.packet.txBuff);    
   }

void printBuf(const uint8_t* pixelBuffer,uint16_t buffersize){
  Serial.println("Pixel Values: { ");
  for (uint16_t k=0; k<buffersize; k++){
    Serial.print(pixelBuffer[k]); 
  }
  Serial.println("}");
}