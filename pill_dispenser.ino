//Iot Pill Dispenser
//Thanks to developers of included libraries
#include "RTClib.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128   //I used 128 X 32 Oled
#define SCREEN_HEIGHT 32   //Replace with 64 if you are using 128 X 64 oled

#define OLED_RESET     -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   32    
#define LOGO_WIDTH    70

RTC_DS3231 rtc;
Servo s1;
Servo s2;
Servo s3;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int hour_1 = 12;
int min_1 = 53;

const int buzzer = D6;
const int button = D7;

unsigned long previous;
unsigned long current;

int state;
int alarm = 0;

const char* ssid = "your ssid nme";  
const char* password = "password";
const char* address1 = "http://maker.ifttt.com/trigger/EVENT_NAME/with/key/KEY"; //Replace EVENT_NAME with applet's event and KEY with your key
const char* address2 = "http://maker.ifttt.com/trigger/EVENT_NAME/with/key/KEY";

void ifttt(int);
void drawbitmap();

static const unsigned char PROGMEM logo_bmp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 
  0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x01, 0xff, 0xff, 
  0xff, 0x10, 0x00, 0x1c, 0x00, 0x00, 0x01, 0xff, 0xff, 0xfe, 0x1e, 0x00, 0x38, 0x00, 0x00, 0x01, 
  0xff, 0xff, 0xc0, 0x00, 0xf0, 0x30, 0x00, 0x00, 0x01, 0xff, 0xff, 0xe0, 0x00, 0xf0, 0x60, 0x00, 
  0x00, 0x01, 0xff, 0xff, 0xfc, 0x0f, 0x00, 0x60, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x1e, 0x00, 
  0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x1e, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 
  0x3e, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xbf, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 
  0xff, 0xff, 0xbf, 0x00, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xbf, 0x00, 0xc0, 0x00, 0x00, 
  0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x60, 
  0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x70, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfc, 
  0x00, 0x30, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x38, 0x00, 0x00, 0x01, 0xff, 0xff, 
  0xff, 0xf8, 0x00, 0x1e, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x0f, 0x80, 0x00, 0x01, 
  0xff, 0xff, 0xff, 0xe0, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x7f, 
  0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup () {

  pinMode(buzzer, OUTPUT);
  pinMode(button,INPUT);

  //WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_OFF);  //Otherwise lot of power is consumed if esp is used in ap mode
  
  #ifndef ESP8266
  while (!Serial);
  #endif

  Serial.begin(9600);

  delay(3000);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");   //if ds3231 lost power then set rtc clock to compilation time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  drawbitmap();
  
  display.clearDisplay();

  display.setTextSize(1); 
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);      
  display.setCursor(0,0);             
  display.println(F("   Pristine Pill     "));

  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);               
  display.println(F("      Dispenser      "));

  display.setTextSize(1);            
  display.setTextColor(SSD1306_WHITE);
  display.println(F("Made By:"));
  
  display.setTextSize(1);            
  display.setTextColor(SSD1306_WHITE);
  display.println(F("    Gursimran Singh"));

  display.display();
  
  delay(4000);
}

void loop () {
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);               
    display.print(now.year(), DEC);
    display.print('/');
    display.print(now.month(), DEC);
    display.print('/');
    display.print(now.day(), DEC);
    display.print("   ");
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);
    
    display.println();

    display.setTextSize(2); 
    display.setTextColor(SSD1306_WHITE);
    display.print(" ");                
    display.print(now.hour(), DEC);
    display.print(':');
    display.print(now.minute(), DEC);
    display.print(':');
    display.print(now.second(), DEC);

    display.println();

    display.setTextSize(1);            
    display.setTextColor(SSD1306_WHITE);
    display.print("Temperature: ");
    display.print(rtc.getTemperature());
    display.println(" C");

    display.display();

    state = digitalRead(button);

    if(state == HIGH)   //button is pressed for sometime
    {
      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,1);               
      display.println("  Dispensing.....");

      display.display();

      s3.attach(D5);     //This operates the servos to dispense a pill
      s3.write(0);

      s1.attach(D3);
          
      s1.write(80);
      delay(1000);
      s1.write(0);
      delay(1000);
      s1.write(80);

      s1.detach();

      s2.attach(D8);

      s2.write(10);
      delay(1000);
      s2.write(90);
      delay(1000);
      s2.write(10);

      s2.detach();
      
      s3.write(90);
      delay(1000);
      s3.write(0);

      s3.detach();
    

      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,1);               
      display.println("   Pill dispensed!!");
          
      display.display();
          
      ifttt(1);   //ifttt-kamm hogya
      Serial.println("Pill taken!");

      digitalWrite(buzzer,HIGH);
      delay(500);
      digitalWrite(buzzer,LOW);
    }

    if(hour_1 == now.hour() && min_1 == now.minute() && (now.second() == 0 || now.second() == 1))
    {
      alarm = 1;
      previous =  millis();
      current = previous;
      
      while(current - previous < 60000)   //keep buzzinf for 60 seconds unless button is pressed
      {
        current = millis();
        state = digitalRead(button);
        digitalWrite(buzzer,HIGH);

        display.clearDisplay();

        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,1);               
        display.println("Take Pill!");

        display.display();
        
        if(state == 1)    //button pressed
        {
          digitalWrite(buzzer,LOW);   //turn off the alarm
          
          display.clearDisplay();

          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0,1);               
          display.println("  Dispensing.....");

          display.display();

          s3.attach(D5);
      s3.write(0);

      s1.attach(D3);
          
      s1.write(80);
      delay(1000);
      s1.write(0);
      delay(1000);
      s1.write(80);

      s1.detach();

      s2.attach(D8);

      s2.write(10);
      delay(1000);
      s2.write(90);
      delay(1000);
      s2.write(10);

      s2.detach();
      
      s3.write(90);
      delay(1000);
      s3.write(0);

      s3.detach();
    

          display.clearDisplay();

          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(0,1);               
          display.println("   Pill dispensed!!");
          
          display.display();
          
          ifttt(1);   //sent email indicating pill was dispensed
          Serial.println("Pill taken!");
          alarm = 0;

          digitalWrite(buzzer,HIGH);
          delay(500);
          digitalWrite(buzzer,LOW);
          
          break;
        }
        else
        {
          Serial.println("Time to take your pill!");
        }
      }

      if(alarm == 1)  //if 60s passed and button wasn't pressed
      {
       ifttt(2);     //

       digitalWrite(buzzer,LOW);

       display.clearDisplay();

       display.setTextSize(1);
       display.setTextColor(SSD1306_WHITE);
       display.setCursor(0,1);               
       display.println("Pill wasn't dispensed!");

       display.display();

       delay(1000);

       display.clearDisplay();

       display.setTextSize(1);
       display.setTextColor(SSD1306_WHITE);
       display.setCursor(0,1);               
       display.println("Notification Sent!");
          
       display.display();
       
      }
    }

    Serial.println(); 

    delay(1000);
}

void ifttt(int opt)
{  
  WiFi.begin(ssid, password);   //connect to sssid

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("!!");
  }
  
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      if(opt == 1)
      {
        http.begin(address1);
      }
      else if(opt == 2)
      {
        http.begin(address2);
      }
      
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "value1=" + String(20) + "&value2=" + String(20)+ "&value3=" + String(20); // build the string         
      int httpResponseCode = http.POST(httpRequestData);
      
      http.end();
    }

 WiFi.disconnect(WIFI_OFF);   //again turn esp off otherwise a lot of power is wasted
 
}

void drawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(2000);
}
