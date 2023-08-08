#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <driver/i2s.h>

#define SCK 17
#define MISO 19
#define MOSI 23
#define CS 5

SPIClass spi = SPIClass(VSPI);

#define SAMPLE_RATE 44100   // Sample rate for audio recording
#define AUDIO_LENGTH_SEC 5  // Audio clip duration in seconds
#define NUM_SAMPLES (SAMPLE_RATE * AUDIO_LENGTH_SEC)

void startRecording() {

  File audioFile = SD.open("/audio.raw", FILE_WRITE);
  if (!audioFile) {
    Serial.println("Error opening audio file!");
  } else {
    Serial.println("opening audio file!");
  }

  size_t bytesToRead = NUM_SAMPLES * sizeof(uint16_t);
  size_t bytesRead = 0;
  uint16_t buffer[256];  // Buffer for audio data

  Serial.println("Recording audio...");
  while (bytesRead < bytesToRead) {
    size_t bytesToReadNow = min(sizeof(buffer), bytesToRead - bytesRead);
    i2s_read(I2S_NUM_0, buffer, bytesToReadNow, &bytesToReadNow, portMAX_DELAY);
    audioFile.write((uint8_t*)buffer, bytesToReadNow);
    bytesRead += bytesToReadNow;
  }

  audioFile.close();
  Serial.println("Recording complete.");
}

void setup() {
  Serial.begin(115200);
  spi.begin(SCK, MISO, MOSI, CS);
  // Initialize SD card
  if (!SD.begin(CS, spi, 4000000)) {
    Serial.println("Card Mount Failed");
    return;
  }

  // Initialize I2S interface
  i2s_config_t i2sConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
  };
  i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
  i2s_pin_config_t pinConfig = {
    .bck_io_num = 26,
    .ws_io_num = 25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = 32,
  };
  i2s_set_pin(I2S_NUM_0, &pinConfig);

  // Start recording
  startRecording();
}

void loop() {
  // Nothing to do in the loop
}
