#include <Arduino.h>
#include <ArduinoUniqueID.h>
#include "DHT.h"

// Function Declarations
String GetArduinoUniqueID();
uint16_t hwCPUVoltage();

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

  // Measure battery voltage
  long batteryMillivolts = hwCPUVoltage();
  int batteryPcnt = batteryMillivolts / 3 / 1000.0 * 100 + 0.5;

  Serial.print("Battery voltage: ");
  Serial.print(batteryMillivolts / 1000.0);
  Serial.println("V\n");
  Serial.print("Battery percent: ");
  Serial.print(batteryPcnt);
  Serial.println(" %\n");

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

// TODO: Cleanup and make a reference measurement
uint16_t hwCPUVoltage()
{
	// Measure Vcc against 1.1V Vref
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	ADMUX = (_BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1));
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	ADMUX = (_BV(MUX5) | _BV(MUX0));
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	ADMUX = (_BV(MUX3) | _BV(MUX2));
#else
	ADMUX = (_BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1));
#endif
	// Vref settle
	delay(70);
	// Do conversion
	ADCSRA |= _BV(ADSC);
	while (bit_is_set(ADCSRA,ADSC)) {};
	// return Vcc in mV
	return (1125300UL) / ADC;
}