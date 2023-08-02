#include <Arduino.h>
#include <Hash.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>


String temp;
String temperatura;
String umidita;
String ora_serial,ora_mess;
bool rilevato=false;
bool mex=false;

#define DHTPIN 5

#define DHTTYPE DHT22
const long utcOffsetInSeconds = 3600;
DHT dht(DHTPIN, DHTTYPE);

//client time ntp
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, "it.pool.ntp.org" , utcOffsetInSeconds);


// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

unsigned long previousMillis = 0;    // will store last time DHT was updated


// Updates DHT readings every 10 seconds
const long interval = 10000;

#define DEBUG 0

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(x) Serial.println(x, HEX)
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(x)
#endif


#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN   14
#define DATA_PIN  13
#define CS_PIN    4

// HARDWARE SPI
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// SOFTWARE SPI
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);



uint8_t scrollSpeed = 35;    // default frame delay value
textEffect_t scrollEffect =  PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause =0; // in milliseconds
//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Global message buffers shared by Serial and Scrolling functions
#define  BUF_SIZE  75
char curMessage[BUF_SIZE] = { "" };
char newMessage[BUF_SIZE] = { "BUONGIORNO" };
bool newMessageAvailable = true;
const char* ssid     = "";
const char* password = "";
//WiFiServer server(80);
AsyncWebServer server(83);
// Variable to store the HTTP request
String header;
char ora[1];
char minuti[1];
String ore;
String minut;
const char* PARAM_INPUT_1 = "input1";

// Auxiliar variables to store the current output state
String inputMessage;
String inputParam;
String output5State = "off";
String output4State = "off";
unsigned long currentTimeS = millis();
// Previous time
unsigned long previousTimeS = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML><html><head>
  <title>Invia messaggio</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit" >
  </form><br>
  
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


void readSerial(void)
{
  static char *cp = newMessage;

  while (Serial.available())
  {
    *cp = (char)Serial.read();
    if ((*cp == '\n') || (cp - newMessage >= BUF_SIZE-2)) // end of message character or full buffer
    {
      *cp = '\0'; // end the string
      // restart the index for next filling spree and flag we have a message waiting
      cp = newMessage;
      newMessageAvailable = true;
    }
    else  // move char pointer to next position
      cp++;
  }
  
  
}

void setup()
{
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.println(WiFi.localIP());
 // server.begin();

  if (WiFi.status() == WL_CONNECTED){
          timeClient.begin();

          timeClient.update();
          ore=timeClient.getHours()+1;
          minut=timeClient.getMinutes();
                  if (ore.length()==1){
                      ora[0]='0';
                       ora[1]=ore[0];
                       //Serial.print(ora);
                  }else{ Serial.print(ore);  }
     
                   if (minut.length()==1){
                     minuti[0]='0';
                     minuti[1]=minut[0];
                      Serial.print(minuti);
                  }else{ Serial.print(minut);  }  
  }
  
  Serial.println("rtc");
  //Serial.println(timeClient.getFormattedTime());
  delay(200);
  dht.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }

    else {
      inputMessage = "N";
      inputParam = "none";
    }
    if (inputMessage=="X"){
      mex=false;
    }else{
      mex=true;
    }
    //invio rtc comando per aggiungere o togliere un ora
    if ((inputMessage=="P")||(inputMessage=="M")){
      if (inputMessage=="P"){
      Serial.println("P");
      inputMessage="";
      mex=false; 
      }
      if (inputMessage=="M"){
      Serial.println("M");
      inputMessage="";
      mex=false;
      } 
    }else{
      Serial.println("acceso");
    } 
    
    newMessageAvailable = true;
   // inputMessage.toCharArray(newMessage,75);
    
    request->send(200, "text/html", "Messaggio inviato (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Invia ancora</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  P.begin();
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
}

void loop()
{
temp="";
temperatura="";
umidita="";
// inizio controlli
if (mex==false) {
 unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
     // Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
  
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
    //  Serial.println("Failed to read from DHT sensor!");
    }
    else { 
      h = newH;
    //  Serial.println(h);
    }
    temp=String(t)+String(h);
    temperatura.concat(temp.charAt(0));
    temperatura.concat(temp.charAt(1));
    umidita.concat(temp.charAt(5));
    umidita.concat(temp.charAt(6));
    temp="Temp: "+temperatura+"     Umid: "+umidita;
    //Serial.println(temp);
    //P.println(temp);
    temp.toCharArray(newMessage,75);
    newMessageAvailable = true;
   
  }
  
}
    if (P.displayAnimate())
    {
    if (mex==true){
     ora_mess="";
     ora_mess=String(newMessage)+" "+inputMessage;
     ora_mess.toCharArray(newMessage,75);
    }
    if (newMessageAvailable)
    
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    P.displayReset(); 
    } 
    //leggo la seriale 
    
    readSerial();
    
    
}





