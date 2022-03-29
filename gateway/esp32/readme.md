# Used hardware
- DOIT ESP 32 DEVKIT V1
- Wifi connection: [NRF24L01 2.4GHz]()

documentation:
https://github.com/nRF24/RF24/issues/393

wiring:
CE -> GPIO17 GPIO
CSN -> GPIO05 VSPI SS
MISO -> GPIO19 VSPI MISO
MOSI -> GPIO23 VSPI MOSI
CLK -> GPIO18 VSPI CLK
IRQ-> unconnected

go to RF24_config.h and just after line 133 add the following code:
```C
#elif defined (ESP32)
    #include <pgmspace.h>
    #define PRIPSTR "%s"
```