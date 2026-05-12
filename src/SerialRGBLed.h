#pragma once

/////////////////////////////////////////////////////////////////////
// RGB LED Driver for WS2811 driver chips or WS2812 LEDS
// Copyright (C) mumanchu + muman.ch, 2026.05.12
// All rights reversed
// see https://github.com/mumanchu/SerialRGBLed
/*
BLURB
=====
This library has an advantage over most other WS28xx libraries in 
that it is very small (~230 lines, including the comments), making 
it easy to understand and modify. In comparison, the official 
Adafruit library is over 4000 lines of code.

However, it works only on fast STM32 MCUs, 64MHz or faster, because 
the code is in C++, not assembly language. C++ is not fast enough 
on old and slow MCUs.

The fast digital I/O works only on STM32s. Use the OptimizedGPIO
library if you want to modify this code for other MCUs.

A special version is available for ESP32s, which uses the ESP32's 
RMT (Remote Control Transceiver) feature, so it's all done by the
hardware and no software delays are needed.

Software delays are used for the 700us/350us signal timing. 
To select the timing you must `#define MCU_FREQ_MHZ xxx` with the
corrcet MCU frequncy in MHs, e.g 64 or 164.

For different MCU speeds you must adjust the nop timing (the number
of "nop" instructions) using an oscilloscope to view the pulse widths.

It has been tested on several 800KHz LED strips and matrices, and 
also on these 3D printer display boards with 3 x RGB LEDs driven 
by WS2811 chips:
- BIGTREETECH MINI 12864 V2.0
- MAKERBASE MKS MINI 12864 V3

Each LED or chip model has slightly different timing, but the 
chosen timing (700ns/350ns) should work for most 800KHz devices.
For 400KHz devices, just modify the code to double the delays.

Complex color animations can use 'updateLeds(ulong* data)' with 
pointers to a sequence of ulong RGB (or GBR) arrays.


Q. CAN YOU DRIVE WS2812 5V LEDs FROM A 3.3V MCU?
================================================
There's a lot of talk on Internut about this. The WS2812 data sheet 
says the minimum 'high' level is 0.7 x VDD, which is 3.5V. This 
implies that you cannot drive it reliably directly from a 3.3V MCU. 

The max. current for the LED's DIN is +-1 microamps, which is tiny. 
This means it will not damage a 3.3V GPIO, even if the MCU does not 
have 5V-tolerant GPIOs.

But will a 3.3V output be enough to drive the LED's DIN input which 
needs >= 3.5V? The answer to that is YES, it is OK! I have never had 
a problem. The data sheet shows an absolute worst-case voltage which 
never occurs in reality (unless it's a bad chip). I'm pretty sure 
they would not develop a product that could not be driven by a 3.3V
MCU.

NOTE! ESD PROTECTION
====================
If connecting to LEDs on a front panel or NEOPIXEL LEDs in a 3D 
printer etc. ensure the MCU output has good ESD protection! 
If not, add a suitable TVS diode to GND. I damaged an unprotected 
MCU input because I was wearing a fleece. A fleece is a wearable 
Van der Graaf Generator. (For the same reason, never wear silk 
pyjamas while working with delicate electronic equipment :-)


DATA SHEETS
===========

WS2811 DRIVER CHIP
https://cdn-shop.adafruit.com/datasheets/WS2811.pdf

WS2812 LED
https://cdn-shop.adafruit.com/datasheets/WS2812.pdf

WS2812B LED
https://www.mouser.com/pdfDocs/WS2812B-2020_V10_EN_181106150240761.pdf

ESD Protection
https://www.st.com/resource/en/application_note/an5612-esd-protection-of-stm32-mcus-and-mpus-stmicroelectronics.pdf

*/

// '#define MCU_FREQ_MHZ' before '#include "SerialRGBLed.h"'
// see below for which MCU frequencies are supported
#ifndef MCU_FREQ_MHZ
#error MCU_FREQ_MHZ not defined
#endif

#define RGB(r, g, b) \
	(((ulong)(r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff))

#ifndef STM32_CORE_VERSION
#error This version of SerialRGBLed is only for the STM32
#else

class SerialRGBLed
{
	ulong* ledData = NULL;
	uint numberOfLeds;
	GPIO_TypeDef* port;
	uint bitMask;
	bool grb;
public:
	bool begin(uint dataPin, uint numberOfLeds, bool grb = false);
	void setLedColor(uint led, ulong rgb);
	void updateLeds() { updateLeds(ledData); }
	void updateLeds(const ulong* data);
	void clearLedData();
};

// Call this once from setup()
bool SerialRGBLed::begin(uint dataPin, uint numberOfLeds, bool grb)
{
	port = digitalPinToPort(dataPin);
	if (port == NULL)
		return false;
	bitMask = digitalPinToBitMask(dataPin);
	pinMode(dataPin, OUTPUT);
	digitalWrite(dataPin, 0);

	uint size = numberOfLeds * sizeof(ulong);
	ledData = (ulong*)malloc(size);
	if (ledData == NULL)
		return false;
	clearLedData();

	this->numberOfLeds = numberOfLeds;
	this->grb = grb;
	return true;
}

// Clears the LED data array, all LEDs off
// it does not write the data to the display
void SerialRGBLed::clearLedData() 
{ 
	if (ledData) 
		memset(ledData, 0, numberOfLeds * sizeof(ulong)); 
}

// Set the color for an led (0 .. numberOfLeds-1)
// ulong rgb : 24-bit standard RGB value, 0x00rrggbb
// Call updateLeds() to write the data to the LEDs
// Note: 'rgb' values are converted to 'gbr' according to the 
// begin(, bool grb) parameter
void SerialRGBLed::setLedColor(uint led, ulong rgb)
{
	if (led >= numberOfLeds)
		return;

	// if the led is grb not rgb, then swap the red and green bytes
	if (grb) {
		rgb = ((rgb >> 8) & 0x0000ff00) + 
			((rgb << 8) & 0x00ff0000) + 
			(rgb & 0x000000ff);
	}
	ledData[led] = rgb;
}

// Signal Timing
// This defines the approximate timing used for 800kHz chips and LEDs:
//    0 bit = 350ns high (T0H) + 700ns low (T0L)
//    1 bit = 700ns high (T1H) + 350ns low (T1L)
//    total = 1250ns, for 800kHz chip

// Software 'nop' delays are used because the timing for a 'nop' 
// instruction is reliable, depending only on the MCU clock speed.
#define NOP10	"nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
#define NOP5	"nop; nop; nop; nop; nop; "
#define NOP2	"nop; nop; "

// Define the number of NOP delays according to the MCU speed.
// There is no #define for the MCU speed because it can be set
// at run time by the clock configuration on many modern MCUs.
// The timing is not linear according to the MCU speed, so you 
// must to use a 'scope to measure it and adjust the NOP count
// in steps of 10 or 5 NOPs.

#if (MCU_FREQ_MHZ == 64 || MCU_FREQ_MHZ == 72)
// 64Mhz/72MHz STM32
#define T0H		NOP10
#define T0L		NOP10 NOP10 
#define T1H		NOP10 NOP10 NOP2
#define T1L		""

//TODO add more MCU speeds here

#elif (MCU_FREQ_MHZ == 168)
// 168MHz STM32
#define T0H		NOP10 NOP10 NOP10 NOP10 NOP10 NOP5
#define T0L		NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP5
#define T1H		NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10 NOP10
#define T1L		NOP10 NOP10 NOP10 NOP10 NOP10 

#else
#error Undefined MCU_FREQ_MHZ value
#endif


// Send data to all LEDs at 800KHz
// this takes a bit less than 30uS per LED
// NOTE: Leave at least a 300us between calls to give time for 
// the 'reset' frame.
// ensure default optimization -Os for the software timing
__attribute__((optimize("-Os"))) 
void SerialRGBLed::updateLeds(const ulong* data)
{
	if (ledData == NULL)
		return;

	// disable interrupts, interrupts mess up the software timing
	noInterrupts();

	// for fast digital outputs
	ulong odr = port->ODR;
	ulong odr0 = odr & ~bitMask;
	ulong odr1 = odr | bitMask;

	// send data to all LEDs
	// led data is 24 bits per led : 0x00rrggbb 
	// (or may be 0x00ggrrbb for other hardware)
	for (uint led = 0; led < numberOfLeds; ++led) {
		ulong color = data[led];

		// send MS bit first
		ulong mask = 1UL << 23;

		// send 24 bits
		while(mask) {
			// output high
			port->ODR = odr1;
			// 0 or 1 bit?
			bool b = (color & mask) == 0;
			// delay according to the number of NOPs
			if (b)
				__asm volatile (T0H);
			else
				__asm volatile (T1H);

			// output low
			port->ODR = odr0;
			// delay according to the number of NOPs
			if (b)
				__asm volatile (T0L);
			else
				__asm volatile (T1L);

			// next bit
			mask >>= 1;
		}
	}
	interrupts();
}

#endif	// #ifndef STM32_CORE_VERSION
