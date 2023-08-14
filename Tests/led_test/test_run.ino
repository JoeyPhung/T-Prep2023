int myPin = 21;
void setup() {
  // put your setup code here, to run once:
  pinMode(myPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(myPin, HIGH);
  delay(500);
  digitalWrite(myPin, LOW);
  delay(500);
}
