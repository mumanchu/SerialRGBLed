/////////////////////////////////////////////////////////////////////
// Example Sketch for SerialRGBLedRMT on ESP32
// This uses the ESP32's on-chip Remote Control Transmitter (RMT)
// mumanchu & muman.ch, 2026.05.14
// https://github.com/mumanchu/SerialRGBLed
// https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm

#include "SerialRGBLedRMT.h"
SerialRGBLed leds;

// 24-led ring, as shown in the blog
#define NLEDS 24

void setup() 
{
	Serial.begin(115200);
	delay(3000);
	Serial.println("\n\rStarting...\n\r");
	Serial.flush();

	pinMode(LED_BUILTIN, OUTPUT);

	if (!leds.begin(D0, NLEDS, true)) {
		Serial.println("led.begin() failed");
		Serial.flush();
		while (1) yield();
	}
}

void loop() 
{
	// 100ms scheduler
	static ulong t1 = 0;
	ulong t = millis();
	if ((t - t1) >= 100) {
		t1 = t;

		// flash the onboard LED
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

		// three rotating colours on a 24-led circular matrix
		static int nled0 = 0;
		static int nled1 = NLEDS / 4;
		static int nled2 = NLEDS / 2;
		static ulong color0 = 0x000f0000;
		static ulong color1 = 0x00000f00;
		static ulong color2 = 0x0000000f;

		for (int i = 0; i < NLEDS; ++i) {
			ulong color = 0;
			if (i == nled0)
				color = color0;
			else if (i == nled1)
				color = color1;
			else if (i == nled2)
				color = color2;
			leds.setLedColor(i, color);
		}
		if (++nled0 == NLEDS)
			nled0 = 0;
		if (--nled1 < 0)
			nled1 = NLEDS - 1;
		if (++nled2 == NLEDS)
			nled2 = 0;
		
		leds.updateLeds();
	}
}
