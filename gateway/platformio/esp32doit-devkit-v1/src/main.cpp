#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>
#include <RF24Ethernet.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <MQTT.h>


// Constants
bool DEBUG = true; // Set true to activate serial messages, false in final code.

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


// Radio
RF24 radio(17, 5); // CE, CSN
RF24Network network(radio);     // Network uses that radio
const uint16_t this_node = 00;  // Address of our node in Octal format (04, 031, etc)
const uint16_t other_node = 01; // Address of the other node in Octal format

struct payload_t {              // Structure of our payload
  String ms;
  unsigned long counter;
};

// custom functions
void log(String logMessage);

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
    log(WiFi.localIP());
    server.on("/", handleRoot);      //Which routine to handle at root 
    server.on("/action_page", handleForm); //form action is handled here
    server.begin();                  //Start server
  }

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
  }

  // WebServer
  server.handleClient();          //Handle client requests
}

// A logger for debugging
void log(String logMessage){
  if(DEBUG){
    Serial.println(logMessage);
  }
}