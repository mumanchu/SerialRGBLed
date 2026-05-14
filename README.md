# SerialRGBLed Library for WS2811 and WS2812 RGB LEDs

Another library from _mumanchu_.

## *** PRELIMINARY ***

_If it doesn't work, please let me know..._

## Description

This library is for WS28xx chips and RGB LEDs. It has an advantage over most other WS28xx libraries in that it is very small (~260 lines, including the comments), making it easy to understand and modify. In comparison, the official Adafruit library is over 4000 lines of code.

However, it works only on fast STM32 MCUs, 64MHz or faster, because the code is (mostly) in C++ it is not fast enough on old and slow MCUs. It requires delays as short as 350ns (nanoseconds).

Currently, the fast digital I/O works only on STM32s. Use the `OptimizedGPIO` library if you want to modify this code for other MCUs. (An updated multi-platform version will be available soon.)

For the fantastic ESP32 boards, there is a different include file called `SerialRGBLedESP32.h`. This contains the same `SerialRGBLed` class, but crafted for the ESP32. This has been tested on the XIAO ESP32 boards. (Note that with the ESP32, you can use `setCpuFrequencyMhz(mhz)` to change the CPU's clock frequency at run time. But you must do that _before_ calling `begin()`, and only 80/160/240MHz are supported.) 

Software delays are used for the 700ns/350ns (nanosecond) signal timing. To select the timing you must `#define MCU_FREQ_MHZ xxx` with the correct MCU frequency in MHz, e.g 64 or 164 etc.

For different MCU speeds, or other MCUs, you must adjust the NOP timing (the number of "NOP" instructions) using an oscilloscope on the data pin to view the pulse widths.

It has been tested on several 800KHz LED strips and matrices, and also on these 3D printer display boards with 3 x RGB LEDs driven by WS2811 chips:
- BIGTREETECH MINI 12864 V2.0
- MAKERBASE MKS MINI 12864 V3

Each LED or chip model has slightly different timing, but the chosen timing (700ns/350ns) should work for most 800KHz devices. For 400KHz devices, just modify the code to double the delays.

Complex color animations can use 'updateLeds(ulong* data)' with pointers to a sequence of ulong RGB (or GBR) arrays.

## Installation and Use

Copy the `SerialRGBLed.h` file into your Sketch directory and include it as shown below. Before the include file, declare the CPU's frequency with `#define MCU_FREQ_MHZ xx`. Not all frequencies have been tested (yet), so you may need to define a new one by timing the signals on the data pin with an oscilloscope. The example contains some code for this testing, which continuously outputs 0 or 1 bits so you can time the T0 and T1 periods.

```cpp
	// Define the CPU's MHz frequency here, must be >= 64MHz
	#define MCU_FREQ_MHZ 72
	#include "SerialRGBLed.h"
	SerialRGBLed leds;
```

Then call `begin()` in `setup()`:

```cpp
	// set the data pin and number of LEDs
	if (!leds.begin(LED_PIN, NLEDS)) {
		Serial.println("leds.begin() failed, pin number or MCU_FREQ_MHZ?");
		Serial.flush();
		while (1) yield();
	}
```

Control the LEDs from the `loop()` as indicated in the example.

The LED data is first prepared in memory by calling `setLedColor()`. This always takes a 24-bit RGB color (0x00rrggbb), but it is converted for GBR LEDs according to the `begin(..., bool gbr)` parameter.
Once all LED colors have been set, send the data to the LEDs with `updateLeds()`.

This is the LED ring that the example code is driving: \
![LED ring used by example code](https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/serial-rgb-led-example.jpg)

And there's a 3 second video of the effect here (but you can't watch it on github - you need to download it first): \
https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/serial-rgb-led-example.mp4


## Q. Can you drive WS2812 5V LEDs from a 3.3V MCU?

There's a lot of talk on Internut about this. The WS2812 data sheet says the minimum 'high' level is 0.7 x VDD, which is 3.5V. This implies that you cannot drive it reliably directly from a 3.3V MCU. 

The max. current for the LED's DIN is +-1 microamps, which is tiny. This means it will not damage a 3.3V GPIO, even if the MCU does not have 5V-tolerant GPIOs.

But will a 3.3V output be enough to drive the LED's DIN input which needs >= 3.5V? \
The answer to that is YES, it is OK! I have never had a problem. The data sheet shows an absolute worst-case voltage which never occurs in reality (unless it's a bad chip). I'm pretty sure they would not develop a product that could not be driven by a 3.3V MCU.

## NOTE! ESD PROTECTION

If connecting to LEDs on a front panel or NEOPIXEL LEDs in a 3D printer etc. ensure the MCU output has good ESD protection! If not, add a suitable TVS diode to GND. 

I damaged an unprotected MCU input because I was wearing a fleece. A fleece is like a wearable Van der Graaf Generator or Wimshurst machine. For the same reason, never wear silk pyjamas (or a silk negligee) while working with delicate electronic equipment.

## Data Sheets

**WS2811 Driver Chip** \
https://cdn-shop.adafruit.com/datasheets/WS2811.pdf

**WS2812 LED** \
https://cdn-shop.adafruit.com/datasheets/WS2812.pdf


# Revision History

| Date       | Version  | Details |
|:---------- |:---------|:----------- |
| 2026.05.12 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

What helps you get _into_ bed in the morning?

