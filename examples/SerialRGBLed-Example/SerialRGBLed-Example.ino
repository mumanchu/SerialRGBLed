/////////////////////////////////////////////////////////////////////
// SerialRGBLed Example for STM32
// mumachu and muman.ch, 2026.06.22
// 
// The example shows three rotating colours on a 24-led circular matrix
// https://github.com/mumanchu/SerialRGBLed

// IMPORTANT!
// Define the CPU's MHz frequency here, must be >= 64MHz
// not all frequencies are supported (yet), see SerialRGBLed.h for details
// use the value of F_CPU, which may be a runtime variable (not a #define)
#define MCU_FREQ_MHZ 168

#include "C:\Users\matth\Documents\Visual Studio 2022\bigtreetech-skr-mini-e3\bigtreetech-skr-mini-e3\SerialRGBLed.h"
SerialRGBLed leds;

#define LED_PIN		PA10	// Arduino pin D2
#define NLEDS		24		// number of LEDs in the ring
//#define LED_BUILTIN	PA5		// on Arduino it's pin 13

void setup() 
{
	Serial.begin(115200);
	delay(3000);
	Serial.println("\n\rStarted...\n\r");
	Serial.flush();

	pinMode(LED_BUILTIN, OUTPUT);

	// set the data pin and number of LEDs
	if (!leds.begin(LED_PIN, NLEDS, true)) {
		Serial.println("leds.begin() failed, pin number or MCU_FREQ_MHZ?");
		Serial.flush();
		while (1) 
			yield();
	}
}

void loop()
{
	// 100ms scheduler
	static ulong t1 = 0;
	ulong t = millis();
	if ((t - t1) >= 100) {
		t1 = t;

		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

		#if 0
		// use this code to measure the timing with an oscilloscope
		for (int i = 0; i < NLEDS; ++i) {
			leds.setLedColor(i, 0x00000000);	// all 0 bits, measure T0H and T0L
			//leds.setLedColor(i, 0x00ffffff);	// all 1 bits, measure T1H and T1L
		}
		leds.updateLeds();

		#else

		// three rotating colours on a 24-led circular matrix
		static int nled0 = 0;
		static int nled1 = NLEDS / 4;
		static int nled2 = NLEDS / 2;

		// RGB 0x0f = not too bright
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

		#endif
	}
}
