#pragma once

/////////////////////////////////////////////////////////////////////
// RGB LED Driver for WS2811 driver chips or WS2812 LEDS
// Copyright (C) mumanchu + muman.ch, 2026.05.12
// All rights reversed
// https://github.com/mumanchu/SerialRGBLed
// https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm
// 
// >>> BIT-BANG VERSION FOR STM32 <<<

// '#define MCU_FREQ_MHZ xxx' before '#include "SerialRGBLedSTM32.h"'
// see code below for which MCU frequencies are supported
#ifndef MCU_FREQ_MHZ
#error MCU_FREQ_MHZ not defined
#endif

// Create a single 24-bit RGB value from three separate R G B values
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
	void clearLedData();
	void setLedColor(uint led, ulong rgb);
	void updateLeds() { updateLeds(ledData); }
	void updateLeds(const ulong* data);
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
