# SerialRGBLed Library for WS2812 and WS2811 RGB LEDs

Another library from _mumanchu_.

See the blog entry \
https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm


## Description

This library is for WS2812 RGB LEDs and WS2811 RGB LED driver chips. It has an advantage over most other WS28xx libraries in that it is very small (100 or 200 lines, including the comments), making it easy to understand and modify. In comparison, the official Adafruit Neopixel library is over 4000 lines of code (but it probably does a lot more - if you need it).

For the ESP32, there are two versions of this library. One uses the **ESP32's 'Remote Control Transmitter' (RMT)** so all communications is done in the background by the hardware. The other version 'bit-bangs' the communications using software delays. For the STM32 there is only the bit-bang version, which runs on CPUs at 64MHz or faster. The bit-bang versions use `NOP` delays for the 700/350 nanosecond timing.
 
If you are using a single LED on an ESP32 board, you can use the existing `rgbLedWrite()` method that's part of the ESP32 HAL (Hardware Application Layer) in file '..\cores\esp32\esp32-hal-rgb-led.c'. This uses the ESP32's on-chip 'Remote Control Transmitter' (RMT) to generate the signals for the LED, with method `rmtWrite(...)`. The problem is it only works for one LED. You could use `rmtWriteRepeated()` to write the same colour to multiple LEDs - which is OK for single-colour 'NEOPIXEL' style displays. But what if you want to use the RMT for something else? Or you want a fancy colour animation?

If you need a multicoloured animation you can use an array of RMT control data. Each bit sent to the LEDs needs a 32-bit `rmt_data_t` value, and each LED needs 24 bits. So each LED needs 24 * 4 = 96 bytes. This cannot be reduced because the RMT data must be prepared in memory before it is sent.

There's a nice non-blocking method called `rmtWriteAsync()` which does not wait until it's all sent, so you can poll it - this is perfect. The RMT library has this feature. ESP32s have lots of RAM, so for ESP32 applications this is the best solution if you don't need the RMT for something else.

For other MCUs, or to free up the RMT and/or use less RAM, you can use the bit-bang version, which controls the output using nanosecond delays tuned to your CPU's speed. This uses just 4 bytes per LED instead of 96. It needs a fast processor, 64MHz or faster, because slower processors can't do the nanosecond delays. The library provides bit-bang versions for ESP32 and STM32 processors.

These LEDs are very bright. Each RGB colour has a one-byte brightness level of 0..255 (0x00..0xFF). At 0xFF it's too bright to look at, 0x0F is better if it's on the desk next to you. RGB values are 24-bits, usually stored as a 32-bit unsigned integer, 0x00rrggbb (0x00000000 .. 0x00FFFFFF). Some LEDs are GBR (not RGB). In this case call `begin()` with `grb = true`. Method parameters are always in the standard RGB format.
 
## Installation and Usage

The library can be installed from the Arduino IDE's Library Manager (to find it, type 'mumanchu' into the search field). Or it can be installed from the ZIP file using 'Sketch / Include Library \> Add ZIP Library...'. The ZIP file can be downloaded from github.

Take a copy of the `SerialRGBLed.h` file, add it to your sketch and `#include` it. 

Edit `SerialRGBLed.h` to select the version you want to use (RMT or bit-bang). For an STM32, specify the `MCU_FREQ_MHZ` value for the speed of your processor. Not all STM32 speeds are supported (yet), but you can modify the code in `SerialRGBLedSTM32.h` to define the delays for your processor speed. To do this you will need an oscilloscope to measure the timing. Or I could do it if I have a suitable processor - send an email to info@muman.ch. The ESP32 is fixed at 240 MHz (but you could change that, it also has timing for 80 and 160 MHz).

```cpp
	// This #includes the selected code file according to your processor type
	// On the STM32 it also defines the processor speed
	#include "SerialRGBLed.h"
	SerialRGBLed leds;
```
Next, call `begin()` from `setup()`, specifying the LED or RMT pin, and the the number of LEDs that are connected. You can also specify if it's an RGB or GBR LED. If it fails (returns `false`), it's probably an invalid pin number, the RMT failed to initialize (ESP32 only), or you don't have enough memory.

```cpp
	// in setup(), set the data pin and number of LEDs
	if (!leds.begin(LED_PIN, NLEDS)) {
		Serial.println("leds.begin() failed");
		Serial.flush();
		while (1) yield();
	}
```

Control the LEDs as shown in the example sketches. There are separate example for the RMT and the bit-bang versions. You can open an example from the Arduino IDE using 'File / Examples > Examples from custom libraries' and scroll down to find the `SerialRGBLed` entries.

The LED data is first prepared in memory by calling `setLedColor()`. This always takes a 24-bit RGB color (0x00rrggbb), but it is converted for GBR LEDs according to the `begin(..., bool gbr)` parameter.
Once all LED colors have been set, send the data to the LEDs with `updateLeds()`.

This is the LED ring that the example code is driving: \
![LED ring used by example code](https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/serial-rgb-led-example.jpg)

You can find some videos of the effect in the blog \
https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm


## Q. Can you drive WS2812 5V LEDs directly from a 3.3V MCU output?

There's a lot of talk on Internut about this. The WS2812 data sheet says the minimum 'high' level is 0.7 x VDD, which is 3.5V. This implies that you cannot drive it reliably directly from a 3.3V MCU.

The max. current into the LED's DIN pin is +-1 _microamp_, which is tiny. This means it will not damage a 3.3V GPIO, even if the MCU does not have 5V-tolerant GPIOs. **DO NOT CONNECT IT to the LED's 5V Data Out pin!**

Will a 3.3V output be enough to drive the LED's DIN input which needs >= 3.5V?

The answer to that is YES, it is OK! I have never had a problem. The data sheet shows an absolute worst-case minimum switching voltage, which never occurs in reality (unless it's duff chip). I'm pretty sure they would not develop a product that could not be driven by a 3.3V MCU.

## Tip! ESD Protection

Beware of Electrostatic Discharges. If mounting LEDs on a front panel or using NEOPIXEL LEDs with a 3D Printer etc. ensure the MCU output has good ESD protection! If not, think about adding a suitable TVS diode to GND.

While working with these LEDs, I damaged an unprotected MCU output because I was wearing a fleece. A fleece is like a wearable Van der Graaf Generator or Wimshurst machine. For the same reason, never wear silk pyjamas (or a silk negligee) while working with delicate electronic equipment.


## Data Sheets

These LEDs are manufactured by Worldsemi. Their website always seems to download the data sheet onto your computer instead of opening it in the browser. So the Adafruit versions are referenced here.

**WS2811 Driver Chip** \
https://cdn-shop.adafruit.com/datasheets/WS2811.pdf

**WS2812 LED** \
https://cdn-shop.adafruit.com/datasheets/WS2812.pdf


# Revision History

| Date       | Version  | Details |
|:---------- |:---------|:----------- |
| 2026.05.15 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

What helps you get _into_ bed in the morning?

