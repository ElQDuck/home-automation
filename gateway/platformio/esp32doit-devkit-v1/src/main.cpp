#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ArduinoJson.h>


// Constants
bool DEBUG = true; // Set true to activate serial messages, false in final code.

// TODO: Set mqtt broker address in web interface
const char* mqtt_server = "192.168.178.65";

// Globals
bool mqttConnected = false;

// Set web server port number to 80
WebServer server(80);

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<h2>Circuits4you<h2>
<h3> HTML Form ESP8266</h3>

<form action="/action_page">
  First name:<br>
  <input type="text" name="firstname" value="Mickey">
  <br>
  Last name:<br>
  <input type="text" name="lastname" value="Mouse">
  <br><br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
 String firstName = server.arg("firstname"); 
 String lastName = server.arg("lastname"); 

 Serial.print("First Name:");
 Serial.println(firstName);

 Serial.print("Last Name:");
 Serial.println(lastName);
 
 String s = "<a href='/'> Go Back </a>";
 server.send(200, "text/html", s); //Send web page
}

void handleStatus(){
  if (mqttConnected)
  {
    server.send(200, "text/html", "MQTT connected"); //Send web page
  }
  else
  {
    server.send(200, "text/html", "MQTT disconnected"); //Send web page
  }
}


// Radio
RF24 radio(17, 5); // CE, CSN
RF24Network network(radio);     // Network uses that radio
const uint16_t this_node = 00;  // Address of our node in Octal format (04, 031, etc)
const uint16_t other_node = 01; // Address of the other node in Octal format

// Wifi and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

struct payload_t {              // Structure of our payload
  String ms;
  unsigned long counter;
};

// custom functions
void log(String logMessage);
void MqttReconnect();
void publishBME280(StaticJsonDocument<256> bmeValuesJson);
void publishString(String topic, String value);

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }

  // Wifi Manager
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  WiFiManager wm;       //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  // wm.resetSettings();   // reset settings - wipe stored credentials for testing
  bool res = wm.autoConnect("SensorGateway","verrySavePassword"); // password protected Access Point
  if(!res) {
    log("Failed to connect");
  } 
  else {
    log("WiFi connected.");
    log("IP address: ");
    Serial.println(WiFi.localIP());
    server.on("/", handleRoot);      //Which routine to handle at root 
    server.on("/action_page", handleForm); //form action is handled here
    server.on("/status", handleStatus); // Show mqtt connection status
    server.begin();                  //Start server
  }

  // MQTT
  client.setServer(mqtt_server, 1883);

  // Radio
  // radio.begin();
  // radio.openReadingPipe(0, address);
  // radio.setPALevel(RF24_PA_MIN);
  // radio.startListening();
  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (1) {
      // hold in infinite loop
    }
  }
  radio.setChannel(90);
  network.begin(/*node address*/ this_node);
}



void loop(){
  // Radio
  // if (radio.available()) {
  //   log("Radio Availible");
  //   char text[32] = "";
  //   radio.read(&text, sizeof(text));
  //   Serial.println(text);
  // }
  network.update();                  // Check the network regularly
 
  while (network.available()) {      // Is there anything ready for us?
    
    Serial.println(F("Reading header"));
    RF24NetworkHeader header;        // If so, grab it and print it out
    Serial.print(F("Getting payload size: "));
    uint16_t payloadSize = network.peek(header); // Use peek() to get the size of the payload
    char payload[payloadSize];
    Serial.print(String(payloadSize));
    Serial.print("\n");
    Serial.println(F("Read network"));
    Serial.print("\n");
    network.read(header, &payload, payloadSize);
    Serial.print("Received packet, size ");         // Print info about received data
    Serial.print(sizeof(payload));
    Serial.print("\n");
    Serial.println(F("Message: "));
    Serial.println(payload);

    // Convert payload to json for further work
    StaticJsonDocument<256> jsonPayload;
    DeserializationError error = deserializeJson(jsonPayload, payload);
    
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // TODO: switch case for device ids
    publishBME280(jsonPayload);
  }

  // MQTT
  if (!client.connected()) {
    MqttReconnect();
  }

  client.loop(); //TODO: Check if this is neccessary
  //client.publish("homie/device123/mythermostat/temperature", "21", true); // topic, value, retained
  //delay(10000); // TODO: Delete after testing

  // WebServer
  server.handleClient();          //Handle client requests

  // JSON tryout
  StaticJsonDocument<200> doc;
  char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  DeserializationError error = deserializeJson(doc, json);
  
  const char* sensor = doc["sensor"];
  long time = doc["time"];
  double latitude = doc["data"][0];
  double longitude = doc["data"][1];

  // Print values.
  // Serial.println(sensor);
  // Serial.println(time);
  // Serial.println(latitude, 6);
  // Serial.println(longitude, 6);
}


// A logger for debugging
void log(String logMessage){
  if(DEBUG){
    Serial.println(logMessage);
  }
}

// Reconnect to MQTT
void MqttReconnect() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      mqttConnected = true;
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      mqttConnected = false;
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
}

void publishBME280(StaticJsonDocument<256> bmeValuesJson){
  /*
  homie / device123 / $homie → 3.0
  homie / device123 / $name → My device
  homie / device123 / $state → ready
  homie / device123 / $nodes → mythermostat

  homie / device123 / mythermostat / $name → My thermostat
  homie / device123 / mythermostat / $properties → temperature

  homie / device123 / mythermostat / temperature → 22 
  homie / device123 / mythermostat / temperature / $name → Temperature
  homie / device123 / mythermostat / temperature / $unit → °C
  homie / device123 / mythermostat / temperature / $datatype → integer
  homie / device123 / mythermostat / temperature / $settable → true
  */
  // https://homieiot.github.io/specification/
  // The following device attributes are mandatory and MUST be send, even if it is just an empty string.
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/$homie", strdup("4.0.0"));                // homie/88255223255223255255255255/$homie → 4.0.0
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/$name", strdup("Temp. and Hum. Sensor")); // homie/88255223255223255255255255/$name → Temp. and Hum. Sensor
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/$state", strdup("ready"));                // homie/88255223255223255255255255/$state → ready
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/$nodes", strdup("bme280"));               // homie/88255223255223255255255255/$nodes → bme280
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/$extensions", strdup(""));                // homie/88255223255223255255255255/$extensions → 

  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/$name", strdup("BME280 Sensor"));  // homie/88255223255223255255255255/bme280/$name → BME280 Sensor
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/$properties", strdup("battery-voltage,temperature,humidity,pressure"));  // homie/88255223255223255255255255/bme280/$properties → battery-voltage,temperature,humidity,pressure

  // battery-voltage
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/battery-voltage", bmeValuesJson["BatteryVoltageValue"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/battery-voltage/$name", strdup("Battery Voltage"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/battery-voltage/$unit", bmeValuesJson["BatteryVoltageUnit"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/battery-voltage/$datatype", strdup("float"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/battery-voltage/$settable", strdup("false"));
  // temperature
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/temperature", bmeValuesJson["TemperatureValue"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/temperature/$name", strdup("Temperature"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/temperature/$unit", bmeValuesJson["TemperatureUnit"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/temperature/$datatype", strdup("float"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/temperature/$settable", strdup("false"));
  // humidity
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/humidity", bmeValuesJson["HumidityValue"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/humidity/$name", strdup("Humidity"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/humidity/$unit", bmeValuesJson["HumidityUnit"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/humidity/$datatype", strdup("float"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/humidity/$settable", strdup("false"));
  // pressure
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/pressure", bmeValuesJson["PressureValue"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/pressure/$name", strdup("Pressure"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/pressure/$unit", bmeValuesJson["PressureUnit"].as<String>());
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/pressure/$datatype", strdup("float"));
  publishString("homie/" + bmeValuesJson["DeviceID"].as<String>() + "/bme280/pressure/$settable", strdup("false"));
}

void publishString(String topic, String value){
  int stringLength = topic.length();
  char ctopic[stringLength + 1];
  strcpy(ctopic, topic.c_str());

  stringLength = value.length();
  char cvalue[stringLength + 1];
  strcpy(cvalue, value.c_str());

  client.publish(ctopic, cvalue, true); // topic, value, retained
}