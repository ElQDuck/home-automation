# A battery powered temperature and humidity sensor
Used Hardware
- Sensor: [DHT22]()
- Microcontroller: [Arduino Pro Mini ATmega328 3.3V 8MHz ]()
- Wifi connection: [NRF24L01 2.4GHz]()
- Battery: 2 x AA-Battery
- Programmer: [FTDI USB to TTL Serial Adapter](https://www.mikrocontroller.net/topic/435491)
    - [Drivers](https://ftdichip.com/drivers/)

## Development Environment Setup
1. Install [Visual Studio Code](https://code.visualstudio.com/download)
2. Install [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
    1. C++ libraries:
        1. [DHT](https://github.com/markruys/arduino-DHT)
        2. [rapidjson](https://github.com/Tencent/rapidjson)
        3. [ArduinoUniqueID](https://github.com/ricaun/ArduinoUniqueID)
        4. [MySensors](https://github.com/mysensors/MySensors/tree/master)
        5. [RF24](https://github.com/nRF24/RF24)
        6. [RF24Network](https://github.com/nRF24/RF24Network)
        7. [pubsubclient](https://github.com/knolleary/pubsubclient)
4. Install [KiCad]()
    1. [Arduino Libary](https://github.com/Duckle29/kicad-libraries)
    2. [NRF24L01 Library](https://github.com/myelin/myelin-kicad-libraries)
    3. [DHT22 Sensor Library](https://github.com/skorokithakis/kicad-lib.git)
    
    [//]: # (TODO: Move libs into repository)
    
5. (Alternative) install [FreeCAD]()

## Wiring

## Programming
!Attention! After pressing the upload button press the reset button on the Arduino pro mini.