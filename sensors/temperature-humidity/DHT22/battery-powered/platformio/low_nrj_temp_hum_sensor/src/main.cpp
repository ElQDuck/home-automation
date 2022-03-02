#include <Arduino.h>
#include "DHT.h"

DHT dht;

void setup()
{
  Serial.begin(9600);

  dht.setup(3); // data pin 3
}

void loop()
{
  delay(dht.getMinimumSamplingPeriod());

  Serial.print("\nFeuchtigkeit: ");
  Serial.print(dht.getHumidity());
  Serial.print("\nTemperature: ");
  Serial.print(dht.getTemperature());
  Serial.print("\n");
}