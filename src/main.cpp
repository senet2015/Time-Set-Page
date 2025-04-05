#include <arduino.h>
#include <WiFi.h>

#include <Wire.h>
#include <SPI.h>
#include <SSD1306.h>
#include <ESP32Servo.h>

#define PIN_SDA 4
#define PIN_SCL 15

SSD1306Wire display(0x3C, PIN_SDA, PIN_SCL);

unsigned long MyTestTimer = 0;                   // variables MUST be of type unsigned long
const byte    OnBoard_LED = 25;

//const char *ssid     = "WLANBuero_EXT"; 
const   char *ssid     = "hotspot7"; // "iptime_hunslab"; // "WLANBuero_EXT"; // "WLANBuero"; // "WLANBuero_EXT"; // "WLANBuero";

const char *password = "87654321";

const char* ntpServer = "pool.ntp.org"; // "time.nist.gov"; // "time.google.com"; // "time.windows.com"; // "time.apple.com";
//const char* ntpServer = "time.nist.gov"; // "time.google.com"; // "time.windows.com"; // "time.apple.com";
const long  gmtOffset_sec = 3600*9; // GMT+9 for Korea
const int   daylightOffset_sec = 0;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

Servo myServo;
int servoPin = 17;

int currentServoAngle = 0;

void testServo() {
    display.clear();
    display.drawString( 0, 0, "Servo test start");
    display.display();
    delay(500);
    for (int angle = 0; angle <= 180; angle += 30) {
        myServo.write(angle);
        delay(500);
    }
    for (int angle = 180; angle >= 0; angle -= 30) {
        myServo.write(angle);
        delay(500);
    }
}

#include <time.h>                   // time() ctime()
time_t now;                         // this is the epoch
tm myTimeInfo;                      // the structure tm holds time information in a more convient way
const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
// String timeStamp = String(myTimeInfo.tm_hour) + ":" + String(myTimeInfo.tm_min) + ":" + String(myTimeInfo.tm_sec); // string to hold the formatted time string
// String dayStamp = String(myTimeInfo.tm_year + 1900) + "-" + String(myTimeInfo.tm_mon + 1) + "-" + String(myTimeInfo.tm_mday) + daysOfWeek[myTimeInfo.tm_wday]; // string to hold the formatted time string
void showTime() {
  time(&now);                       // read the current time
  localtime_r(&now, &myTimeInfo);           // update the structure tm with the current time
  // Serial.print("year:");
  // Serial.print(myTimeInfo.tm_year + 1900);  // years since 1900
  // Serial.print("\tmonth:");
  // Serial.print(myTimeInfo.tm_mon + 1);      // January = 0 (!)
  // Serial.print("\tday:");
  // Serial.print(myTimeInfo.tm_mday);         // day of month
  // Serial.print("\thour:");
  // Serial.print(myTimeInfo.tm_hour);         // hours since midnight  0-23
  // Serial.print("\tmin:");
  // Serial.print(myTimeInfo.tm_min);          // minutes after the hour  0-59
  // Serial.print("\tsec:");
  // Serial.print(myTimeInfo.tm_sec);          // seconds after the minute  0-61*
  // Serial.print("\twday");
  // const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  // Serial.print(daysOfWeek[myTimeInfo.tm_wday]); // days since Sunday 0-6
  // if (myTimeInfo.tm_isdst == 1)             // Daylight Saving Time flag
  //   Serial.print("\tDST");
  // else
  //   Serial.print("\tstandard");
  String timeStamp = (myTimeInfo.tm_hour < 10 ? "0" : "") + String(myTimeInfo.tm_hour) + ":" +
                     (myTimeInfo.tm_min < 10 ? "0" : "") + String(myTimeInfo.tm_min) + ":" +
                     (myTimeInfo.tm_sec < 10 ? "0" : "") + String(myTimeInfo.tm_sec); // string to hold the formatted time string
  Serial.print(timeStamp); // print the formatted time string
  String dayStamp = String(myTimeInfo.tm_year + 1900) + "-" + String(myTimeInfo.tm_mon + 1) + "-" + String(myTimeInfo.tm_mday) + daysOfWeek[myTimeInfo.tm_wday]; // string to hold the formatted time string 
  Serial.print("\t");
  Serial.print(dayStamp); // print the formatted time string
  Serial.println();

  // Display time and date on OLED
  display.clear();
  display.setFont(ArialMT_Plain_16); // There are 3 font sizes(10, 16, 24) for ArialMT font.
  display.drawString(0, 0, dayStamp);
  //display.setFont(ArialMT_Plain_10); // Smaller font for date
  display.drawString(0, 15, timeStamp);
  display.display();
}


boolean TimePeriodIsOver (unsigned long &periodStartTime, unsigned long TimePeriod) {
  unsigned long currentMillis  = millis();  
  if ( currentMillis - periodStartTime >= TimePeriod )
  {
    periodStartTime = currentMillis; // set new expireTime
    return true;                // more time than TimePeriod) has elapsed since last time if-condition was true
  } 
  else return false;            // not expired
}

void BlinkHeartBeatLED(int IO_Pin, int BlinkPeriod) {
  static unsigned long MyBlinkTimer;
  pinMode(IO_Pin, OUTPUT);
  
  if ( TimePeriodIsOver(MyBlinkTimer,BlinkPeriod) ) {
    digitalWrite(IO_Pin,!digitalRead(IO_Pin) ); 
  }
}

void connectToWifi() {
  Serial.print("Connecting to "); 
  Serial.println(ssid);

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    BlinkHeartBeatLED(OnBoard_LED, 333);
    delay(332);
    Serial.print(".");
  }
  Serial.print("\n connected.");
  Serial.println(WiFi.localIP() );

}

void synchroniseWith_NTP_Time() {
  Serial.print("configTime uses ntpServer ");
  Serial.println(ntpServer);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("synchronising time");
  
  while (myTimeInfo.tm_year + 1900 < 2000 ) {
    time(&now);                       // read the current time
    localtime_r(&now, &myTimeInfo);
    BlinkHeartBeatLED(OnBoard_LED, 100);
    delay(100);
    Serial.print(".");
  }
  Serial.print("\n time synchronsized \n");
  showTime();    
}


void PrintFileNameDateTime() {
  Serial.println( F("Code running comes from file ") );
  Serial.println(__FILE__);
  Serial.print( F("  compiled ") );
  Serial.print(__DATE__);
  Serial.print( F(" ") );
  Serial.println(__TIME__);  
}

void setup() {
  //Servo initalization
  myServo.attach(servoPin);
  myServo.write(0);
  delay(20);
  Serial.begin(115200);
  //OLED initialization
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);
  delay(50);
  display.init();
  display.setFont(ArialMT_Plain_10);
  delay(50);
  display.drawString( 0, 0, "Starting up ...");
  display.drawString( 0,12, "- and initializing.");
  display.display();
  delay(1500);
  Serial.println("\n Setup-Start \n");
  PrintFileNameDateTime();
  
  connectToWifi();
  synchroniseWith_NTP_Time();
}

void loop() {
  BlinkHeartBeatLED(OnBoard_LED,100);

  if ( TimePeriodIsOver(MyTestTimer,1000) ) {
    showTime();    
  }  
}