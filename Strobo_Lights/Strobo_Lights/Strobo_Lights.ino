const int potPin = A0;
const int ledPin = 3;
const float minWait = 1;
const float maxWait = 1000;

float potPower;
float waitTime;
bool ledStatus = LOW;
float waitRange;

void setup() {
  Serial.begin(9600);  
  waitTime = maxWait;
  waitRange = maxWait - minWait;
}

void loop() {
  ledStatus = !ledStatus;
  // potPower value range (0,255)
  potPower = analogRead(potPin)/4;
  
  // waitTime value range (minWait, maxWait)
  waitTime = (255/waitRange) * potPower;

  digitalWrite(ledPin, ledStatus);
  Serial.print("PotPower: ");
  Serial.print(potPower);
  Serial.print(" | WaitTime: ");
  Serial.print(waitTime);
  Serial.println("+++++++++++++++++");
  delay(waitTime);
}
