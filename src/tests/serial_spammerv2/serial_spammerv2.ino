#define RDWR

#include "SoftwareSerial.h"

const byte RX_PIN1 = 7;
const byte TX_PIN1 = 8;
const byte CTL_PIN1 = 4;   // is used to turn RO/DI pins on RS232 to TTL converter 

const byte RX_PIN2 = 12;
const byte TX_PIN2 = 13;
const byte CTL_PIN2 = 10;

template<typename SerialType>
void serial_flush(SerialType serial) {
  while (serial.available()) serial.read();
}

void setup() {
  Serial.begin(9600);

  pinMode(13, OUTPUT);      // set LED pin as output
  digitalWrite(13, LOW);    // switch off LED pin  
}  

#ifdef RDWR
void loop() {
  while (Serial.available() > 0) {
    digitalWrite(13, HIGH);
    char rxbyte;
    Serial.readBytes(&rxbyte, 1);
    Serial.write(rxbyte);    
  }
}
#endif