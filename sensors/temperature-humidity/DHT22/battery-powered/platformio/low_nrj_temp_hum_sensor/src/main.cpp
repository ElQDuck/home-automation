#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include "DHT.h"

// Function Declarations
String GetArduinoUniqueID();

// Constants
const String ARDUINO_ID = GetArduinoUniqueID(); // The unique Arduino ID

DHT dht; // The DHT Sensor

void setup()
{
  Serial.begin(9600);
  dht.setup(3); // data pin 3
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  // Print value if both values are real numbers (prevent NaN print)
  if (!isnan(humidity) && !isnan(temperature))
  {
    // JSON Example:
    // {
    // 	  "DeviceID": "88255223255223255255255255",
    // 	  "Humidity": 48.0,
    // 	  "Temperature": 21.1
    // }
    String json = "{\"DeviceID\":\"" + String(ARDUINO_ID) + "\",\"Humidity\":" + String(dht.getHumidity()) + ",\"Temperature\":" + String(dht.getTemperature()) + "}";
    Serial.print(json);
    Serial.print("\n");
  }
  
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