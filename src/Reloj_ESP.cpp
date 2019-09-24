//Attribution to  Jon Fuge and his code "Wol Clock"

// -------------------- Declare Libraries --------------------
#include <NTPClient.h>
//#include <Time.h>
//#include <TimeLib.h>
#include <SmartLeds.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RtcDS3231.h>

// -------------------- Main Definitions --------------------
#define DEBUG

const int GES_PIN = 35;
const int SEC_PIN = 34;
const int LED_PIN = 14;
const int LED_COUNT = 84;
const int CHANNEL = 0;

volatile bool newSec = false;

struct TimeStr
{
	byte Hour, Minute, Second;
};

// -------------------- Editable Options --------------------

const Rgb Twelve = {64, 10, 0};						//The colour of the "12" to give visual reference to the top
const Rgb Quarters = {20, 2, 0};						//The colour of the "quarters" 3, 6 & 9 to give visual reference
const Rgb Divisions = {8, 16, 0};						//The colour of the "divisions" 1,2,4,5,7,8,10 & 11 to give visual reference
const Rgb Background = {1, 3, 10};						//All the other pixels with no information

const Rgb Hour = {64, 0, 32};							//The Hour hand
const Rgb Minute = {64, 0, 32};							//The Minute hand
const Rgb Second = {32, 0, 16};							//The Second hand

// Set brightness by time for night and day mode
const TimeStr WeekNight = {21, 30};						// Night time to go dim
const TimeStr WeekMorning = {6, 15};						//Morning time to go bright
const TimeStr WeekendNight = {21, 30};						// Night time to go dim
const TimeStr WeekendMorning = {9, 30};					//Morning time to go bright


// Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = -5;

RtcDS3231<TwoWire> Rtc(Wire);
SmartLed pixels(LED_WS2812B, LED_COUNT, LED_PIN, CHANNEL, DoubleBuffer);
WiFiUDP ntpUDP;											// By default 'time.nist.gov' is used.
NTPClient timeClient(ntpUDP);

// -------------------- WiFi Configuration --------------------
//const char *ssid = "Familia Gil Vargas";				//  your network SSID (name)
//const char *password = "dVarAlz0725*";					// your network password
const char *ssid      = "CrearmeIng_Corp";			//  your network SSID (name)
const char *password  = "Corporativo0725*";			// your network password

byte SetClock;
// -------------------- Methods --------------------
void resetPixels(){
	for (int i = 0; i < 84; i++) {
		pixels[i] = Rgb{0,0,0};
	}
	pixels.show();										// show all the pixels
}

void Draw_Clock(RtcDateTime t) // Function to draw the clock
{
	for (int i = 0; i < 84; i++) {
		if (i < 60){
			if (i == t.Second()) pixels[i] = Second;
			else if (i == t.Minute()) pixels[i] = Minute;
			else if(i == 0) pixels[i] = Twelve;
			else if(i%15 == 0) pixels[i] = Quarters;
			else if(i%5 == 0) pixels[i] = Divisions;
			else pixels[i] = Background;
		} else {
			if (i == (((t.Hour() + hours_Offset_From_GMT) % 12) * 2) + 60) pixels[i] = Hour;
			else if(i == 60) pixels[i] = Twelve;
			else if((i-60)%6 == 0) pixels[i] = Quarters;
			else if((i-60)%2 == 0) pixels[i] = Divisions;
			else pixels[i] = Background;
		}
	}
	//SetBrightness(t);									// Set the clock brightness dependant on the time
	pixels.show();										// show all the pixels
}
void SetRTCFromNtp()
{
	timeClient.update();								// get the time from the NTP server
	RtcDateTime dt = RtcDateTime(timeClient.getEpochTime() - c_Epoch32OfOriginYear);
	Rtc.SetDateTime(dt);
	dt = Rtc.GetDateTime();
	#ifdef DEBUG
		Serial.printf("RTC: %i/%i/%i, %i:%i:%i\r\n",dt.Year(),dt.Month(),dt.Day(),dt.Hour(),dt.Minute(),dt.Second());
	#endif
}

void IRAM_ATTR secIsr(){
	newSec = true;
}

// -------------------- Setup function for Wol_Clock --------------------
void setup()
{
	pinMode(SEC_PIN,INPUT);
	attachInterrupt(digitalPinToInterrupt(SEC_PIN),secIsr,RISING);
	#ifdef DEBUG
		Serial.begin(115200);
	#endif
	Rtc.Begin();
	Rtc.Enable32kHzPin(false);
	Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
	Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
	resetPixels();
	WiFi.begin(ssid, password); // Try to connect to WiFi
	while (WiFi.status() != WL_CONNECTED) delay(500);
	SetRTCFromNtp();								// get the time from the NTP server with timezone correction
}

// -------------------- Main program loop for Wol_Clock --------------------
void loop()
{
	/*time_t t = now();									// Get the current time

	Draw_Clock(t, 4);									// Draw the whole clock face with hours minutes and seconds
	if (minute(t) == 0)
	{													// at the start of each hour, update the time from the time server
		if (SetClock == 1)
		{
			SetRTCFromNtp();							// get the time from the NTP server with timezone correction
			SetClock = 0;
		}
	}
	else
	{
		delay(200);										// Just wait for 0.1 seconds
		SetClock = 1;
	}*/
	if (Rtc.IsDateTimeValid() && newSec){
		RtcDateTime dt = Rtc.GetDateTime();
		Draw_Clock(dt);
		#ifdef DEBUG
			Serial.printf("RTC: %i/%i/%i, %i:%i:%i\r\n",dt.Year(),dt.Month(),dt.Day(),dt.Hour(),dt.Minute(),dt.Second());
		#endif
		if ((dt.Hour() == 0) && (dt.Minute() == 0) && (dt.Second() == 0) && (WiFi.status() == WL_CONNECTED)) {
			SetRTCFromNtp();
		}
		newSec = false;
	}
}
