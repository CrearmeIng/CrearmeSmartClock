//************* Wol Clock by Jon Fuge ******************************

//************* Declare included libraries ******************************
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//************* Declare structures ******************************
//Create structure for LED RGB information
struct RGB {
  byte r, g, b;
};

//Create structure for time information
struct TIME {
  byte Hour, Minute;
};

enum phase { off, background, divisions, quarters, currTime };

//************* Editable Options ******************************
//The colour of the "12" to give visual reference to the top
const RGB Twelve = { 80, 128, 0 };
//The colour of the "quarters" 3, 6 & 9 to give visual reference
const RGB Quarters = { 40, 64, 0 };
//The colour of the "divisions" 1,2,4,5,7,8,10 & 11 to give visual reference
const RGB Divisions = { 32, 20, 0 }; //purple
//All the other pixels with no information
const RGB Background = { 1, 3, 10 };//blue

//The Hour hand
const RGB Hour = { 64, 0, 32 };
//The Minute hand
const RGB Minute = { 64, 0, 32 };
//The Second hand
const RGB Second = { 32, 0, 16 };

// Make clock go forwards or backwards (dependant on hardware)
const char ClockGoBackwards = 0;

//Set brightness by time for night and day mode
const TIME WeekNight = {21, 30}; // Night time to go dim
const TIME WeekMorning = {6, 15}; //Morning time to go bright
const TIME WeekendNight = {21, 30}; // Night time to go dim
const TIME WeekendMorning = {9, 30}; //Morning time to go bright

const int day_brightness = 128;
const int night_brightness = 16;

//Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = -5;

//Set your wifi details so the board can connect and get the time from the internet
const char *ssid      = "CrearmeIng_Corp";    //  your network SSID (name)
const char *password  = "Corporativo0725*"; // your network password

byte SetClock;

// By default 'time.nist.gov' is used.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Which pin on the ESP8266 is connected to the NeoPixels?
#define PIN            14 // This is the D5 pin

//************* Declare user functions ******************************
void Draw_Clock(time_t t, byte Phase);
int ClockCorrect(int Pixel);
void SetBrightness(time_t t);
void SetClockFromNTP ();

//************* Declare NeoPixel ******************************
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(84, PIN, NEO_GRB + NEO_KHZ800);

//************* Setup function for Wol_Clock ******************************
void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  Draw_Clock(0, 1); // Just draw a blank clock

  WiFi.begin(ssid, password); // Try to connect to WiFi
  Draw_Clock(0, 2); // Draw the clock background

  while ( WiFi.status() != WL_CONNECTED )
    delay ( 500 ); // keep waiging until we successfully connect to the WiFi

  Draw_Clock(0, 3); // Add the quater hour indicators

  SetClockFromNTP(); // get the time from the NTP server with timezone correction
}

void SetClockFromNTP ()
{
  timeClient.update(); // get the time from the NTP server
  setTime(timeClient.getEpochTime()); // Set the system yime from the clock
  adjustTime(hours_Offset_From_GMT * 3600); // offset the system time with the user defined timezone (3600 seconds in an hour)
}

        
//************* Main program loop for Wol_Clock ******************************
void loop() {
  time_t t = now(); // Get the current time

  Draw_Clock(t, 4); // Draw the whole clock face with hours minutes and seconds
  if (minute(t) == 0) { // at the start of each hour, update the time from the time server
    if (SetClock == 1)
    {
      SetClockFromNTP(); // get the time from the NTP server with timezone correction
      SetClock = 0;
    }
  }
  else
  {
    delay(200); // Just wait for 0.1 seconds
    SetClock = 1;
  }
}

//************* Functions to draw the clock ******************************
void Draw_Clock(time_t t, byte Phase)
{
  if (Phase <= off)
    for (int i = 0; i < 84; i++)
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // for Phase = 0 or less, all pixels are black

  if (Phase >= background)
    for (int i = 0; i < 84; i++)
      pixels.setPixelColor(i, pixels.Color(Background.r, Background.g, Background.b)); // for Phase = 1 or more, draw minutes with Background colour

  if (Phase >= divisions)
    for (int i = 0; i < 60; i = i + 5)
      pixels.setPixelColor(i, pixels.Color(Divisions.r, Divisions.g, Divisions.b)); // for Phase = 2 or more, draw 5 minute divisions

  if (Phase >= quarters) {
    for (int i = 0; i < 60; i = i + 15)
      pixels.setPixelColor(i, pixels.Color(Quarters.r, Quarters.g, Quarters.b)); // for Phase = 3 or more, draw 15 minute divisions
    pixels.setPixelColor(0, pixels.Color(Twelve.r, Twelve.g, Twelve.b)); // for Phase = 3 and above, draw 12 o'clock indicator
    for (int i = 0; i < 24; i = i + 6)
      pixels.setPixelColor(i + 60, pixels.Color(Quarters.r, Quarters.g, Quarters.b)); // for Phase = 3 or more, draw 15 minute divisions
    pixels.setPixelColor(60, pixels.Color(Twelve.r, Twelve.g, Twelve.b)); // for Phase = 3 and above, draw 12 o'clock indicator
  }

  if (Phase >= currTime) {
    pixels.setPixelColor(second(t), pixels.Color(Second.r, Second.g, Second.b)); // draw the second hand first
    pixels.setPixelColor(minute(t), pixels.Color(Minute.r, Minute.g, Minute.b)); // draw the minute hand
    pixels.setPixelColor(((hour(t) % 12) * 2) + 60, pixels.Color(Hour.r, Hour.g, Hour.b)); // draw the hour hand last
  }

  SetBrightness(t); // Set the clock brightness dependant on the time
  pixels.show(); // show all the pixels
}

//************* Function to set the clock brightness ******************************
void SetBrightness(time_t t)
{
  int NowHour = hour(t);
  int NowMinute = minute(t);

  if ((weekday() >= 2) && (weekday() <= 6))
    if ((NowHour > WeekNight.Hour) || ((NowHour == WeekNight.Hour) && (NowMinute >= WeekNight.Minute)) || ((NowHour == WeekMorning.Hour) && (NowMinute <= WeekMorning.Minute)) || (NowHour < WeekMorning.Hour))
      pixels.setBrightness(night_brightness);
    else
      pixels.setBrightness(day_brightness);
  else if ((NowHour > WeekendNight.Hour) || ((NowHour == WeekendNight.Hour) && (NowMinute >= WeekendNight.Minute)) || ((NowHour == WeekendMorning.Hour) && (NowMinute <= WeekendMorning.Minute)) || (NowHour < WeekendMorning.Hour))
    pixels.setBrightness(night_brightness);
  else
    pixels.setBrightness(day_brightness);
}
