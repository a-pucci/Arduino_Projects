#include "ClickButton.h"
#include <SPI.h>
#include <WiFi101.h>
#include <RTCZero.h>


//****************************** Codice per luci ******************************

// Lampadario
const int chandPin = 2;
bool chandStatus = true;

// Striscia Led
const int stripPin = 3;
bool stripStatus = true;

// Faretto
const int spotPin = 5;
bool spotStatus = true;

//Presa Comodino
const int socketPin = 4;
bool socketStatus = true;

// Pulsante Porta
const int doorButtonPin = 8;
ClickButton doorButton(doorButtonPin, HIGH);

//Pulsante Comodino
const int bedButtonPin = 9;
ClickButton bedButton(bedButtonPin, HIGH);

//Sensore Magnetico Porta
const int magnetPin = 6;

//Sensore Fororesistore
const int phototransistorPin = A0;

// Costante valore di luce Fototransistor
const int LIGHTVALUE = 50;

//Variabili 
int doorFunction = 0;
int bedFunction = 0;
int phototransistorValue;
int newMagnetValue;
int oldMagnetValue; 

//Variabili per il reset del WiFi
int timeToReset = 1;
bool isTimeToResetWifi = false;
int millisCount = 0;
int secondsCount = 0;
int minutesCount = 0;
int hoursCount = 0;

//Variabili per l'ora
const int GMT = 1;
RTCZero rtc;

//****************************** Codice per WIFI ******************************

char ssid[] = "Telecom-22050449";      //  your network SSID (name)
char pass[] = "GiuliaStudia!0";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() 
{
   Serial.begin(9600);
   
// ****************************** Setup Luci ******************************
  
  // Set Pin OUTPUT
  pinMode(chandPin, OUTPUT);   // Pin lampadario
  pinMode(stripPin, OUTPUT);   // Pin striscia led
  pinMode(spotPin, OUTPUT);    // Pin faretto
  pinMode(socketPin, OUTPUT);  // Pin presa comodino

  // Set Pin INPUT
  pinMode(magnetPin, INPUT);      // Pin sensore magnetico
  pinMode(phototransistorPin, INPUT);  // Pin sensore fotoresistore
  

  // Set timer del pulsante in ms
  doorButton.debounceTime = 20;       // Tempo Debounce
  doorButton.multiclickTime = 200;    // Tempo per i multi click
  doorButton.longClickTime = 500;    // Tempo per registrare i click lunghi

  bedButton.debounceTime = 20;       // Tempo Debounce
  bedButton.multiclickTime = 200;    // Tempo per i multi click
  bedButton.longClickTime = 500;    // Tempo per registrare i click lunghi

//****************************** Setup WIFI ******************************

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status

//****************************** Setup Orario ******************************

  rtc.begin();

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do 
  {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) || (numberOfTries > maxTries));

  if (numberOfTries > maxTries) 
  {
    Serial.print("NTP unreachable!!");
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);

    Serial.println();
  }
}

void loop() 
{
//****************************** Loop Luci ******************************
  
  doorButton.Update();
  bedButton.Update();
  newMagnetValue = digitalRead(magnetPin);
  phototransistorValue = analogRead(phototransistorPin);
  /*Serial.println();
  Serial.print("Sensore Magnetico Prima: ");
  Serial.print(oldMagnetValue);
  Serial.print("  Sensore Magnetico Dopo: ");
  Serial.print(newMagnetValue);
  Serial.print("  Sensore Fototransistor: ");
  Serial.print(phototransistorValue);
  Serial.print("  ");
  printDate();
  Serial.print("\t");
  printTime();*/ 

  // Se la porta si apre ed è buio si accende il Lampadario e e la Striscia LED
  if(oldMagnetValue == 1 && newMagnetValue == 0 && phototransistorValue < LIGHTVALUE && (rtc.getHours() + GMT) < 23 && (rtc.getHours() + GMT) > 12)
  {
      chandStatus = false;
      stripStatus = false;
      spotStatus = false;   
  }
  
  // Salva il numero dei click nella variabile doorFunction
  if(doorButton.clicks != 0) doorFunction = doorButton.clicks;
  if(bedButton.clicks != 0) bedFunction = bedButton.clicks;
    
  // Accende/Spegne la presa del comodino
  if(bedButton.clicks == 1) 
  {
    socketStatus = !socketStatus;
  }

  // Accende/Spegne il Lampadario se 1 click
  if(doorFunction == 1) 
  {
    chandStatus = !chandStatus;
  }
  
  // Accende/Spegne Lampadario, Striscia LED e Faretto se 2 click 
  if(doorFunction == 2) 
  {
     if(chandStatus == stripStatus)
    {
      chandStatus = !chandStatus;
      stripStatus = !stripStatus;
      spotStatus = !spotStatus;
    }
    if(chandStatus == false && stripStatus == true)
    {
      stripStatus = false;
      spotStatus = false;

    }
    if(chandStatus == true && stripStatus == false)
    {
      chandStatus = false;
    }
  }

  // Accende/Spegne il faretto
  if(doorFunction == 3)
  {
    spotStatus = !spotStatus;
  }

  // Spegne tutto se premuto 1 secondo
  if(doorFunction == -1)
  {
    chandStatus = true;
    stripStatus = true;
    spotStatus = true;
    socketStatus = true;
  }

  // Accende/Spegne la Striscia LED
  if(doorFunction == -2)
  {
    stripStatus = !stripStatus;
  }  

  
   // Reset del WiFi
  if(checkTimeForWifiReset())
  {
    WiFi.disconnect();
    Serial.println("Disconnected from the network.");
    delay(10000);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);  
    server.begin();                           // start the web server on port 80
    printWifiStatus();                        // you're connected now, so print out the status
    bedButton.clicks = 0;
  }

  // Aggiorna le uscite
  digitalWrite(chandPin, chandStatus);
  digitalWrite(stripPin, stripStatus);
  digitalWrite(spotPin, spotStatus);
  digitalWrite(socketPin, socketStatus);
  doorFunction = 0;
  bedFunction = 0;
  oldMagnetValue = newMagnetValue;


//****************************** Loop WIFI ******************************

  WiFiClient client = server.available();   // listen for incoming clients

    if (client) {                             // if you get a client,
     // Serial.println("new client");           // print a message out the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      while (client.connected()) {            // loop while the client's connected
        if (client.available()) {             // if there's bytes to read from the client,
          char c = client.read();             // read a byte, then
          //Serial.write(c);                    // print it out the serial monitor
          if (c == '\n') {                    // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
  
              // the content of the HTTP response follows the header:

              // Ora
              client.print("ORA: ");
              client.print(rtc.getHours() + GMT);
              client.print(":");
              if (rtc.getMinutes() < 10) 
              {
                client.print("0");
              }
              client.print(rtc.getMinutes());
              client.println("<br>");              
              
              // Lampadario
              client.print("Lampadario: ");
              if(chandStatus)
              {
                client.println("Spento<br>");              
              }
              else
              {
               client.println(" Acceso<br>");
              }         
              // Striscia Led
              client.print("Striscia Led: ");
              if(stripStatus)
              {
                client.println("Spento<br>");               
              }
              else
              {
                client.println("Acceso<br>");
              } 
              // Faretto
              client.print("Faretto: ");
              if(spotStatus)
              {
                client.println("Spento<br>");               
              }
              else
              {
                client.println("Acceso<br>");
              }
              
              // Comodino
              client.print("Comodino: ");
              if(socketStatus)
              {
                client.println("Spento<br>");               
              }
              else
              {
                client.println("Acceso<br>");
              } 
             
              
  
              // The HTTP response ends with another blank line:
              client.println();
              // break out of the while loop:
              break;
            }
            else {      // if you got a newline, then clear currentLine:
              currentLine = "";
            }
          }
          else if (c != '\r') {    // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
  
          // Check to see if the client request was "GET /L" or "GET /S" or "GET /F" or "GET /C" or "GET /O":
  
          // Lampadario
          if (currentLine.endsWith("GET /L")) {
            chandStatus = !chandStatus;
            digitalWrite(chandPin, chandStatus);        
          }
          // Striscia Led
          if (currentLine.endsWith("GET /S")) {
            stripStatus = !stripStatus;
            digitalWrite(stripPin, stripStatus);           
          }
          
          // Faretto
          if (currentLine.endsWith("GET /F")) {
            spotStatus = !spotStatus;
            digitalWrite(spotPin, spotStatus);            
          }
  
          // Comodino
          if (currentLine.endsWith("GET /C")) {
            socketStatus = !socketStatus;
            digitalWrite(socketPin, socketStatus);
          }
  
          // Spegne Tutto
          if (currentLine.endsWith("GET /O")) {
            chandStatus = true;
            stripStatus = true;
            spotStatus = true;
            socketStatus = true;
            digitalWrite(chandPin, chandStatus);       
            digitalWrite(stripPin, stripStatus); 
            digitalWrite(spotPin, spotStatus); 
            digitalWrite(socketPin, socketStatus);   
          }     
        }
      }
      // close the connection:
      client.stop();
     // Serial.println("client disonnected");
    }  
  }

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void printTime()
{
  print2digits(rtc.getHours() + GMT);
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

void printDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());

  Serial.print(" ");
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

bool checkTimeForWifiReset()
{
  int currentMillis = millis();
  if((currentMillis - millisCount) >= 1000 || (currentMillis - millisCount) <= 1000 )
  {
    secondsCount++;
  }
  millisCount = currentMillis;
  if(secondsCount == 60)
  {
    secondsCount = 0;
    minutesCount++;
  }
  if(minutesCount == 60)
  {
    minutesCount = 0;
    hoursCount++;
  }  
  if(hoursCount == timeToReset)
  {
    secondsCount = 0;
    minutesCount = 0;
    hoursCount = 0;
    return true;
  }
  else
  {
    return false;
  }
}

