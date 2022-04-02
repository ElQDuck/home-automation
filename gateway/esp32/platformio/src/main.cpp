#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

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
const byte address[6] = "00001";

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
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop(){
  // Radio
  if (radio.available()) {
    log("Radio Availible");
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
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