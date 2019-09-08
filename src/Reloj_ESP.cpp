//************* Wol Clock by Jon Fuge ******************************

//************* Declare included libraries ******************************
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <SmartLeds.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const int LED_COUNT = 84;
const int DATA_PIN = 14;
const int CHANNEL = 0;

//************* Declare structures ******************************

//Create structure for time information
struct TIME
{
	byte Hour, Minute;
};

enum phase
{
	off,
	background,
	divisions,
	quarters,
	currTime
};

// -------------------- Editable Options --------------------

const Rgb Twelve = {80, 128, 0};						//The colour of the "12" to give visual reference to the top
const Rgb Quarters = {40, 64, 0};						//The colour of the "quarters" 3, 6 & 9 to give visual reference
const Rgb Divisions = {32, 20, 0};						//The colour of the "divisions" 1,2,4,5,7,8,10 & 11 to give visual reference
const Rgb Background = {1, 3, 10};						//All the other pixels with no information


const Rgb Hour = {64, 0, 32};							//The Hour hand
const Rgb Minute = {64, 0, 32};							//The Minute hand
const Rgb Second = {32, 0, 16};							//The Second hand


const char ClockGoBackwards = 0;						// Make clock go forwards or backwards (dependant on hardware)

// Set brightness by time for night and day mode
const TIME WeekNight = {21, 30};						// Night time to go dim
const TIME WeekMorning = {6, 15};						//Morning time to go bright
const TIME WeekendNight = {21, 30};						// Night time to go dim
const TIME WeekendMorning = {9, 30};					//Morning time to go bright

const int day_brightness = 128;
const int night_brightness = 16;

// Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = -5;

// -------------------- Configuraciones WiFi --------------------
const char *ssid = "Familia Gil Vargas";				//  your network SSID (name)
const char *password = "dVarAlz0725*";					// your network password
//const char *ssid      = "CrearmeIng_Corp";			//  your network SSID (name)
//const char *password  = "Corporativo0725*";			// your network password

byte SetClock;

// By default 'time.nist.gov' is used.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Which pin on the ESP8266 is connected to the NeoPixels?
#define PIN 14 // This is the D5 pin

// -------------------- Declare user functions --------------------
void Draw_Clock(time_t t, byte Phase);
int ClockCorrect(int Pixel);
void SetBrightness(time_t t);
void SetClockFromNTP();

// -------------------- Declare SmartLeds --------------------
SmartLed pixels(LED_WS2812B, LED_COUNT, DATA_PIN, CHANNEL, DoubleBuffer);

// -------------------- Setup function for Wol_Clock --------------------
void setup()
{
	pixels.begin();   // This initializes the NeoPixel library.
	Draw_Clock(0, 1); // Just draw a blank clock

	WiFi.begin(ssid, password); // Try to connect to WiFi
	Draw_Clock(0, 2);			// Draw the clock background

	while (WiFi.status() != WL_CONNECTED)
		delay(500); // keep waiging until we successfully connect to the WiFi

	Draw_Clock(0, 3); // Add the quater hour indicators

	SetClockFromNTP(); // get the time from the NTP server with timezone correction
}

void SetClockFromNTP()
{
	timeClient.update();								// get the time from the NTP server
	setTime(timeClient.getEpochTime());					// Set the system time from the clock
	adjustTime(hours_Offset_From_GMT * 3600);			// offset the system time with the user defined timezone (3600 seconds in an hour)
}

// -------------------- Main program loop for Wol_Clock --------------------
void loop()
{
	time_t t = now();									// Get the current time

	Draw_Clock(t, 4);									// Draw the whole clock face with hours minutes and seconds
	if (minute(t) == 0)
	{													// at the start of each hour, update the time from the time server
		if (SetClock == 1)
		{
			SetClockFromNTP();							// get the time from the NTP server with timezone correction
			SetClock = 0;
		}
	}
	else
	{
		delay(200);										// Just wait for 0.1 seconds
		SetClock = 1;
	}
}

// -------------------- Functions to draw the clock --------------------
void Draw_Clock(time_t t, byte Phase)
{
	if (Phase <= off) {
		for (int i = 0; i < 84; i++) {
			pixels[i] = Rgb{0, 0, 0};					// for Phase = 0 or less, all pixels are black
		}
	}
	if (Phase >= background) {
		for (int i = 0; i < 84; i++) {
			pixels[i] = Background;						// for Phase = 1 or more, draw minutes with Background colour
		}
	}
	if (Phase >= divisions) {
		for (int i = 0; i < 60; i = i + 5) {
			pixels[i] = Divisions;						// for Phase = 2 or more, draw 5 minute divisions
		}
	}
	if (Phase >= quarters) {
		for (int i = 0; i < 60; i = i + 15) {
			pixels[i] = Quarters;						// for Phase = 3 or more, draw 15 minute divisions
		}
		pixels[i] = Twelve;								// for Phase = 3 and above, draw 12 o'clock indicator
		for (int i = 0; i < 24; i = i + 6) {
			pixels[i] = Quarters;						// for Phase = 3 or more, draw 15 minute divisions
		}
		pixels[60] = Twelve;							// for Phase = 3 and above, draw 12 o'clock indicator
	}

	if (Phase >= currTime) {
		pixels[second(t)] = Second;						// draw the second hand first
		pixels[minute(t)] = Minute;						// draw the minute hand
		pixels[((hour(t) % 12) * 2) + 60] = Hour;		// draw the hour hand last
	}

	SetBrightness(t);									// Set the clock brightness dependant on the time
	pixels.show();										// show all the pixels
}

// -------------------- Function to set the clock brightness --------------------
/*void SetBrightness(time_t t)
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
}*/
