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

// Function Declarations
struct BME_VALUES {float humidity; float pressure; float altitude; float temperature;};

String GetArduinoUniqueID();
long ReadVcc();
void Log(String logMessage);
BME_VALUES ReadBmeValues();


// Constants
bool DEBUG = true; // Set true to activate serial messages, false in final code.
const String ARDUINO_ID = GetArduinoUniqueID(); // The unique Arduino ID
const uint16_t other_node = 00;      // Address of the other node (gateway) in Octal format TODO: Rename variable name
const uint16_t this_node = 01;       // Address of our node (sensor) in Octal format

struct payload_t {                   // Structure of our payload
  String ms;
  unsigned long counter;
};

unsigned long packets_sent;          // How many have we sent already
int sleepCounter = 0;                // Counts the number of consecutive sleeps

BME280 bme;                          // The BME280 Sensor
RF24 radio(9, 10);                   // CE, CSN
RF24Network network(radio);          // Network uses that radio

void setup()
{
  Serial.begin(115200);

  // BME280
  Wire.begin();
  Wire.setClock(400000); //Increase to fast I2C speed!
  bme.setI2CAddress(0x76);
  bme.beginI2C();
  bme.setMode(MODE_SLEEP); //Sleep for now

  if (!radio.begin()) {
    Log("Radio hardware not responding!");
    while (1) {
      // hold in infinite loop
    }
  }
  radio.setChannel(90);
  network.begin(this_node);
}

void loop()
{
  sleepCounter ++;  // The counter for the sleep cycles

  // The deep sleep mode can only sleep for max. 8 seconds.
  // For a sleep period of >8s we have to go in sleep mode periodicaly.
  // Example: Sleep for 16s: sleepCounter >= 2 -> 2*8s=16s
  // If in debug mode sleep for 16 seconds, if not sleep for 4 minutes
  if (sleepCounter >= (DEBUG ? 2 : 30))
  {
    BME_VALUES sensorValues = ReadBmeValues();

    // JSON Example:
    // {
    // 	  "DeviceID": "88255223255823855255255255",
    //    "BatteryVoltage" 3.15, (V)
    //    "SensorType": "BME280",
    // 	  "Temperature": 21.1 (°C)
    // 	  "Humidity": 48.0, (%)
    //    "Pressure": 996.63 (hPa)
    // }
    

    // TODO: replace json with char[]

    String json = "askldjalskdjaskldjalskdjaskldjalskdjaskldjalskdjask";
    //String json = "{\"DeviceID\":\"" + ARDUINO_ID + "\", \
    //         \"BatteryVoltageValue\":" + ReadVcc() / 1000. + ", \
    //         \"BatteryVoltageUnit\": \"V\", \
    //         \"SensorType\":\"BME280\", \
    //         \"TemperatureValue\":" + sensorValues.temperature + ", \
    //         \"TemperatureUnit\": \"°C\", \
    //         \"HumidityValue\":" + sensorValues.humidity + ", \
    //         \"HumidityUnit\": \"%\", \
    //         \"PressureValue\":" + sensorValues.pressure + ", \
    //         \"PressureUnit\": \"hPa\"}";
    Log(json);

    radio.powerUp();  // Wake up the RF24 from sleep mode
    network.update(); // Check the network regularly

    // Convert the json string into a char array because we cant use the String for sending
    int stringLength = json.length();
    char msg[stringLength + 1];
    strcpy(msg, json.c_str());
    uint16_t msgLength = sizeof(msg);

    Log("Sending... " + json);
    Log("With size: " + String(msgLength));
    RF24NetworkHeader header(other_node); // To Gateway
    bool ok = network.write(header, &msg, msgLength);
    Log(ok ? "ok." : "failed.");

    radio.powerDown();  // Put the RF24 into sleep mode: https://nrf24.github.io/RF24/examples_2old_backups_2pingpair_sleepy_2pingpair_sleepy_8ino-example.html
    sleepCounter = 0;   // Reset the sleep counter to put the Arduino into sleep mode
    delay(100);         // delay before go to sleep to finish background work
  }

  // Sleep
  // For more information on the different types of sleep mode refer to: https://elektro.turanis.de/html/prj270/index.html
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

// Function to get the ArduinoUniqueID
String GetArduinoUniqueID(){
  String id = "";
  for (size_t i = 0; i < UniqueIDsize; i++)
	{
		if (UniqueID[i] < 0x10)
      id += "0";
		id += String(UniqueID[i]);
	}
  return id;
}

// Reads the Voltage in mV
// TODO: make a reference measurement
// Used from https://forum.arduino.cc/t/how-to-know-vcc-voltage-in-arduino/344001
long ReadVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

// A logger for debugging
void Log(String logMessage){
  if(DEBUG){
    Serial.println(logMessage);
  }
}

// Read values from BME280 Sensor
// Code from: https://github.com/sparkfun/SparkFun_BME280_Arduino_Library/blob/master/examples/Example6_LowPower/Example6_LowPower.ino
// TODO: Return Values (replace void)
BME_VALUES ReadBmeValues() {
  BME_VALUES returnValue;

  bme.setMode(MODE_FORCED);           // Wake up sensor and take reading
  while(bme.isMeasuring() == false);  // Wait for sensor to start measurment
  while(bme.isMeasuring() == true);   // Hang out while sensor completes the reading

  // Sensor is now back asleep but the data can be read from its registers
  returnValue.humidity = bme.readFloatHumidity();       // in %
  returnValue.pressure = bme.readFloatPressure() / 100; // in hPa
  returnValue.altitude = bme.readFloatAltitudeMeters(); // in m
  returnValue.temperature = bme.readTempC();            // in °C

  Log("Humidity: " + String(returnValue.humidity) + "%");
  Log("Pressure: " + String(returnValue.pressure) + "hPa");
  Log("Altitude: " + String(returnValue.altitude) + "m");
  Log("Temperature: " + String(returnValue.temperature) + "°C");

  return returnValue;
}