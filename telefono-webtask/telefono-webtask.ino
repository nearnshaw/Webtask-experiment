


//#include <WiFiClientSecure.h>


#include <ESP8266HTTPClient.h>
//#include <b64.h>
//#include <HttpClient.h>



#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


// wifi connection variables

const char* ssid = "NICTOPIA";
const char* password = "queganasdejoder";
boolean wifiConnected = false;
const char* pcRemoteHost = "192.168.0.15";
const int pcRemotePort = 1301;
const int localPort = 1301;
const char* controllerId = "hab13tel";

IPAddress ip(192,168,0,50);
IPAddress gateway(192,168,11,1);
IPAddress subnet(255,0,0,0); 
IPAddress dns(10,0,2,200);


String baseURL = "https://webtask.it.auth0.com/api/run/wt-1fc0ea3f18fe05976be8ee4ba5c7f23f-0/hello?number=";

const char* fingerprint = "C0 5D 08 5E E1:3E E0 66 F3 79 27 1A CA 1F FC 09 24 11 61 62";   //C0:5D:08:5E:E1:3E:E0:66:F3:79:27:1A:CA:1F:FC:09:24:11:61:62


// specific variables

int in = A0;            // resistencia 230 ohms



bool tuboUp = false;
int linetone=0;
int needToPrint = 0;
int count;
int lastState = LOW;
int trueState = LOW;
long lastStateChangeTime = 0;
int cleared = 0;
String fullnum = "";

const String objectivenum = "157";
bool done = false;




// constants

const int dialHasFinishedRotatingAfterMs = 100;
const int debounceDelay = 10;
const int hangPhoneTime = 300;
const int resetTime = 2000000;



WiFiUDP UDP;
boolean udpConnected = false;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged"; // a string to send back

ESP8266WebServer server(80);   //connecto HTTP in





void setup()
{
  Serial.begin(115200);
  delay(1000);
  
  wifiConnected = connectWifi();
  // only proceed if wifi connection successful
  if(wifiConnected){
    
    server.on("/", handleRoot);
    
    server.on("/test", handleTest);
    
    server.on("/reset", handleReset);

    server.on("/manual", handleManual);


    server.onNotFound(handleNotFound);

    
    
    server.begin();
    Serial.println("HTTP server started");
    udpConnected = connectUDP();
    if (udpConnected){

      
//////// initialise pins

  
  pinMode(in, INPUT);


/////

  
    }
  }
}


// handle incomimg msg

void handleRoot() {
  digitalWrite(LED_BUILTIN, 1);
  delay(500);

/////// DESCRIPTION 


  String message = "si telefono marca bien, manda WIN, numero entero mal FAIL, UP o DOWN levanta o baja tubo. \n";
  message += controllerId;
  message += "\n\n metodos: \n";
  message += "/test /reset /manual /agua /trueno  \n\n" ;
  message += "manda a puerto: \n";
  message += pcRemotePort ;
  message += "\n recibe en puerto: \n";
  message += localPort; 
  server.send(200, "text/plain", message);

  Serial.println("root request");
///////

  Serial.println("root request");
  digitalWrite(LED_BUILTIN, 0);
}

void handleTest()
{
   server.send(200, "text/plain", controllerId);
   Serial.println("testing request");
}

void handleReset()
{ 

   server.send(200, "text/plain", "reset");
   Serial.println("reset");

////  VARIABLES TO RESET

linetone=0;
needToPrint = 0;
count = 0;
lastState = LOW;
trueState = LOW;
lastStateChangeTime = 0;
cleared = 0;
fullnum = "";
done = false;
////
   
}


void handleManual()
{
/////////////////// VARIABLES TO CHANGE MANUALLY

 fullnum = objectivenum;
  
  
/////////////////// END
  
}


//////  HANDLE OTHER CALLS





/////





void loop()
{
// check if the WiFi and UDP connections were successful
if(wifiConnected){
    server.handleClient();
    
    if(udpConnected){
      
      UDPRead();
      delay(15);


  ///////////// SPECIFIC CODE
  int analogin = analogRead(in);
  int reading = 0;


  // passar a 3 opciones
  if(analogin > 850)
  {
      reading = HIGH;    // colgado o ticks
  }
  else if (analogin > 400)
  {
      reading = 2;   //  descolgado pasivo
  }
  else
  { 
      reading = LOW;    //en medio de discar
  }


  //Serial.println(reading);
  //digitalWrite(ledPin, reading);
  
  if ((millis() - lastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being dialed, or has just finished being dialed.
    if (needToPrint) {
      // if it's only just finished being dialed, we need to send the number down the serial
      // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
      
      //Serial.println(count % 10, DEC);
      fullnum = fullnum + (count % 10);
      Serial.println(fullnum);
      needToPrint = 0;
      count = 0;
      cleared = 0;

      
      
    }
  } 



  if (reading != lastState) 
  {
    lastStateChangeTime = millis();
  }
  
  if ((millis() - lastStateChangeTime) > debounceDelay) 
  {
    // debounce - this happens once it's stablized
      if (reading != trueState) {
      // this means that the switch has either just gone from closed->open or vice versa.
      trueState = reading;
      if (trueState == HIGH) {
        // increment the count of pulses if it's gone high.
        count++; 
        needToPrint = 1; // we'll need to print this number (once the dial has finished rotating)
      } 
    }
  }




   if ((millis() - lastStateChangeTime) > hangPhoneTime) 
   {
      if (reading == HIGH && tuboUp == true)
      {
      // no dialing for a while - number resets
            fullnum = "";
            UDP.beginPacket(pcRemoteHost, pcRemotePort);
            UDP.print("DOWN");
            UDP.endPacket();
            Serial.println("hanging phone");
            tuboUp = false;
      }
      else if (reading == 2 && tuboUp == false)
      {
         UDP.beginPacket(pcRemoteHost, pcRemotePort);
         UDP.print("UP");
         UDP.endPacket();
         Serial.println("phone up");
         fullnum = "";
         tuboUp = true;
      }
      
   }  


  
  
  if ((millis() - lastStateChangeTime) > resetTime  && done == false) {
    // no dialing for a while - number resets
      if(fullnum.length() > 0)
        {
          fullnum = "";
          UDP.beginPacket(pcRemoteHost, pcRemotePort);
          UDP.print("FAIL");
          UDP.endPacket();
          Serial.println("reset number");
        }
   }  

 
  if (fullnum.length() > 3  && done == false)
  {
      UDP.beginPacket(pcRemoteHost, pcRemotePort);
      UDP.print("CALLED");
      UDP.endPacket();
      Serial.println(fullnum);
      sendNum(fullnum);
      
      UDP.beginPacket(pcRemoteHost, pcRemotePort);
      UDP.print("DOWN");
      UDP.endPacket();
       
      handleReset();
   }



  lastState = reading;









/////////////////// END
    
    }
  }
}



void sendNum(String fullnum)
{

  String fullURL = baseURL + fullnum;
  //String payload = String(""); 
  HTTPClient http;
  http.begin(fullURL, fingerprint);
  Serial.println(fullURL);
  int httpCode = http.GET();  // send request
  
  if(httpCode == HTTP_CODE_OK)
  {
     Serial.print("HTTP response code ");
     Serial.println(httpCode);
     String response = http.getString();
     Serial.println(response);
  }
  else
  {
     Serial.println("Error in HTTP request");
  }
  http.end();
  
  
}














