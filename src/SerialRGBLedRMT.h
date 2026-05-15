#pragma once

/////////////////////////////////////////////////////////////////////
// RGB LED Driver for WS2811 driver chips or WS2812 LEDS
// Copyright (C) mumanchu + muman.ch, 2026.05.13
// All rights reversed
// https://github.com/mumanchu/SerialRGBLed
// https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm
// 
// >>> THIS VERSION FOR ESP32 WITH REMOTE CONTROL TRANSMITTER RMT <<<

// Create a single 24-bit RGB value from three separate R G B values
#define RGB(r, g, b) \
	(((ulong)(r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff))

#ifndef ESP32
#error This version of SerialRGBLed is only for the ESP32 with RMT
#else

class SerialRGBLed
{
	uint rmtPin;
	uint numberOfLeds;
	bool grb;					// true if GRB instead of RGB
	uint rmtSize;				// number of RMT symbols
	rmt_data_t* rmtData = NULL;

	static const rmt_data_t one;
	static const rmt_data_t zero;

public:
	bool begin(uint rmtPin, uint numberOfLeds, bool grb = false);
	void clearLedData();
	void setLedColor(uint led, ulong rgb);
	void updateLeds();
	void updateLedsAsync();
	bool ledsBusy() { return !rmtTransmitCompleted(rmtPin); }
};

// Each bit needs a 32-bit RMT value
// { duration0, level0, duration1, level1 }
const rmt_data_t SerialRGBLed::one  = { 8, 1, 4, 0 };
const rmt_data_t SerialRGBLed::zero = { 4, 1, 8, 0 };


// Call this once from setup()
bool SerialRGBLed::begin(uint rmtPin, uint numberOfLeds, bool grb)
{
	if (rmtPin >= SOC_GPIO_PIN_COUNT)
		return false;
	if (!rmtInit(rmtPin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 10000000))
		return false;

	// 24 bits per LED, rmtSize is the number of RMT symbols
	rmtSize = numberOfLeds * 24;
	rmtData = (rmt_data_t*)malloc(rmtSize * sizeof(rmt_data_t));
	if (rmtData == NULL)
		return false;

	this->rmtPin = rmtPin;
	this->numberOfLeds = numberOfLeds;
	this->grb = grb;

	clearLedData();

	return true;
}

// Clears the LED data array, all LEDs off
// it does not write the data to the display
void SerialRGBLed::clearLedData() 
{
	if (rmtData) {
		// set all the RMT data to zero bits
		rmt_data_t* p = rmtData;
		for (uint i = 0; i < rmtSize; ++i)
			*p++ = zero;
	}
}

// Set the color for an led (0 .. numberOfLeds-1)
// ulong rgb : 24-bit standard RGB value, 0x00rrggbb
// Call updateLeds() to write the data to the LEDs
// Note: 'rgb' values are converted to 'gbr' according to the 
// begin(, bool grb) parameter
void SerialRGBLed::setLedColor(uint led, ulong rgb)
{
	if (rmtData == NULL || led >= numberOfLeds)
		return;

	// if the led is grb not rgb, then swap the red and green bytes
	if (grb)
		rgb = ((rgb >> 8) & 0xff00) + ((rgb << 8) & 0xff0000) + (rgb & 0xff);
	
	// pointer to 24 bits of rmt_data_t values
	rmt_data_t* p = rmtData + (led * 24);

	// fill 24 x rmt_data_t values, MS bit 23 first
	uint bitMask = 0x00800000;
	while (bitMask) {
		*p++ = (rgb & bitMask) ? one : zero;
		bitMask >>= 1;
	}
}

// Sends all data and returns once it has been sent (blocking)
// DO NOT CALL AGAIN FOR AT LEAST 100 MICROSECONDS
void SerialRGBLed::updateLeds()
{
	if (rmtData)
		rmtWrite(rmtPin, rmtData, rmtSize, RMT_WAIT_FOR_EVER);
}

// Starts sending data and returns immediately (non blocking)
// poll with ledsBusy() until it returns false
// DO NOT CALL AGAIN FOR AT LEAST 100 MICROSECONDS
void SerialRGBLed::updateLedsAsync()
{
	if (rmtData)
		rmtWriteAsync(rmtPin, rmtData, rmtSize);
}

#endif	// #ifndef ESP32

