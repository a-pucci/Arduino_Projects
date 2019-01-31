void setup() {
  const int potPin = A0

}

void loop() {
  // put your main code here, to run repeatedly:
  analogRead(potPin);
  serial.print(potPin)_Light;
}
