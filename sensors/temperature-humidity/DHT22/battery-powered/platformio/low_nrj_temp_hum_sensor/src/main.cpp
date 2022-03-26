#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include "DHT.h"

// Function Declarations
String GetArduinoUniqueID();
long readVcc();

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

  // Measure battery voltage with readVcc()
  Serial.print("Battery voltage: ");
  Serial.print(readVcc());
  Serial.print("mV\n");
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