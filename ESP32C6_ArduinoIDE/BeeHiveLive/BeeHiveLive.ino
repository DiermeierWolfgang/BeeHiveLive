#include <Arduino.h>
#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

/* Zigbee temperature sensor configuration */
#define TEMP_SENSOR_ENDPOINT_NUMBER 1
#define SOC_SENSOR_ENDPOINT_NUMBER 2
uint8_t button = BOOT_PIN;

ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);
ZigbeeAnalog zbSocSensor = ZigbeeAnalog(SOC_SENSOR_ENDPOINT_NUMBER);

/************************ Temp sensor *****************************/
static void temp_sensor_value_update() {  //void *arg) {
                                          //for (;;) {
  // Read temperature sensor value
  float tsens_value = temperatureRead();
  Serial.printf("Updated temperature sensor value to %.2f°C\r\n", tsens_value);
  // Update temperature value in Temperature sensor EP
  zbTempSensor.setTemperature(tsens_value);
  zbTempSensor.reportTemperature();
  //delay(1000);
  //}
}

/************************ Battery SoC sensor *****************************/
void soc_sensor_value_update() {
  int   adc_value = analogRead(A2);
  float voltage   = (adc_value / 4095.0f) * 3.3f;

  // Beispiel: Li-Ion 3.0–4.2V → SoC 0–100%
  float soc = (voltage - 3.0f) * (100.0f / (4.2f - 3.0f));
  soc = constrain(soc, 0.0f, 100.0f);

  Serial.printf("Battery ADC=%d Voltage=%.2fV SoC=%.1f%%\r\n",
                adc_value, voltage, soc);

  // Wir senden SoC als Analog Input (float)
  zbSocSensor.setAnalogInput(adc_value);
  zbSocSensor.reportAnalogInput();
}

/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);

  // Init button switch
  pinMode(button, INPUT_PULLUP);

  // Optional: set Zigbee device name and model
  zbTempSensor.setManufacturerAndModel("CustomPCB_V2.0", "BeeHiveLive");
  // Set minimum and maximum temperature measurement value (10-50°C is default range for chip temperature measurement)
  zbTempSensor.setMinMaxValue(10, 50);
  // Optional: Set default (initial) value for the temperature sensor to 10.0°C to match the minimum temperature measurement value
  zbTempSensor.setDefaultValue(10.0);

  // init analog sensor
  pinMode(A2, INPUT);
  analogReadResolution(10);
  zbSocSensor.addAnalogInput();
  zbSocSensor.setAnalogInputApplication(ESP_ZB_ZCL_AI_TEMPERATURE_OTHER);  // Generic Sensor
  zbSocSensor.setAnalogInputDescription("ADC Sensor");
  zbSocSensor.setAnalogInputResolution(0.1);
  zbSocSensor.setAnalogInputMinMax(0.0, 100.0);

  // Endpoints registrieren
  Zigbee.addEndpoint(&zbTempSensor);
  Zigbee.addEndpoint(&zbSocSensor);

  Serial.println("Starting Zigbee...");
  // When all EPs are registered, start Zigbee in End Device mode
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  } else {
    Serial.println("Zigbee started successfully!");
  }
  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  delay(500);

  // Start Temperature sensor reading task
  //xTaskCreate(temp_sensor_value_update, "temp_sensor_update", 2048, NULL, 10, NULL);
  //temp_sensor_value_update();
  //soc_sensor_value_update();
  // Set reporting interval for temperature measurement in seconds, must be called after Zigbee.begin()
  // min_interval and max_interval in seconds, delta (temp change in 0,1 °C)
  // if min = 1 and max = 0, reporting is sent only when temperature changes by delta
  // if min = 0 and max = 10, reporting is sent every 10 seconds or temperature changes by delta
  // if min = 0, max = 10 and delta = 0, reporting is sent every 10 seconds regardless of temperature change

  //esp_zb_sleep_enable(true);
  //esp_zb_sleep_set_threshold(3000);
  //esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
  //Serial.println("Stopping Zigbee");
  //delay(200);
  //Serial.println("Entering Deep Sleep");
  //Zigbee.stop();
  //esp_deep_sleep_start();
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(button) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
    zbTempSensor.reportTemperature();
  }
  temp_sensor_value_update();
  soc_sensor_value_update();
  delay(1000);
}
