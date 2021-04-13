#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include "RTClib.h"
#include "Wire.h"
#include "Timezone.h"

//general for Wemos d1 mini:
//byte d0 = 16;
//byte d1 = 5;
//byte d2 = 4;
//byte d3 = 0;
//byte d4 = 2;
//byte d5 = 14;
//byte d6 = 12;
//byte d7 = 13;
//byte d8 = 15;

//Waveshare 1.54 Display
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

uint16_t w = GxEPD_WHITE;
uint16_t b = GxEPD_BLACK;

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2 No! D2-pin is used for the RTC*/ -1)); // GDEH0154D67
int16_t tbx, tby;
uint16_t tbw, tbh;
uint16_t x, y;

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

const char HelloWorld[] = "Hi,\n\nI am a low power\nNTP-clock.\n\nBest wishes!";
const char Welcome[] = "If there is no time\ndisplayed shortly:\nPlease connect\nto acces point\n\"ClockCfgAP\"\nand navigate to\n192.168.4.1\nto configure\nthe device.";
const char Connected[] = "Connected!";

//Week Days
String weekDays[7]={"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};

WiFiUDP ntpUDP;
 
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
// NTPClient timeClient(ntpUDP);
 
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
//int GTMOffset = 0; // SET TO UTC TIME
//NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", GTMOffset*60*60, 1800);
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 1800);
 
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

// RTC
RTC_DS3231 rtc;

DateTime rtc_time;

bool useNTP() {
  Serial.println(F("useNTP"));
  rtc_time = getRTCtime();
  // Readjust RTC always at night and close to potential clock change
  return (rtc_time.hour() == 2 && rtc_time.minute() == 1 && rtc_time.month() <= 3) || (rtc_time.hour() == 3 && rtc_time.minute() == 1 && rtc_time.month() >= 4);
}

void helloWorld(void)
{
  Serial.println(F("helloWorld"));
  display.fillScreen(w);
  u8g2Fonts.setCursor(0, 30);
  u8g2Fonts.print(HelloWorld);
  display.display();
  Serial.println(F("helloWorld done"));
}

void welcome(void) {
  Serial.println(F("welcome"));
  display.fillScreen(w);
  u8g2Fonts.setCursor(0, 30);
  u8g2Fonts.print(Welcome);
  display.display(true);
  Serial.println(F("welcome done"));
}

void connected(void) {
  Serial.println(F("connected"));
  display.fillScreen(w);
  u8g2Fonts.setFont(u8g2_font_helvB24_te);
  int16_t tw = u8g2Fonts.getUTF8Width(Connected); // text box width
  int16_t ta = u8g2Fonts.getFontAscent(); // positive
  int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
  int16_t th = ta - td; // text box height
  uint16_t x = (display.width() - tw) / 2;
  uint16_t y = (display.height() - th) / 2 + th;
  u8g2Fonts.setCursor(x, y);
  u8g2Fonts.print(Connected);
  display.display(true);
  Serial.println(F("connected done"));
}

void connectToWifi(void) {
  Serial.println(F("connectToWifi"));
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  //wifiManager.autoConnect("AutoConnectAP");
  wifiManager.autoConnect("ClockCfgAP");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  //Serial.println("Connected.");
}

DateTime getNTPtime(void) {
  Serial.println(F("getNTPtime"));
  timeClient.update();
 
  unsigned long localOffsetEpochTime = CE.toLocal(timeClient.getEpochTime());
  String weekDay = weekDays[timeClient.getDay()];
   
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&localOffsetEpochTime);

  byte monthDay = ptm->tm_mday;
  byte currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;
  byte currentHour = ptm->tm_hour;
  byte currentMinute = ptm->tm_min;
  byte currentSecond = ptm->tm_sec;

  Serial.print("NTP Formatted Time: ");
  Serial.println(timeClient.getFormattedTime());

  return DateTime(currentYear, currentMonth, monthDay, currentHour, currentMinute, currentSecond);
}

DateTime getRTCtime(void) {
  Serial.println(F("getRTCtime"));
  rtc_time = rtc.now();
  return rtc_time;
}

void displayUpdate(void) {
  Serial.println(F("displayUpdate"));
  display.fillScreen(w);    
  rtc_time = getRTCtime();

  String dateStr = String(rtc_time.day()) + "." + String(rtc_time.month()) + "." + String(rtc_time.year());
  char timeStr[5];
  int h = rtc_time.hour();
  sprintf(timeStr, "%d:%02d",  h, rtc_time.minute());
  String weekDay = weekDays[rtc_time.dayOfTheWeek()];

  //u8g2Fonts.setFont(u8g2_font_7Segments_26x42_mn);
  u8g2Fonts.setFont(u8g2_font_logisoso62_tn);

  int16_t tw = u8g2Fonts.getUTF8Width(timeStr); // text box width
  int16_t ta = u8g2Fonts.getFontAscent(); // positive
  int16_t td = u8g2Fonts.getFontDescent(); // negative; in mathematicians view
  int16_t th = ta - td; // text box height
  uint16_t x = (display.width() - tw) / 2;
  uint16_t y = display.height() - 0.33 * th;
  //Serial.print("bounding box    ("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", "); Serial.print(tw); Serial.print(", "); Serial.print(th); Serial.println(")");
  u8g2Fonts.setCursor(x, y);
  if (h <= 12) {  
    display.fillScreen(w);
    display.setTextColor(b);
    u8g2Fonts.setForegroundColor(b);         // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(w);         // apply Adafruit GFX color
  } else {
    display.fillScreen(b);
    display.setTextColor(w);
    u8g2Fonts.setForegroundColor(w);
    u8g2Fonts.setBackgroundColor(b);
  }
  u8g2Fonts.print(timeStr);
  u8g2Fonts.setFont(u8g2_font_helvB18_te);
  u8g2Fonts.setCursor(8, y - 140);
  u8g2Fonts.print(weekDay);
  u8g2Fonts.setFont(u8g2_font_helvB18_te);
  u8g2Fonts.setCursor(8, y - 104);
  u8g2Fonts.print(dateStr);
  display.display(); //here we are after deep sleep and a partial update is not easily possible.
  Serial.println(F("display.display() done"));  
  Serial.println(F("displayUpdate done"));    
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println(F("setup"));
  String resetReason = ESP.getResetReason();

  // Connect to RTC
  Wire.begin();  //sets up the I2C
  rtc.begin();   //initializes the I2C to the RTC

  display.init();
  u8g2Fonts.begin(display);
  display.fillScreen(w);
  u8g2Fonts.setForegroundColor(b);         // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(w);         // apply Adafruit GFX color
  u8g2Fonts.setFont(u8g2_font_helvB14_te); //select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  display.setRotation(1);
  display.setFullWindow();

  if ( resetReason == "Power On" ) {
    helloWorld();
    delay(5000);
    welcome();
    delay(5000);
  }
     
  Serial.println("resetReason: " + resetReason);
  rtc_time = getRTCtime();
  if ( (resetReason != "Deep-Sleep Wake") || useNTP() ) {
    // Connect to Wi-Fi
    connectToWifi();
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("."));
    }
    connected();
    delay(1000);
               
    // Initialize a NTPClient to get time
    timeClient.begin();
  
    // Set RTC
    rtc.adjust(getNTPtime());
    
    displayUpdate();
    Serial.println(F("setup done"));
  }
}
 
void loop() {
  Serial.println(F("loop"));  
  rtc_time = getRTCtime();

  display.setFullWindow();
  display.setRotation(1);
    
  while (rtc_time.second() != 0) {
    delay(100);
    Serial.print(F("in loop "));  
    rtc_time = getRTCtime();
  }
  Serial.println(F("second 0 reached"));
  displayUpdate();
  display.hibernate();
    
  char buf1[20];
  sprintf(buf1, "%02d:%02d %02d/%02d/%02d",  rtc_time.hour(), rtc_time.minute(), rtc_time.day(), rtc_time.month(), rtc_time.year());
  Serial.print(F("RTC Date/Time: "));
  Serial.println(buf1);

//  delay(58500);
  if (useNTP()) {
    Serial.println(F("useNTP: Deep sleep WAKE_RF_DEFAULT"));
    ESP.deepSleep(585e5, WAKE_RF_DEFAULT); //595e5 = 59500000 microseconds = 59.5 seconds   
  } else {
    Serial.println(F("!(useNTP): Deep sleep WAKE_RF_DISABLED"));
    ESP.deepSleep(585e5, WAKE_RF_DISABLED); //595e5 = 59500000 microseconds = 59.5 seconds
  }
  Serial.println(F("loop done"));  
}
