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
#include <SparkFun_APDS9960.h>

// -------------------- Main Definitions --------------------
#define DEBUG

const int GES_PIN = 32;
const int SEC_PIN = 34;
const int LED_PIN = 14;
const int LED_COUNT = 84;
const int CHANNEL = 0;

volatile bool newSec = false;
volatile bool newGes = false;

uint8_t proximity_data = 0;

struct TimeStr
{
	byte Hour, Minute, Second;
};

struct rgbStr
{
	uint8_t r, g, b;
};

// -------------------- Editable Options --------------------

const rgbStr Twelve = {64, 10, 0};	//The colour of the "12" to give visual reference to the top
const rgbStr Quarters = {20, 2, 0};   //The colour of the "quarters" 3, 6 & 9 to give visual reference
const rgbStr Divisions = {8, 16, 0};  //The colour of the "divisions" 1,2,4,5,7,8,10 & 11 to give visual reference
const rgbStr Background = {1, 3, 10}; //All the other pixels with no information

const rgbStr Hour = {64, 0, 32};   //The Hour hand
const rgbStr Minute = {64, 0, 32}; //The Minute hand
const rgbStr Second = {32, 0, 16}; //The Second hand
// Set brightness by time for night and day mode
const TimeStr WeekNight = {21, 30};		// Night time to go dim
const TimeStr WeekMorning = {6, 15};	//Morning time to go bright
const TimeStr WeekendNight = {21, 30};  // Night time to go dim
const TimeStr WeekendMorning = {9, 30}; //Morning time to go bright

// Set your timezone in hours difference rom GMT
const int hours_Offset_From_GMT = -5;

RtcDS3231<TwoWire> Rtc(Wire);
SmartLed pixels(LED_WS2812B, LED_COUNT, LED_PIN, CHANNEL, DoubleBuffer);
WiFiUDP ntpUDP; // By default 'time.nist.gov' is used.
NTPClient timeClient(ntpUDP);
SparkFun_APDS9960 apds = SparkFun_APDS9960();

// -------------------- WiFi Configuration --------------------
//const char *ssid = "Familia Gil Vargas";				//  your network SSID (name)
//const char *password = "dVarAlz0725*";					// your network password
const char *ssid = "CrearmeIng_Corp";	  //  your network SSID (name)
const char *password = "Corporativo0725*"; // your network password

byte SetClock;
// -------------------- Methods --------------------
void resetPixels()
{
	for (int i = 0; i < 84; i++)
	{
		pixels[i] = Rgb{0, 0, 0};
	}
	pixels.show(); // show all the pixels
}

void Draw_Clock(RtcDateTime t) // Function to draw the clock
{
	for (int i = 0; i < 84; i++)
	{
		if (i < 60)
		{
			if (i == t.Second())
				pixels[i] = Rgb{Second.r, Second.g, Second.b};
			else if (i == t.Minute())
				pixels[i] = Rgb{Minute.r, Minute.g, Minute.b};
			else if (i == 0)
				pixels[i] = Rgb{Twelve.r, Twelve.g, Twelve.b};
			else if (i % 15 == 0)
				pixels[i] = Rgb{Quarters.r, Quarters.g, Quarters.b};
			else if (i % 5 == 0)
				pixels[i] = Rgb{Divisions.r, Divisions.g, Divisions.b};
			else
				pixels[i] = Rgb{Background.r, Background.g, Background.b};
		}
		else
		{
			if (i == (((t.Hour() + hours_Offset_From_GMT) % 12) * 2) + 60)
				pixels[i] = Rgb{Hour.r, Hour.g, Hour.b};
			else if (i == 60)
				pixels[i] = Rgb{Twelve.r, Twelve.g, Twelve.b};
			else if ((i - 60) % 6 == 0)
				pixels[i] = Rgb{Quarters.r, Quarters.g, Quarters.b};
			else if ((i - 60) % 2 == 0)
				pixels[i] = Rgb{Divisions.r, Divisions.g, Divisions.b};
			else
				pixels[i] = Rgb{Background.r, Background.g, Background.b};
		}
	}
	//SetBrightness(t);									// Set the clock brightness dependant on the time
	pixels.show(); // show all the pixels
}
void SetRTCFromNtp()
{
	timeClient.update(); // get the time from the NTP server
	RtcDateTime dt = RtcDateTime(timeClient.getEpochTime() - c_Epoch32OfOriginYear);
	Rtc.SetDateTime(dt);
	dt = Rtc.GetDateTime();
#ifdef DEBUG
	Serial.printf("RTC: %i/%i/%i, %i:%i:%i\r\n", dt.Year(), dt.Month(), dt.Day(), dt.Hour() + hours_Offset_From_GMT, dt.Minute(), dt.Second());
#endif
}

void handleGesture()
{
	if (apds.isGestureAvailable())
	{
		switch (apds.readGesture())
		{
		case DIR_UP:
			Serial.println("UP");
			break;
		case DIR_DOWN:
			Serial.println("DOWN");
			break;
		case DIR_LEFT:
			Serial.println("LEFT");
			break;
		case DIR_RIGHT:
			Serial.println("RIGHT");
			break;
		case DIR_NEAR:
			Serial.println("NEAR");
			break;
		case DIR_FAR:
			Serial.println("FAR");
			break;
		default:
			Serial.println("NONE");
		}
	}
}

void IRAM_ATTR secIsr()
{
	newSec = true;
}

void IRAM_ATTR gesIsr()
{
	newGes = true;
}

// -------------------- Setup function for Wol_Clock --------------------
void setup()
{
	pinMode(SEC_PIN, INPUT);
	pinMode(GES_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(SEC_PIN), secIsr, RISING);
	attachInterrupt(digitalPinToInterrupt(GES_PIN), gesIsr, FALLING);
#ifdef DEBUG
	Serial.begin(115200);
#endif
	Rtc.Begin();
	Rtc.Enable32kHzPin(false);
	Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeClock);
	Rtc.SetSquareWavePinClockFrequency(DS3231SquareWaveClock_1Hz);
	resetPixels();
	WiFi.begin(ssid, password); // Try to connect to WiFi
	//while (WiFi.status() != WL_CONNECTED)
	//	delay(500);
	//SetRTCFromNtp(); // get the time from the NTP server with timezone correction
	// Initialize APDS-9960 (configure I2C and initial values)
	if (apds.init())
	{
		Serial.println(F("APDS-9960 initialization complete"));
	}
	else
	{
		Serial.println(F("Something went wrong during APDS-9960 init!"));
	}

	// Start running the APDS-9960 gesture sensor engine
	if (apds.enableGestureSensor(true))
	{
		Serial.println(F("Gesture sensor is now running"));
	}
	else
	{
		Serial.println(F("Something went wrong during gesture sensor init!"));
	}
	if (apds.setGestureLEDDrive(LED_DRIVE_100MA))
	{
		Serial.println(F("Current is now 100mA"));
	}
	else
	{
		Serial.println(F("Current coun't be modified"));
	}
	if (apds.setGestureGain(GGAIN_8X))
	{
		Serial.println(F("Gain is now 8X"));
	}
	else
	{
		Serial.println(F("Gain coun't be modified"));
	}
}

// -------------------- Main program loop for Wol_Clock --------------------
void loop()
{
	if (Rtc.IsDateTimeValid() && newSec)
	{
		RtcDateTime dt = Rtc.GetDateTime();
		Draw_Clock(dt);

#ifdef DEBUG
		Serial.printf("RTC: %i/%i/%i, %i:%i:%i\r\n", dt.Year(), dt.Month(), dt.Day(), dt.Hour() + hours_Offset_From_GMT, dt.Minute(), dt.Second());
#endif
		if (((dt.Hour()+ hours_Offset_From_GMT) == 0) && (dt.Minute() == 0) && (dt.Second() == 0) && (WiFi.status() == WL_CONNECTED))
		{
			SetRTCFromNtp();
		}
		newSec = false;
	}
	if (newGes)
	{
		detachInterrupt(digitalPinToInterrupt(GES_PIN));
		handleGesture();
		newGes = false;
		attachInterrupt(digitalPinToInterrupt(GES_PIN), gesIsr, FALLING);
	}
}
