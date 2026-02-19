#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define I2C_SDA 26  // Blue wire from BME680
#define I2C_SCL 32  // Yellow wire from BME680
#define SEALEVELPRESSURE_HPA (1013.25)

TwoWire I2C_BME = TwoWire(0);
Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // Wait for Serial

  Serial.println(F("BME680 test with custom I2C pins!"));

  // Initialize custom I2C bus
  I2C_BME.begin(I2C_SDA, I2C_SCL, 100000);  // 100kHz frequency [web:11][page:1]

  if (!bme.begin(0x77, &I2C_BME)) {  // Try 0x77 first; change to 0x76 if needed
    Serial.println(F("Could not find valid BME680 sensor, check wiring/I2C address!"));
    while (1) delay(10);
  }

  // Configure sensor
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320°C for 150 ms [web:1][page:2]
  Serial.println(F("BME680 ready!"));
}

void loop() {
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading!"));
    delay(2000);
    return;
  }

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading!"));
    delay(2000);
    return;
  }

  // Print all readings
  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" °C"));

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));

  Serial.print(F("Gas Resistance = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));

  Serial.print(F("Approx. Altitude = "));
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(F(" m"));

  Serial.println();  // Blank line
  delay(2000);
}
