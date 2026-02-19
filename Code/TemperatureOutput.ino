#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SDA_PIN 33
#define SCL_PIN 32

#define SEALEVELPRESSURE_HPA (1013.25)

TwoWire I2CBME = TwoWire(0);
Adafruit_BME680 bme(&I2CBME);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("BME680 Sensor Readings");

  I2CBME.begin(SDA_PIN, SCL_PIN, 100000);

  if (!bme.begin(0x77, &I2CBME)) {
    Serial.println("Could not find BME680! Check wiring.");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  Serial.println("Sensor initialized successfully!");
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading, retrying...");
    delay(2000);
    return;
  }

  Serial.println("========== BME680 Readings ==========");

  Serial.print("  Temperature : ");
  Serial.print(bme.temperature, 2);
  Serial.println(" C");

  Serial.print("  Humidity    : ");
  Serial.print(bme.humidity, 2);
  Serial.println(" %");

  Serial.print("  Pressure    : ");
  Serial.print(bme.pressure / 100.0, 2);
  Serial.println(" hPa");

  Serial.print("  Altitude    : ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA), 2);
  Serial.println(" m");

  Serial.print("  Gas (VOC)   : ");
  Serial.print(bme.gas_resistance / 1000.0, 2);
  Serial.println(" KOhm");

  Serial.println("=====================================");
  Serial.println();

  delay(3000);
}