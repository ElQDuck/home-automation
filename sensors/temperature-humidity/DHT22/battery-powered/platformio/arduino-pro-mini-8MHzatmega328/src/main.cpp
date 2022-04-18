#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include "DHT.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>
#include <string.h>

// Function Declarations
String GetArduinoUniqueID();
long readVcc();
void log(String logMessage);

// Constants
bool DEBUG = true; // Set true to activate serial messages, false in final code.
const String ARDUINO_ID = GetArduinoUniqueID(); // The unique Arduino ID
const uint16_t other_node = 00;      // Address of the other node (gateway) in Octal format
const uint16_t this_node = 01;       // Address of our node (sensor) in Octal format

struct payload_t {                   // Structure of our payload
  String ms;
  unsigned long counter;
};

unsigned long packets_sent;          // How many have we sent already

DHT dht;                             // The DHT Sensor
RF24 radio(9, 10);                   // CE, CSN
RF24Network network(radio);          // Network uses that radio

void setup()
{
  Serial.begin(9600);
  dht.setup(3); // data pin 3

  if (!radio.begin()) {
    log("Radio hardware not responding!");
    while (1) {
      // hold in infinite loop
    }
  }
  radio.setChannel(90);
  network.begin(this_node);
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  String json = "";

  // Print value if both values are real numbers (prevent NaN print)
  if (!isnan(humidity) && !isnan(temperature))
  {
    // JSON Example:
    // {
    // 	  "DeviceID": "88255223255823855255255255",
    //    "BatteryVoltage" 3.15, (V)
    //    "SensorType": "DHT22",
    // 	  "Humidity": 48.0, (%)
    // 	  "Temperature": 21.1 (°C)
    // }
    json = "{\"DeviceID\":\"" + String(ARDUINO_ID) + "\", \
            \"BatteryVoltage\":" + readVcc() / 1000. + ", \
            \"SensorType\":\"DHT22\", \
            \"Humidity\":" + String(dht.getHumidity()) + ", \
            \"Temperature\":" + String(dht.getTemperature()) + "}";
    log(json);
  }

  network.update(); // Check the network regularly
  // If it's time to send a message, send it!
    int stringLength = json.length();
    char msg[stringLength + 1];
    strcpy(msg, json.c_str());

    log("Sending... ");
    log(json);
    log("With size:");
    uint16_t msgLength = sizeof(msg);
    log(String(msgLength));
    RF24NetworkHeader header(/*to node*/ other_node);
    bool ok = network.write(header, &msg, msgLength);
    log(ok ? "ok." : "failed.");
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

// TODO: make a reference measurement
// Used from https://forum.arduino.cc/t/how-to-know-vcc-voltage-in-arduino/344001
// Reads the Voltage in mV
long readVcc() {
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
void log(String logMessage){
  if(DEBUG){
    Serial.println(logMessage);
  }
}