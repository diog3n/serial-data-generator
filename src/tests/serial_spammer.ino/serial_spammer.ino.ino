#define TESTER // defines if an arduino is going to be 
               // a sender or a receiver

int counter = 15;
int RELAY_PIN = 11;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);      // set LED pin as output
  digitalWrite(13, LOW);    // switch off LED pin  
  pinMode(RELAY_PIN, INPUT);
}

#ifdef TESTER
void loop() {
  if (counter > 0) {
    Serial.write(counter * (counter * counter + counter));
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
    counter--;
  }
  else {
    Serial.write('\0');
  }
}
#endif

#ifdef WRITER
void loop() {
  if (counter > 0) {
    Serial.write(counter);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
    counter--;
}
#endif

#ifdef READER
void loop() {
  if (digitalRead(RELAY_PIN) == LOW) {
    Serial.write(0b1);
  } else {
    Serial.write(0b0);
  }
}
#endif