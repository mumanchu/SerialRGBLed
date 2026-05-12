/////////////////////////////////////////////////////////////////////
// SerialRGBLed Example
// mumachu and muman.ch, 2026.05.12
// 
// The example shows three rotating colours on a 24-led circular matrix
// see http://github/mumanchu/SerialRGBLed

// Define the CPU's MHz frequency here, must be >= 64MHz
#define MCU_FREQ_MHZ 72

#include "C:\Users\matth\Documents\Visual Studio 2022\bigtreetech-skr-mini-e3\bigtreetech-skr-mini-e3\SerialRGBLed.h"
SerialRGBLed leds;

// Adapt the pins for your board, these are for a Nucleo-64 STM32
#define LED_PIN		PA10	// Arduino pin D2
#define NLEDS		24		// number of LEDs in the ring

#define LED_BUILTIN	PA5		// Arduino pin D13

void setup() 
{
	Serial.begin(115200);
	delay(3000);
	Serial.println("\n\rStarted...\n\r");
	Serial.flush();

	pinMode(LED_BUILTIN, OUTPUT);

	// set the data pin and number of LEDs
	if (!leds.begin(LED_PIN, NLEDS)) {
		Serial.println("leds.begin() failed, pin number or MCU_FREQ_MHZ?");
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
