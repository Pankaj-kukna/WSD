#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

// ADXL345 I2C address
#define ADXL345_ADDRESS (0x53)

// Initialize ADXL345 sensor object
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Threshold values for shock, vibration, and fall detection
#define SHOCK_THRESHOLD (10.0F)      // Adjust according to your needs
#define VIBRATION_THRESHOLD (2.0F)   // Adjust according to your needs
#define FALL_THRESHOLD (20.0F)       // Adjust according to your needs

void setup() {
  // Start the serial communication
  Serial.begin(115200);

  // Initialize I2C communication
  Wire.begin();

  // Check if the ADXL345 sensor is connected
  if (!accel.begin(ADXL345_ADDRESS)) {
    Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
    while (1);
  }

  // Set the measurement range to +/- 16g
  accel.setRange(ADXL345_RANGE_16_G);

  // Enable activity and inactivity detection
  accel.writeRegister(ADXL345_ACT_INACT_CTL, 0x70);

  // Configure the activity threshold and time
  accel.writeRegister(ADXL345_THRESH_ACT, static_cast<byte>(SHOCK_THRESHOLD));
  accel.writeRegister(ADXL345_TIME_ACT, 10); // 10 * 625us = 6.25ms

  // Configure the inactivity threshold and time
  accel.writeRegister(ADXL345_THRESH_INACT, static_cast<byte>(VIBRATION_THRESHOLD));
  accel.writeRegister(ADXL345_TIME_INACT, 10); // 10 * 1s = 10s

  // Configure the free-fall threshold and time
  accel.writeRegister(ADXL345_THRESH_FF, static_cast<byte>(FALL_THRESHOLD));
  accel.writeRegister(ADXL345_TIME_FF, 5); // 5 * 5ms = 25ms

  // Enable interrupts for activity, inactivity, and free-fall
  accel.writeRegister(ADXL345_INT_ENABLE, ADXL345_ACTIVITY + ADXL345_INACTIVITY + ADXL345_FREE_FALL);

  // Attach interrupt handler function
  attachInterrupt(digitalPinToInterrupt(34), interruptHandler, RISING);
}

void loop() {
  // Nothing to do here, all actions are performed in the interrupt handler
}

void interruptHandler() {
  // Read the interrupt source register to determine the cause of the interrupt
  byte intSource = accel.readRegister(ADXL345_INT_SOURCE);

  // Check if the interrupt was caused by an activity
  if (intSource & ADXL345_ACTIVITY) {
    Serial.println("Activity detected!");
    // Handle activity event
    // ...
  }

  // Check if the interrupt was caused by inactivity
  if (intSource & ADXL345_INACTIVITY) {
    Serial.println("Inactivity detected!");
    // Handle inactivity event
    // ...
  }

  // Check if the interrupt was caused by free-fall
  if (intSource & ADXL345_FREE_FALL) {
    Serial.println("Free-fall detected!");
    // Handle free-fall event
    // ...
  }
}
