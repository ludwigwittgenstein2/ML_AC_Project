//The code monitors room temperature using a BME680 sensor over I2C (pins 21/22) 
// and automatically toggles an AC unit relay on GPIO 27. 
// It supports four modes: auto (thermostat control), manual on/off,
// and learn (adapts preferred temperature from history). 
// Hysteresis prevents rapid relay cycling: AC turns on above T_ON (default 25.8°C) 
// and off below T_OFF (23.8°C), with a 60-second minimum switch cooldown
//  for compressor protection

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define RELAY_PIN 27
#define RELAY_ON_LEVEL  LOW
#define RELAY_OFF_LEVEL HIGH

#define I2C_SDA 21
#define I2C_SCL 22

const float TEMP_EPS = 0.0;  // set 0.05 if you want a deadband margin

enum Mode { MODE_AUTO, MODE_MANUAL_ON, MODE_MANUAL_OFF, MODE_LEARN };
Mode mode = MODE_LEARN;

bool ac_on = false;
unsigned long last_switch_ms = 0;
const unsigned long MIN_SWITCH_MS = 60000; // compressor protection

struct LearnedModel {
  float preferred_temp = 24.0;
  float learning_rate  = 0.05;
  float comfort_low    = 22.0;
  float comfort_high   = 27.0;
  unsigned long last_learn_ms = 0;
};

LearnedModel ml;

float T_ON  = 25.0;
float T_OFF = 23.8;

static const int HISTORY_N = 24;
float temp_history[HISTORY_N];
int history_idx = 0;
int history_count = 0;
bool history_seeded = false;

float last_temp = 0.0;

Adafruit_BME680 bme;
bool bme_ok = false;

void updateSetpoints() {
  T_ON  = ml.preferred_temp + 0.8;
  T_OFF = ml.preferred_temp - 0.2;
}

void setAC(bool on) {
  unsigned long now = millis();
  if (now - last_switch_ms < MIN_SWITCH_MS) return;

  ac_on = on;
  digitalWrite(RELAY_PIN, on ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
  last_switch_ms = now;

  Serial.println(on ? "AC -> ON" : "AC -> OFF");
}

void seedHistory(float t) {
  for (int i = 0; i < HISTORY_N; i++) temp_history[i] = t;
  history_idx = 0;
  history_count = HISTORY_N;
  history_seeded = true;
}

void pushHistory(float t) {
  temp_history[history_idx] = t;
  history_idx = (history_idx + 1) % HISTORY_N;
  if (history_count < HISTORY_N) history_count++;
}

float historyAverage() {
  if (history_count == 0) return ml.preferred_temp;
  float s = 0;
  for (int i = 0; i < history_count; i++) s += temp_history[i];
  return s / history_count;
}

void mlLearn(float current_temp) {
  if (!history_seeded) seedHistory(current_temp);

  pushHistory(current_temp);
  float avg_temp = historyAverage();

  ml.preferred_temp += (avg_temp - ml.preferred_temp) * ml.learning_rate;

  if (ml.preferred_temp < ml.comfort_low)  ml.preferred_temp = ml.comfort_low;
  if (ml.preferred_temp > ml.comfort_high) ml.preferred_temp = ml.comfort_high;

  updateSetpoints();

  Serial.print("LEARN: preferred=");
  Serial.print(ml.preferred_temp, 1);
  Serial.print(" avg=");
  Serial.print(avg_temp, 1);
  Serial.print(" ON>=");
  Serial.print(T_ON, 1);
  Serial.print(" OFF<=");
  Serial.println(T_OFF, 1);
}

float getTemperature() {
  if (bme_ok && bme.performReading()) {
    last_temp = bme.temperature;
    return last_temp;
  }
  // If sensor read fails, keep last known temp (avoid random jumps)
  return last_temp;
}

void printHelp() {
  Serial.println("Commands: on | off | auto | learn | status");
}

void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toLowerCase();

  if (cmd == "on") { mode = MODE_MANUAL_ON; setAC(true); }
  else if (cmd == "off") { mode = MODE_MANUAL_OFF; setAC(false); }
  else if (cmd == "auto") { mode = MODE_AUTO; Serial.println("MODE: AUTO"); }
  else if (cmd == "learn") { mode = MODE_LEARN; Serial.println("MODE: LEARN"); }
  else if (cmd == "status") {
    Serial.print("STATUS: temp=");
    Serial.print(last_temp, 1);
    Serial.print(" AC=");
    Serial.print(ac_on ? "ON" : "OFF");
    Serial.print(" mode=");
    Serial.print(mode == MODE_LEARN ? "LEARN" : (mode == MODE_AUTO ? "AUTO" : "MANUAL"));
    Serial.print(" preferred=");
    Serial.print(ml.preferred_temp, 1);
    Serial.print(" ON>=");
    Serial.print(T_ON, 1);
    Serial.print(" OFF<=");
    Serial.print(T_OFF, 1);
    Serial.print(" histAvg=");
    Serial.println(historyAverage(), 1);
  } else {
    printHelp();
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF_LEVEL);

  Wire.begin(I2C_SDA, I2C_SCL);

  bme_ok = bme.begin(0x77);
  if (!bme_ok) bme_ok = bme.begin(0x76);

  if (bme_ok) {
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setGasHeater(320, 150);
    Serial.println("BME680: OK");
  } else {
    Serial.println("BME680: NOT FOUND");
  }

  updateSetpoints();

  Serial.println("ML Thermostat (BME680)");
  printHelp();
}

void loop() {
  handleSerial();

  float temp = getTemperature();

  if (!history_seeded) seedHistory(temp);

  if (mode == MODE_LEARN) {
    unsigned long now = millis();
    if (now - ml.last_learn_ms >= 300000UL) {
      mlLearn(temp);
      ml.last_learn_ms = now;
    }
  }

  if (mode == MODE_LEARN || mode == MODE_AUTO) {
    if (!ac_on && temp >= (T_ON + TEMP_EPS)) setAC(true);
    if (ac_on  && temp <= (T_OFF - TEMP_EPS)) setAC(false);
  } else if (mode == MODE_MANUAL_ON) {
    setAC(true);
  } else if (mode == MODE_MANUAL_OFF) {
    setAC(false);
  }

  Serial.print("TEMP=");
  Serial.print(temp, 1);
  Serial.print(" AC=");
  Serial.print(ac_on ? "ON" : "OFF");
  Serial.print(" MODE=");
  Serial.print(mode == MODE_LEARN ? "LEARN" : (mode == MODE_AUTO ? "AUTO" : "MANUAL"));
  Serial.print(" PREF=");
  Serial.print(ml.preferred_temp, 1);
  Serial.print(" ON>=");
  Serial.print(T_ON, 1);
  Serial.print(" OFF<=");
  Serial.println(T_OFF, 1);

  delay(3000);
}
