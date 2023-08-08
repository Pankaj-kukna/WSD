#include <Arduino.h>
#include <driver/i2s.h>
#include <SD.h>
#include "FS.h"

// Define the GPIO pin for the trigger input
const int triggerPin = 4;

//uint8_t
//uint8_t

// Define the I2S configuration
const int sampleRate = 44100;
const int bitsPerSample = 32;
const int bufferSize = 1024; // Adjust buffer size as needed

// SD card configuration
const char *sdCardMountPoint = "/sdcard";
const char *fileName = "/sdcard/recording.wav";

// Variables
File audioFile;
volatile bool recording = false;
volatile int samplesRecorded = 0;

// I2S configuration
i2s_config_t i2sConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = sampleRate,
    .bits_per_sample = (i2s_bits_per_sample_t)bitsPerSample,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = bufferSize,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
};

void IRAM_ATTR triggerCallback() {
  if (digitalRead(triggerPin) == HIGH) { // Trigger on GPIO 4 going high
    recording = true;
  }
}

void setup() {
  Serial.begin(115200);

  // Mount SD card
  if (!SD.begin()) {
    Serial.println("Failed to mount SD card");
    while (1);
  }

  // Initialize I2S
  i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
  i2s_pin_config_t pinConfig;
  pinConfig.bck_io_num = 26;
  pinConfig.ws_io_num = 25;
  pinConfig.data_out_num = -1; // Not used
  pinConfig.data_in_num = 22;
  i2s_set_pin(I2S_NUM_0, &pinConfig);

  // Set up the trigger input
  pinMode(triggerPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(triggerPin), triggerCallback, CHANGE);
}

void loop() {
  if (recording) {
    if (!audioFile) {
      audioFile = SD.open(fileName, FILE_WRITE);
      if (!audioFile) {
        Serial.println("Failed to open file");
        recording = false;
        return;
      }

      // Write the WAV header
      writeWavHeader(audioFile);
    }

    int32_t buffer[bufferSize];
    size_t bytes_read;

    // Read data from I2S
    i2s_read(I2S_NUM_0, buffer, bufferSize * sizeof(int32_t), &bytes_read, portMAX_DELAY);

    // Write data to the file
    audioFile.write((uint8_t *)buffer, bytes_read);
    samplesRecorded += bytes_read / sizeof(int32_t);

    // Check if 10 seconds have been recorded
    if (samplesRecorded >= sampleRate * 10) {
      recording = false;
      audioFile.close();

      // Update WAV header with the total data size
      audioFile = SD.open(fileName, FILE_WRITE);
      if (audioFile) {
        int32_t dataSize = audioFile.size() - 44;
        audioFile.seek(40);
        audioFile.write((uint8_t *)&dataSize, sizeof(dataSize));
        audioFile.seek(4);
        dataSize += 36;
        audioFile.write((uint8_t *)&dataSize, sizeof(dataSize));
        audioFile.close();
      } else {
        Serial.println("Failed to update WAV header");
      }

      Serial.println("Recording finished");
    }
  }
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
void writeWavHeader(File &file) {
  uint32_t bytesPerSample = bitsPerSample / 8;
  uint32_t byteRate = sampleRate * bytesPerSample;

  file.write("RIFF");
  int32_t dataSize = 0; // We will update this later
  int32_t fileSize = dataSize + 36;
  file.write((uint8_t *)&fileSize, sizeof(fileSize));
  file.write("WAVE");
  file.write("fmt ");
  int32_t fmtSize = 16;
  file.write((uint8_t *)&fmtSize, sizeof(fmtSize));
  int16_t audioFormat = 1; // PCM
  file.write((uint8_t *)&audioFormat, sizeof(audioFormat));
  int16_t numChannels = 1; // Mono
  file.write((uint8_t *)&numChannels, sizeof(numChannels));
  file.write((uint8_t *)&sampleRate, sizeof(sampleRate));
  file.write((uint8_t *)&byteRate, sizeof(byteRate));
  int16_t blockAlign = numChannels * bytesPerSample;
  file.write((uint8_t *)&blockAlign, sizeof(blockAlign));
  file.write((uint8_t *)&bitsPerSample, sizeof(bitsPerSample));
  file.write("data");
  file.write((uint8_t *)&dataSize, sizeof(dataSize));
}
