// ----- LEDS ---------
// Uncomment to enable leds
#define INCLUDE_WS2812B 
// How many leds
#define WS2812B_RGBLEDCOUNT 12
// Data pin
#define WS2812B_DATAPIN 6
// 0 left to right, 1 right to left
#define WS2812B_RIGHTTOLEFT 0



#include <Adafruit_NeoPixel.h>

void WriteToComputer() {
	while (Serial1.available()) {
		char c = (char)Serial1.read();
		Serial.write(c);
	}
}

#ifdef INCLUDE_WS2812B

Adafruit_NeoPixel WS2812B_strip = Adafruit_NeoPixel(WS2812B_RGBLEDCOUNT, WS2812B_DATAPIN, NEO_GRB + NEO_KHZ800);
bool LedsDisabled = false;
byte r, g, b;
void ReadLeds() {
	while (!Serial.available()) {}
	WS2812B_strip.setBrightness(Serial.read());

	for (int i = 0; i < WS2812B_RGBLEDCOUNT; i++) {
		while (!Serial.available()) {}
		r = Serial.read();
		while (!Serial.available()) {}
		g = Serial.read();
		while (!Serial.available()) {}
		b = Serial.read();
		if (WS2812B_RIGHTTOLEFT > 0)
			WS2812B_strip.setPixelColor(WS2812B_RGBLEDCOUNT - 1 - i, r, g, b);
		else {
			WS2812B_strip.setPixelColor(i, r, g, b);
		}
		WriteToComputer();
	}
	WS2812B_strip.show();

	for (int i = 0; i < 3; i++) {
		while (!Serial.available()) {}
		Serial.read();
	}
}

void DisableLeds() {
	LedsDisabled = true;
	Serial.write("Leds disabled");
}

#endif



static long baud = 9600;
static long newBaud = baud;

void lineCodingEvent(long baud, byte databits, byte parity, byte charFormat)
{
	newBaud = baud;
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(baud);
	Serial1.begin(baud);

#ifdef INCLUDE_WS2812B
	WS2812B_strip.begin();
	WS2812B_strip.setPixelColor(0, 0, 0, 0);
	WS2812B_strip.show();
#endif

}

int readSize = 0;
int endofOutcomingMessageCount = 0;
int messageend = 0;
String command = "";

void UpdateBaudRate() {
	// Update baudrate if required
	newBaud = Serial.baud();
	if (newBaud != baud) {
		baud = newBaud;
		Serial1.end();
		Serial1.begin(baud);
	}
}

void loop() {


	UpdateBaudRate();

	while (Serial.available()) {
		WriteToComputer();
#ifdef INCLUDE_WS2812B
		char c = (char)Serial.read();
		if (!LedsDisabled) {

			if (messageend < 6) {
				if (c == (char)0xFF) {
					messageend++;
				}
				else {
					messageend = 0;
				}
			}

			if (messageend >= 3 && c != (char)(0xff)) {
				command += c;
				while (command.length() < 5) {
					WriteToComputer();

					while (!Serial.available()) {}
					c = (char)Serial.read();
					command += c;
				}

				if (command == "sleds") {
					ReadLeds();
				}
				if (command == "dleds") {
					DisableLeds();
				}
				else {
					Serial1.print(command);
				}
				command = "";
				messageend = 0;
			}
			else {
				Serial1.write(c);
			}
		}
		else {
			Serial1.write(c);
		}
#else
		char c = (char)Serial.read();
		Serial1.write(c);
#endif

		}

	WriteToComputer();
	}
