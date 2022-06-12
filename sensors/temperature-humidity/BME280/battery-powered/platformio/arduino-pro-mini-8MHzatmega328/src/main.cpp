#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include "SparkFunBME280.h"
#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>
#include <string.h>
#include "LowPower.h"

// Structs
struct BME_VALUES {char humidity[6]; char pressure[8]; char altitude[6]; char temperature[6];};

// Function Declarations
double ReadVccInV();
void Log(String logMessage);
BME_VALUES ReadBmeValues();

// Constants
const bool DEBUG = true;                        // Set true to activate serial messages, false in final code.
const uint16_t receiverAddress = 00;            // Address of the other node (gateway) in Octal format TODO: Rename variable name
const uint16_t senderAddress = 01;              // Address of our node (sensor) in Octal format

BME280 bme;                          // The BME280 Sensor
RF24 radio(9, 10);                   // CE, CSN
RF24Network network(radio);          // Network uses that radio

// Global variables
int sleepCounter = 0;                // Counts the number of consecutive sleeps

void setup()
{
  Serial.begin(115200);

  // BME280 configuration
  Wire.begin();
  Wire.setClock(400000);    //Increase to fast I2C speed!
  bme.setI2CAddress(0x76);
  bme.beginI2C();
  bme.setMode(MODE_SLEEP);  //Sleep for now

  // NRF24L01 configuration
  if (!radio.begin()) {
    Log("Radio hardware not responding!");
    while (1) {
      // hold in infinite loop
    }
  }
  radio.setChannel(90);
  network.begin(senderAddress);
}

void loop()
{
  sleepCounter ++;  // The counter for the sleep cycles

  // The deep sleep mode can only sleep for max. 8 seconds.
  // For a sleep period of >8s we have to go in sleep mode periodicaly.
  // Example: Sleep for 16s: sleepCounter >= 2 -> 2*8s=16s
  // If in debug mode sleep for 2 = 16 sec, if not sleep for 30 = 240 sec = 4 min
  if (sleepCounter >= (DEBUG ? 2 : 30))
  {
    BME_VALUES sensorValues = ReadBmeValues();

    // JSON Example:
    // {
    //   "DeviceID": "88255223255223255255255255",
    //   "BatteryVoltageValue": 3.22,
    //   "BatteryVoltageUnit": "V",
    //   "SensorType": "BME280",
    //   "TemperatureValue": 18.39,
    //   "TemperatureUnit": "°C",
    //   "HumidityValue": 52.27,
    //   "HumidityUnit": "%",
    //   "PressureValue": 1007.08,
    //   "PressureUnit": "hPa"
    // }
    char json[251];
    uint16_t jsonLength = sizeof(json);
    snprintf_P(json, jsonLength, PSTR("{\"DeviceID\":\"%s\",\"BatteryVoltageValue\":%s,\"BatteryVoltageUnit\":\"V\",\"SensorType\":\"BME280\",\"TemperatureValue\":%s,\"TemperatureUnit\":\"°C\",\"HumidityValue\":%s,\"HumidityUnit\":\"%%\",\"PressureValue\":%s,\"PressureUnit\":\"hPa\"}"), String(senderAddress).c_str(), String(ReadVccInV()).c_str(), sensorValues.temperature, sensorValues.humidity, sensorValues.pressure);

    radio.powerUp();  // Wake up the RF24 from sleep mode
    network.update(); // Check if the network is up and ready    

    // Print the json string for debuging
    if (DEBUG)
    {
      for (size_t i = 0; i < jsonLength; i++)
      {
        Serial.print(json[i]);
      }
    }
    
    Log("\nSending data with size " + String(jsonLength));
    RF24NetworkHeader header(receiverAddress);  // To Gateway
    bool ok = network.write(header, &json, jsonLength);
    Log(ok ? "ok." : "failed.");

    radio.powerDown();  // Put the RF24 into sleep mode: https://nrf24.github.io/RF24/examples_2old_backups_2pingpair_sleepy_2pingpair_sleepy_8ino-example.html
    sleepCounter = 0;   // Reset the sleep counter to put the Arduino into sleep mode
    delay(100);         // delay before go to sleep to finish background work TODO Check if necessary
  }

  // Sleep
  // For more information on the different types of sleep mode refer to: https://elektro.turanis.de/html/prj270/index.html
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

// Reads the Voltage in mV
// TODO: make a reference measurement
// Used from https://forum.arduino.cc/t/how-to-know-vcc-voltage-in-arduino/344001
double ReadVccInV() {
  long result;
  double resultInVolt;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  resultInVolt = result / 1000.;
  return resultInVolt;
}

// A logger for debugging
void Log(String logMessage){
  if(DEBUG){
    Serial.println(logMessage);
  }
}

// Read values from BME280 Sensor
// Code from: https://github.com/sparkfun/SparkFun_BME280_Arduino_Library/blob/master/examples/Example6_LowPower/Example6_LowPower.ino
BME_VALUES ReadBmeValues() {
  BME_VALUES returnValue;

  bme.setMode(MODE_FORCED);           // Wake up sensor and take reading
  while(bme.isMeasuring() == false);  // Wait for sensor to start measurment
  while(bme.isMeasuring() == true);   // Hang out while sensor completes the reading

  // Sensor is now back asleep but the data can be read from its registers
  snprintf_P(returnValue.humidity, 6, PSTR("%s"), String(bme.readFloatHumidity(), 2).c_str());       // in %
  snprintf_P(returnValue.pressure, 8, PSTR("%s"), String(bme.readFloatPressure()/100, 2).c_str());   // in hPa
  snprintf_P(returnValue.altitude, 6, PSTR("%s"), String(bme.readFloatAltitudeMeters(), 2).c_str()); // in m
  snprintf_P(returnValue.temperature, 6, PSTR("%s"), String(bme.readTempC(), 2).c_str());            // in °C

  return returnValue;
}