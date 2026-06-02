#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

// Add required libraries
#include <Arduino.h>
#include <OneWireNg_CurrentPlatform.h>
#include <Zigbee.h>

// Input/Output definition
#define BATTERY_VOLTAGE_PIN A2
#define ONE_WIRE_BUS D4

// Zigbee endpoint sensor configuration
#define INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER 1
#define SOC_SENSOR_ENDPOINT_NUMBER 2
#define DALLAS_TEMP_SENSOR_ENDPOINT_NUMBER 3

ZigbeeTempSensor zbInternalTempSensor = ZigbeeTempSensor(INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER);
ZigbeeAnalog zbSocSensor = ZigbeeAnalog(SOC_SENSOR_ENDPOINT_NUMBER);
ZigbeeTempSensor zbDallasTempSensor = ZigbeeTempSensor(DALLAS_TEMP_SENSOR_ENDPOINT_NUMBER);

OneWireNg_CurrentPlatform oneWire(ONE_WIRE_BUS, true);
//DallasTemperature dallasSensors(&oneWire);

void initInternalTempSensor() {
  zbInternalTempSensor.setManufacturerAndModel("Wolfgang Diermeier", "BeeHiveLive");              // Set Zigbee device name and model
  zbInternalTempSensor.setMinMaxValue(10, 50);                                                    // Set minimum and maximum temperature measurement value (10-50°C is default range for chip temperature measurement)
  zbInternalTempSensor.setDefaultValue(10.0);                                                     // Optional: Set default (initial) value for the temperature sensor to 10.0°C to match the minimum temperature measurement value
  Zigbee.addEndpoint(&zbInternalTempSensor);                                                      // Register end point
}

void initAnalogSensor() {
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);                                                            // Configure Pin as input
  zbSocSensor.addAnalogInput();                                                                   // Adding analog input
  zbSocSensor.setAnalogInputApplication(ESP_ZB_ZCL_AI_PERCENTAGE_OTHER);                          // Define senor reading as percentage value
  zbSocSensor.setAnalogInputDescription("ADC Sensor");                                            // 
  zbSocSensor.setAnalogInputResolution(0.1);
  zbSocSensor.setAnalogInputMinMax(0.0, 100.0);
  Zigbee.addEndpoint(&zbSocSensor);
}

void initDallas() {
  zbDallasTempSensor.setMinMaxValue(-40, 85);                                                   // Set minimum and maximum temperature measurement value
  zbDallasTempSensor.setDefaultValue(20.0);                                                     // Optional: Set default (initial) value for the temperature sensor to 20.0°C
  Zigbee.addEndpoint(&zbDallasTempSensor);                                                     // Register end point
}

/************************ Temp sensor *****************************/
void internal_temp_sensor_value_update() {
  float tsens_value = temperatureRead();
  Serial.printf("Updated internal temperature sensor value to %.2f°C\r\n", tsens_value);
  zbInternalTempSensor.setTemperature(tsens_value);
  zbInternalTempSensor.reportTemperature();
}

/************************ Battery SoC sensor *****************************/
void soc_sensor_value_update() {
  int   adc_value = analogRead(BATTERY_VOLTAGE_PIN);
  
  float voltage   = (adc_value / 1023.0f) * 3.3f * 1.694915254237288;   // 1023: 10-bit adc / 3.3: max voltage / 1.69... voltage divider
  float soc = (voltage - 3.0f) * (100.0f / (4.2f - 3.0f));              // Li-Ion 3.0–4.2V → SoC 0–100%
  soc = constrain(soc, 0.0f, 100.0f);

  Serial.printf("Battery ADC=%d Voltage=%.2fV SoC=%.1f%%\r\n",
                adc_value, voltage, soc);

  zbSocSensor.setAnalogInput(soc);
  zbSocSensor.reportAnalogInput();
}

float readOneWireTemp() {
  uint8_t oneWireAddr[8];
  uint8_t rawOneWiredata[9];

  for (int i = 0; i < 3; i++) {
    oneWire.searchReset();
    if (!oneWire.search(oneWireAddr)) {
      Serial.print("No more addresses.\n");
    }
    
    oneWire.reset();
    oneWire.writeByte(0xCC);
    oneWire.writeByte(0x44);
    delay(800);
    oneWire.reset();
    oneWire.writeByte(0xCC);
    oneWire.writeByte(0xBE);
    for (int i = 0; i < 9; i++) {
      rawOneWiredata[i] = oneWire.readByte();
    }
    uint8_t crc = oneWire.crc8(rawOneWiredata, 8);

    Serial.printf("One Wire Addr: ");
    for(int i = 7; i >= 0; i--) {
      Serial.printf("%x ", oneWireAddr[i]);
    }
    Serial.printf("\nOne Wire Data: ");
    for (int i = 8; i >= 0; i--) {
      Serial.printf("%x ", rawOneWiredata[i]);
    }
    Serial.println();
    if (crc == rawOneWiredata[8]){
      uint16_t result = (rawOneWiredata[1] << 8) | rawOneWiredata[0];
      return result / 16.0;
    } else {
      Serial.println("CRC fail\n");
    }
  }
  return NAN;
}

void dallas_temp_sensor_value_update() {
  float tsens_value = readOneWireTemp();
  Serial.printf("Updated external temperature sensor value to %.2f°C\r\n", tsens_value);
  
  zbDallasTempSensor.setTemperature(tsens_value);
  zbDallasTempSensor.reportTemperature();
}

/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);
  
  // Init button switch
  pinMode(BOOT_PIN, INPUT_PULLUP);

  initInternalTempSensor();           // Initialize internal temperature reading of the ESP32C6
  initAnalogSensor();                 // Initialize analog reading for the Liion battery voltage for SoC calculation
  initDallas();

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
  internal_temp_sensor_value_update();
  soc_sensor_value_update();
  dallas_temp_sensor_value_update();

  esp_zb_sleep_enable(true);
  esp_zb_sleep_set_threshold(3000);
  esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
  //Serial.println("Stopping Zigbee");
  //delay(200);
  //Serial.println("Entering Deep Sleep");
  //Zigbee.stop();
  //esp_deep_sleep_start();
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(BOOT_PIN) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(BOOT_PIN) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
  }
  delay(10000);
  internal_temp_sensor_value_update();
  soc_sensor_value_update();
  dallas_temp_sensor_value_update();
}
