#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

// Add required libraries
#include <Arduino.h>
#include <OneWireNg_CurrentPlatform.h>
#include <HX711.h>
#include <Zigbee.h>

// Input/Output definition
#define BATTERY_VOLTAGE_PIN A2
#define I2C_CLK_PIN         D5
#define I2C_DATA_PIN        D4
#define ONE_WIRE_BUS_PIN    I2C_DATA_PIN
#define HX711_SCK_PIN       D0
#define HX711_DOUT_PIN      D1
#define CN3791_CHRG_PIN     D3
#define CN3791_DONE_PIN     D6

// Zigbee endpoint sensor configuration
#define INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER  1
#define DS18B20_TEMP_SENSOR_1_ENDPOINT_NUMBER 2
#define DS18B20_TEMP_SENSOR_2_ENDPOINT_NUMBER 3
#define DS18B20_TEMP_SENSOR_3_ENDPOINT_NUMBER 4
#define HX711_SENSOR_ENDPOINT_NUMBER          5
#define CN3791_CHRG_ENDPOINT_NUMBER           6
#define CN3791_DONE_ENDPOINT_NUMBER           7
#define DEEPSLEEP_ENDPOINT_NUMBER             8
#define HX711_GAIN_ENDPOINT_NUMBER            9
#define HX711_OFFSET_ENDPOINT_NUMBER          10

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

// Get the overall amount of connected DS18B20 temperature sensors
static const int DS18B20_AMOUNT = sizeof(DS18B20) / sizeof(DS18B20[0]);

// Optional: Define names for the temperature sensors
static const char* DS18B20_NAME[] = {
    "BeeHive_1",
    "BeeHive_2",
    "BeeHive_3",
};


ZigbeeTempSensor zbInternalTempSensor = ZigbeeTempSensor(INTERNAL_TEMP_SENSOR_ENDPOINT_NUMBER);
ZigbeeTempSensor zbDS18B20TempSensor[DS18B20_AMOUNT] = {
  ZigbeeTempSensor(DS18B20_TEMP_SENSOR_1_ENDPOINT_NUMBER),
  ZigbeeTempSensor(DS18B20_TEMP_SENSOR_2_ENDPOINT_NUMBER),
  ZigbeeTempSensor(DS18B20_TEMP_SENSOR_3_ENDPOINT_NUMBER)
};
ZigbeeAnalog zbHX711Sensor = ZigbeeAnalog(HX711_SENSOR_ENDPOINT_NUMBER);
ZigbeeAnalog zbHX711Gain = ZigbeeAnalog(HX711_GAIN_ENDPOINT_NUMBER);
ZigbeeAnalog zbHX711Offset = ZigbeeAnalog(HX711_OFFSET_ENDPOINT_NUMBER);
ZigbeeBinary zbCN3791Chrg = ZigbeeBinary(CN3791_CHRG_ENDPOINT_NUMBER);
ZigbeeBinary zbCN3791Done = ZigbeeBinary(CN3791_DONE_ENDPOINT_NUMBER);
ZigbeeBinary zbDeepSleep = ZigbeeBinary(DEEPSLEEP_ENDPOINT_NUMBER);

OneWireNg_CurrentPlatform oneWire(ONE_WIRE_BUS_PIN, true);
HX711 hx711;

// Global variable definition
float hx711gain;
float hx711offset;
bool deepSleep;


void initInternalTempSensor() {
  // Set the name of the zigbee device
  zbInternalTempSensor.setManufacturerAndModel("Wolfgang Diermeier", "BeeHiveLive");
  // Define the device is battery powered as information for the power cluster. Required for the Battery SoC information.
  zbInternalTempSensor.setPowerSource(ZB_POWER_SOURCE_BATTERY);
  // Set minimum and maximum temperature measurement value (10-50°C is default range for chip temperature measurement)
  zbInternalTempSensor.setMinMaxValue(10, 50);
  // Optional: Set default (initial) value for the temperature sensor to 10.0°C to match the minimum temperature measurement value
  zbInternalTempSensor.setDefaultValue(10.0);
  // Register end point
  Zigbee.addEndpoint(&zbInternalTempSensor);
}

void initSOCSensor() {
  pinMode(BATTERY_VOLTAGE_PIN, INPUT);
}

void initDS18B20() {
  for (int i = 0; i < DS18B20_AMOUNT; i++) {
    // Set minimum and maximum temperature measurement value
    zbDS18B20TempSensor[i].setMinMaxValue(-40, 85);
    // Optional: Set default (initial) value for the temperature sensor to 20.0°C
    zbDS18B20TempSensor[i].setDefaultValue(20.0);
    // Register end points
    Zigbee.addEndpoint(&zbDS18B20TempSensor[i]);
  }
}

void initHX711() {
  // Set up the HX711 device
  hx711.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  hx711.power_up();
  // Create a new analog zigbee endpoint for the HX711 sensor reading
  zbHX711Sensor.addAnalogInput();
  zbHX711Gain.addAnalogOutput();
  zbHX711Offset.addAnalogOutput();
  // Define the name of the reading (shows up as friendly name in Home Assistant)
  zbHX711Sensor.setAnalogInputDescription("Weight (kg)");
  zbHX711Gain.setAnalogOutputDescription("Scale Gain");
  zbHX711Offset.setAnalogOutputDescription("Scale Offset");
  // Set an unidentified Application to prevent units from showing up in Home Assistant (kg is not avaliable)
  zbHX711Sensor.setAnalogInputApplication(0xffffff);
  zbHX711Gain.setAnalogOutputApplication(0xffffff);
  zbHX711Offset.setAnalogOutputApplication(0xffffff);
  // raw sensor output resolution is 1
  zbHX711Sensor.setAnalogInputResolution(0.001);
  zbHX711Gain.setAnalogOutputResolution(0.00001);
  zbHX711Offset.setAnalogOutputResolution(0.001);
  // Set default values
  //zbHX711Gain.setAnalogOutput(0.00004);
  //zbHX711Offset.setAnalogOutput(1.647);
  // If analog output cluster is added, set callback function for analog output change
  zbHX711Gain.onAnalogOutputChange(setHX711Gain);
  zbHX711Gain.onAnalogOutputChange(setHX711Offset);
  // Register end points
  Zigbee.addEndpoint(&zbHX711Sensor);
  Zigbee.addEndpoint(&zbHX711Gain);
  Zigbee.addEndpoint(&zbHX711Offset);
}

void initBinarySensor() {
  // Configure binary gpios as input (No pull-up since this is hardware implemented)
  pinMode(CN3791_CHRG_PIN, INPUT);
  pinMode(CN3791_DONE_PIN, INPUT);
  // Create new binary zigbee endpoints
  zbCN3791Chrg.addBinaryInput();
  zbCN3791Done.addBinaryInput();
  zbDeepSleep.addBinaryOutput();
  // Set a suited application
  zbCN3791Chrg.setBinaryInputApplication(0x00000000);
  zbCN3791Done.setBinaryInputApplication(0x00000000);
  zbDeepSleep.setBinaryInputApplication(0x00000000);
  // Define the name for each binary sensor
  zbCN3791Chrg.setBinaryInputDescription("Charging");
  zbCN3791Done.setBinaryInputDescription("Charged");
  zbDeepSleep.setBinaryInputDescription("Deep Sleep");
  // Callback function on binary change
  zbDeepSleep.onBinaryOutputChange(enableDeepSleep);
  // Register end points
  Zigbee.addEndpoint(&zbCN3791Chrg);
  Zigbee.addEndpoint(&zbCN3791Done);
  Zigbee.addEndpoint(&zbDeepSleep);
}


// Internal Temperature Sensor
void internal_temp_sensor_value_update() {
  float tsens_value = temperatureRead();
  //Serial.printf("Internal → %.4f °C", tsens_value);
  //Serial.println();
  zbInternalTempSensor.setTemperature(tsens_value);
  zbInternalTempSensor.reportTemperature();
}

// Battery SoC Sensor
void soc_sensor_value_update() {
  int adc_value = analogRead(BATTERY_VOLTAGE_PIN);
  
  float voltage   = (adc_value / 1023.0f) * 3.3f * 1.694915254237288;   // 1023: 10-bit adc / 3.3: max voltage / 1.69... voltage divider
  float soc = (voltage - 3.0f) * (100.0f / (4.2f - 3.0f));              // Li-Ion 3.0–4.2V → SoC 0–100%
  soc = constrain(soc, 0.0f, 100.0f);                                   // Limit from 0% to 100%

  //Serial.printf("Battery ADC=%d Voltage=%.2fV SoC=%.1f%%\r", adc_value, voltage, soc);
  //Serial.println();

  // Battery information is placed into first endpoint
  zbInternalTempSensor.setBatteryPercentage((uint8_t) soc);
  //zbInternalTempSensor.setBatteryVoltage((uint8_t) voltage * 10);
  zbInternalTempSensor.reportBatteryPercentage();
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

bool readSingleOneWireTemp(int index, float *temp_out)
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
  for (int i = 0; i < 3; i++) {                                   // Try three times in case of any fault
    if (oneWire.addressAll() == OneWireNg::EC_SUCCESS) {          // Trigger broadcast conversion off all connected DS18B20 sensors
      oneWire.writeByte(DS18B20_CMD_CONVERT_T);                   
      delay(DS18B20_CONVERT_MS);
      for (int j = 0; j < DS18B20_AMOUNT; j++) {                  // Loop through all known temperature sensor addresses
        float temp;
        if (readSingleOneWireTemp(j, &temp)) {                    // Read individual sensor
            //printf("%-10s → %.4f °C\n", DS18B20_NAME[j], temp);   // Print value on serial monitor if reading is valid
            tsens_value[j] = temp;                                // Store value in correspondind index j for the sensor
        } else {
            printf("%-10s → READ FAILED\n", DS18B20_NAME[j]);     // Print failure on serial monitor
        }
      }
      break;                                                      // Leave for loop after successful broadcast conversion
    }
    delay(100);                                                   // Wait before retry broadcast conversion
  }
}

void DS18B20_temp_sensor_value_update() {
  float tsens_value[DS18B20_AMOUNT];                              // Define float array for all temperature readings
  readAllOneWireTemps(tsens_value);                               // Get temperature readings from one wire bus and store them in array
  
  for (int i = 0; i < DS18B20_AMOUNT; i++) {                      // Loop through all known temperature sensor addresses
    zbDS18B20TempSensor[i].setTemperature(tsens_value[i]);        // Set temperature in zigbee endpoint
    zbDS18B20TempSensor[i].reportTemperature();                   // report temperature via zigbee
  }
}

void setHX711Gain(float analog_output) {
  hx711gain = analog_output;
}

void setHX711Offset(float analog_output) {
  hx711offset = analog_output;
}

void hx711_sensor_value_update() {
  // Use default values if gain is not valid
  if (hx711gain == 0) {
    hx711gain = 0.00004;
    hx711offset = 1.647;
  }
  // Timeout data retrieval after 10 tries
  for (int i = 0; i < 10; i++) {
    if (hx711.is_ready()) {
      // get raw data from HX711
      float raw_value = hx711.get_units(10);
      float weight = raw_value * hx711gain + hx711offset;
      // power down the IC to safe battery life
      hx711.power_down();

      Serial.printf("HX711 reading: raw %f gain %f offset %f weight %f", raw_value, hx711gain, hx711offset, weight);
      Serial.println();

      zbHX711Sensor.setAnalogInput(weight);
      //Serial.printf("HX711 Report: %d\n", zbHX711Sensor.reportAnalogInput());
      break;
    } else {
      Serial.printf("Retry%d: HX711 is not ready\n", i+1);
      delay(10);
    }
  }
}

void cn3791_status_update() {
  // Read status of the CN3791 output pins (inverted and with conversion to bool)
  bool cn3791_chrg = !(bool)digitalRead(CN3791_CHRG_PIN);
  bool cn3791_done = !(bool)digitalRead(CN3791_DONE_PIN);

  //Serial.printf("Charging State: %d\n", cn3791_chrg);
  //Serial.printf("Charged State: %d\n", cn3791_done);

  // Write pin status to the zigbee binary inputs and report via zigbee
  zbCN3791Chrg.setBinaryInput(cn3791_chrg);
  zbCN3791Done.setBinaryInput(cn3791_done);
  zbCN3791Chrg.reportBinaryInput();
  zbCN3791Done.reportBinaryInput();
}

void enableDeepSleep(bool state) {
  deepSleep = state;
}


/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);
  
  // Init button switch
  pinMode(BOOT_PIN, INPUT_PULLUP);

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

  initInternalTempSensor();           // Initialize internal temperature reading of the ESP32C6
  initSOCSensor();                    // Initialize analog reading for the Liion battery voltage for SoC calculation
  initDS18B20();                      // Initialize external DS18B20 temperature sensors
  initHX711();                        // Initialize HX711 scale IC
  initBinarySensor();                 // initialize Binary Status from Solar charging IC (Charging + Done) and allow sleep bit from Home Assistant

  Serial.println("Starting Zigbee...");
  // When all EPs are registered, start Zigbee in End Device mode
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Going back to sleep...");
    esp_zb_sleep_enable(true);
    esp_zb_sleep_set_threshold(3000);
    esp_zb_set_rx_on_when_idle(false);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL);
    Serial.println("Entering Deep Sleep");
    esp_deep_sleep_start();
  } else {
    Serial.println("Zigbee started successfully!");
  }
  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
}

// Loop is never used due to deep sleep. All function calls are sequential.
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

  internal_temp_sensor_value_update();
  soc_sensor_value_update();
  DS18B20_temp_sensor_value_update();
  hx711_sensor_value_update();
  cn3791_status_update();
  delay(2000);

  if (deepSleep) {
    Serial.println("Stopping Zigbee");
    Zigbee.stop();
    esp_zb_sleep_enable(true);
    esp_zb_sleep_set_threshold(3000);
    esp_zb_set_rx_on_when_idle(false);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(600ULL * 1000000ULL);
    Serial.println("Entering Deep Sleep");
    esp_deep_sleep_start();
  }

  delay(9000);
  hx711.power_up();
  delay(1000);
}
