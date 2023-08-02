#include <Wire.h>
#include "RTClib.h"
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
RTC_DS3231 rtc;
char *res = malloc(5);
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char secchio[5];
String ora="";
String minuti="";
char invio[75];
bool agg=false;
bool ntp=false;
bool piu,meno,mess=false;

int ora_mod;

unsigned long previousMillis = 0;    // will store last time DHT was updated


const int interval = 8000;
void setup () {
 
 
 pinMode(7,OUTPUT);
 
 delay(1000);
 Serial.begin(115200);
 inputString.reserve(50);
  

  if (! rtc.begin()) {
   Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2023, 8, 2, 20, 39, 0));
  }
 
 
}

void loop () {
   
    
   // Serial.print(now.year(), DEC);
   // Serial.print('/');
   // Serial.print(pad(now.month()));
   // Serial.print('/');
   // Serial.print(now.day(), DEC);
    //Serial.print(" (");
   // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
   // Serial.print(") ");
  //  Serial.print(pad(now.hour()));
  //  Serial.print(':');
  //  Serial.print(pad(now.minute()));
  //  Serial.print(':');
   // Serial.print(now.second(), DEC);
  
    
    //delay(1000);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
            DateTime now = rtc.now();
            previousMillis = currentMillis;
            Serial.print(pad(now.hour()));
            Serial.print(':');
            Serial.print(pad(now.minute())
            );
            Serial.println('\0');
  }
  if (piu){  
       tone(7, 2000);
       delay(1000);
       noTone(7);
       piu=false;
       DateTime now = rtc.now();
       ora_mod=now.hour()+1;
       ora=String(ora_mod);
       minuti=String(now.minute());
       rtc.adjust(DateTime(now.year(),now.month(),now.day(),ora.toInt(),minuti.toInt(),now.second()));
  }
  if (meno){  
       tone(7, 2000);
       delay(1000);
       noTone(7);
       meno=false;
       DateTime now = rtc.now();
       ora_mod=now.hour()-1;
       ora=String(ora_mod);
       minuti=String(now.minute());
       rtc.adjust(DateTime(now.year(),now.month(),now.day(),ora.toInt(),minuti.toInt(),now.second()));
      }

    
   if (mess){
   tone(7, 2000);
   delay(1000);
   noTone(7);
   mess=false;
   }
   if (agg) {
    tone(7, 2000);
    delay(1000);
    noTone(7);
    agg=false;
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(),now.month(),now.day(),ora.toInt(),minuti.toInt(),now.second()));
    ora="";
    minuti="";
    Serial.flush();
   } 
  
//delay(1000);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    
    if (inChar == '\n') {
      
      if (inputString.indexOf("rtc")>-1){  
        ora.concat(inputString.charAt(0));
        ora.concat(inputString.charAt(1));
        minuti.concat(inputString.charAt(2));
        minuti.concat(inputString.charAt(3));
        agg=true;
        inputString = "";
        //Serial.println(ora+":"+minuti+"aggiornata");
      }
      if (inputString.indexOf("acceso")>-1){  
       mess=true;
       inputString = "";
      }
       if (inputString.indexOf("P")>-1){  
       piu=true;
       inputString = "";
      }
      if (inputString.indexOf("M")>-1){  
       meno=true;
       inputString = "";
      }
   }
  }
  
}
String pad(int n) {
  sprintf(res, "%02d",n);
  return String(res);
}
