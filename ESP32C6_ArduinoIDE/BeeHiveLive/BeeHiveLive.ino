#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

// Add required libraries
#include <Arduino.h>
#include <OneWireNg_CurrentPlatform.h>
#include <Zigbee.h>

// Input/Output definition
#define BATTERY_VOLTAGE_PIN A2
#define ONE_WIRE_BUS        D4

// Zigbee endpoint sensor configuration
#define INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER  1
#define SOC_SENSOR_ENDPOINT_NUMBER            2
#define DALLAS_TEMP_SENSOR_ENDPOINT_NUMBER    3

// 1-Wire definitions
#define ONEWIRE_CMD_MATCH_ROM     0x55
#define ONEWIRE_CMD_SKIP_ROM      0xCC
#define DS18B20_FAMILY_CODE       0x28
#define DS18B20_CMD_CONVERT_T     0x44
#define DS18B20_CMD_READ_SCRATCH  0xBE
#define DS18B20_SCRATCHPAD_SIZE   9
#define DS18B20_CONVERT_MS        750

// Known DS18B20 addresses (0x28 at the end represents device family code)
static const OneWireNg::Id DS18B20[] = {
  { 0x28, 0xC1, 0xC4, 0x9C, 0x31, 0x21, 0x03, 0xEA },
  { 0x28, 0xA8, 0x6A, 0xD0, 0x63, 0x20, 0x01, 0x90 },
  { 0x28, 0x6D, 0xE1, 0xC2, 0x63, 0x20, 0x01, 0x60 }
};

static const int DS18B20_AMOUNT = sizeof(DS18B20) / sizeof(DS18B20[0]);

static const char* DS18B20_NAMES[] = {
    "BeeHive_1",
    "BeeHive_2",
    "BeeHive_3",
};

ZigbeeTempSensor zbInternalTempSensor = ZigbeeTempSensor(INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER);
ZigbeeAnalog zbSocSensor = ZigbeeAnalog(SOC_SENSOR_ENDPOINT_NUMBER);
ZigbeeTempSensor zbDallasTempSensor = ZigbeeTempSensor(DALLAS_TEMP_SENSOR_ENDPOINT_NUMBER);

OneWireNg_CurrentPlatform oneWire(ONE_WIRE_BUS, true);



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



// Internal Temperature Sensor
void internal_temp_sensor_value_update() {
  float tsens_value = temperatureRead();
  Serial.printf("Internal → %.4f °C", tsens_value);
  Serial.println();
  zbInternalTempSensor.setTemperature(tsens_value);
  zbInternalTempSensor.reportTemperature();
}

// Battery SoC Sensor
void soc_sensor_value_update() {
  int   adc_value = analogRead(BATTERY_VOLTAGE_PIN);
  
  float voltage   = (adc_value / 1023.0f) * 3.3f * 1.694915254237288;   // 1023: 10-bit adc / 3.3: max voltage / 1.69... voltage divider
  float soc = (voltage - 3.0f) * (100.0f / (4.2f - 3.0f));              // Li-Ion 3.0–4.2V → SoC 0–100%
  soc = constrain(soc, 0.0f, 100.0f);

  Serial.printf("Battery ADC=%d Voltage=%.2fV SoC=%.1f%%\r", adc_value, voltage, soc);
  Serial.println();

  zbSocSensor.setAnalogInput(soc);
  zbSocSensor.reportAnalogInput();
}

// Temporary function to read the address of one single connected one wire device
void discoverOneWire() {
  uint8_t oneWireAddr[8];
  oneWire.searchReset();
  if (!oneWire.search(oneWireAddr)) {
    Serial.println("No more addresses.");
  }

  // Print 1-Wire Addresses on the serial monitor
  Serial.printf("1-Wire Addr: ");
  for(int i = 7; i >= 0; i--) {
    Serial.printf("%x ", oneWireAddr[i]);
  }
  Serial.println();
}

bool read_sensor(int index, float *temp_out)
{
    if (oneWire.addressSingle(DS18B20[index]) != OneWireNg::EC_SUCCESS)
        return false;

    oneWire.writeByte(DS18B20_CMD_READ_SCRATCH);

    uint8_t sp[9];
    oneWire.readBytes(sp, 9);

    if (OneWireNg::crc8(sp, 8) != sp[8])
        return false;

    int16_t raw = (int16_t)((sp[1] << 8) | sp[0]);
    *temp_out = raw / 16.0f;
    return true;
}

void readAllOneWireTemps(float *tsens_value) {
  //discoverOneWire();
  for (int i = 0; i < 3; i++) {                               // Try three times in case of any fault
    if (oneWire.addressAll() == OneWireNg::EC_SUCCESS) {      // Trigger broadcast convertion off all connected DS18B20 sensors
      oneWire.writeByte(DS18B20_CMD_CONVERT_T);
      delay(DS18B20_CONVERT_MS);
      for (int j = 0; j < DS18B20_AMOUNT; j++) {
        float temp;
        if (read_sensor(j, &temp)) {
            printf("%-10s → %.4f °C\n", DS18B20_NAMES[j], temp);
            tsens_value[j] = temp;
        } else {
            printf("%-10s → READ FAILED\n", DS18B20_NAMES[j]);
        }
      }
      break;
    }
    delay(100);                                               // Wait before retry
  }
}

void dallas_temp_sensor_value_update() {
  float tsens_value[3];
  readAllOneWireTemps(tsens_value);
  
  zbDallasTempSensor.setTemperature(tsens_value[0]);
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
