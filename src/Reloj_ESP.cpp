//************* Wol Clock by Jon Fuge ******************************

//************* Declare included libraries ******************************
#include <NTPClient.h>
//#include <Time.h>
//#include <TimeLib.h>
#include <SmartLeds.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define DEBUG;

const int SEC_PIN = 34;
const int LED_PIN = 14;
const int LED_COUNT = 84;
const int CHANNEL = 0;

volatile bool newSec = false;

// -------------------- Declare structures --------------------

//Create structure for time information
struct TimeStr
{
	byte Hour, Minute;
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
const TimeStr WeekNight = {21, 30};						// Night time to go dim
const TimeStr WeekMorning = {6, 15};						//Morning time to go bright
const TimeStr WeekendNight = {21, 30};						// Night time to go dim
const TimeStr WeekendMorning = {9, 30};					//Morning time to go bright

const int day_brightness = 128;
const int night_brightness = 16;

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
/*void SetBrightness(time_t t)						// Function to set the clock brightness
{
	int NowHour = hour(t);
	int NowMinute = minute(t);

	if ((weekday() >= 2) && (weekday() <= 6))
		if ((NowHour > WeekNight.Hour) || ((NowHour == WeekNight.Hour) && (NowMinute >= WeekNight.Minute)) || ((NowHour == WeekMorning.Hour) && (NowMinute <= WeekMorning.Minute)) || (NowHour < WeekMorning.Hour))
			//pixels.setBrightness(night_brightness);
		else
			//pixels.setBrightness(day_brightness);
	else if ((NowHour > WeekendNight.Hour) || ((NowHour == WeekendNight.Hour) && (NowMinute >= WeekendNight.Minute)) || ((NowHour == WeekendMorning.Hour) && (NowMinute <= WeekendMorning.Minute)) || (NowHour < WeekendMorning.Hour))
		//.setBrightness(night_brightness);
	else
		//pixels.setBrightness(day_brightness);
}*/

/*void Draw_Clock(time_t t, byte Phase) // Function to draw the clock
{
	for (int i = 0; i < 84; i++) {
		if (i < 60){
			if (i == second(t)) pixels[i] = Second;
			else if (i == minute(t)) pixels[i] = Minute;
			else if(i == 0) pixels[i] = Twelve;
			else if(i%15 == 0) pixels[i] = Quarters;
			else if(i%5 == 0) pixels[i] = Divisions;
			else pixels[i] = Background;
		} else {
			if (i == ((hour(t) % 12) * 2) + 60) pixels[i] = Hour;
			else if(i == 60) pixels[i] = Twelve;
			else pixels[i] = Background;
		}
	}

	//SetBrightness(t);									// Set the clock brightness dependant on the time
	pixels.show();										// show all the pixels
}*/
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
		
		#ifdef DEBUG
			Serial.printf("RTC: %i/%i/%i, %i:%i:%i\r\n",dt.Year(),dt.Month(),dt.Day(),dt.Hour(),dt.Minute(),dt.Second());
		#endif
		newSec = false;
	}
}
