#define WRITER // defines if an arduino is going to be 
               // a sender or a receiver

#include <SoftwareSerial.h>

int counter = 15;

int SWRX_PIN = 7;
int SWTX_PIN = 8;
int CTL_PIN = 4;

SoftwareSerial swSerial1(SWRX_PIN, SWTX_PIN, CTL_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);      // set LED pin as output
  digitalWrite(13, LOW);    // switch off LED pin  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}

#ifdef WRITER
void loop() {
  if (counter > 0) {
    Serial.write(counter);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
    counter--;
  } else {
    Serial.write('\0');
  }
}
#endif

#ifdef READER
void loop() {
  if (swSerial1.available() > 0) {
    size_t size = 256;
    
    char rx_bytes[size];

    swSerial1.readBytes(rx_bytes, size);
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(200);
    Serial.write("Received data");
  } else {
    Serial.write('\0');
  }
}
#endif