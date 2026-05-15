#pragma once

/////////////////////////////////////////////////////////////////////
// RGB LED Driver for WS2811 driver chips or WS2812 LEDS
// Include file for the mumanchu SerialRGBLed library
// see github or blog
// https://github.com/mumanchu/SerialRGBLed
// https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm

#if defined(STM32_CORE_VERSION)
	// STM32 Bit-bang Version, needs to know the MCU speed in MHz
	#define MCU_FREQ_MHZ 72
	#include "SerialRGBLedSTM32.h"

#elif defined(ESP32)
	// ESP32 Versions
	// select the version you want to use
	#if 1
		// Remote Control Transmitter (RMT) version
		#include "SerialRGBLedRMT.h"
	#else 
		// Bit-bang version (assumes 240MHz)
		#include "SerialRGBLedESP32.h"
	#endif
	
#else
	#error This library is for STM32 or ESP32 only
#endif
