# SerialRGBLed Library for WS2812 and WS2811 RGB LEDs

Another library from _mumanchu_.

See the blog entry \
https://muman.ch/muman/index.htm?muman-serial-rgb-leds.htm

![57 Varieties](https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/57-varieties.jpg)


## Library Description

This library is for WS2812 RGB LEDs and WS2811 RGB LED driver chips. It has an advantage over most other WS28xx libraries in that it is _very small_ (100 or 200 lines of code, including the comments), making it easy to understand and modify. In comparison, the official Adafruit Neopixel library is over 4000 lines of code (and it probably does way more than you need).

For the ESP32, there are two versions of this library. One uses the **ESP32's 'Remote Control Transmitter' (RMT)** so all communications is done in the background by the hardware - which is great! The other version 'bit-bangs' the communications using software delays (as does the Adafruit library). For the STM32 (no RMT) there is only the bit-bang version which runs on CPUs at 64MHz or faster. The bit-bang versions use `NOP` delays for the nanosecond timing.
 
If you are using a single LED on an ESP32 board, you can use the existing `rgbLedWrite()` method that's part of the ESP32 HAL (Hardware Application Layer) in file '..\cores\esp32\esp32-hal-rgb-led.c'. This uses the ESP32's on-chip 'Remote Control Transmitter' (RMT) to generate the signals for the LED, with method `rmtWrite(...)`. The problem is it only works for one LED. You could use `rmtWriteRepeated()` to write the same colour to multiple LEDs - which is OK for single-colour 'NEOPIXEL' style displays. But what if you want to use the RMT for something else? Or you want a fancy colour animation?

If you need a multicoloured animation you can use an array of RMT control data. Each bit sent to the LEDs needs a 32-bit `rmt_data_t` value, and each LED needs 24 bits. So each LED needs 24 * 4 = 96 bytes. This cannot be reduced because the RMT data must be prepared in memory before it is sent.

There's a nice non-blocking method called `rmtWriteAsync()` which does not wait until it's all sent, so you can poll it - this is perfect. The RMT library has this feature. ESP32s have lots of RAM, so for ESP32 applications this is the best solution if you don't need the RMT for something else.

For other MCUs, or to free up the RMT and/or use less RAM, you can use the bit-bang version, which controls the output using nanosecond delays tuned to your CPU's speed. This uses just 4 bytes per LED instead of 96. It needs a fast processor, 64MHz or faster, because slower processors can't do the nanosecond delays. The library provides bit-bang versions for ESP32 and STM32 processors.

These LEDs are very bright. Each RGB colour has a one-byte brightness level of 0..255 (0x00..0xFF). At 0xFF it's too bright to look at, 0x0F is better if it's on the desk next to you. RGB values are 24-bits, usually stored as a 32-bit unsigned integer, 0x00rrggbb (0x00000000 .. 0x00FFFFFF). Some LEDs are GBR (not RGB). In this case call `begin()` with `grb = true`. Method parameters are always in the standard RGB format.

## LED Signal Timing

These LEDs need an 800KHz digital signal which has timed pulses to indicate '0' or '1' bits. '1' is high for 700ns (nanoseconds) and low for 350ns. '0' high for 350ns and low for 700ns. These timings are in nanoseconds, so they are very short. The LEDs have a data in pin (DI) and a data out pin (DO), so they can be chained together. You can connect the first DI pin directly to an output of your 3.3V or 5V microcontroller - see note about driving it with 3.3V below.

Below is the timing for a '1' bit (T1) and a '0' bit (T0). T1H and T0L (T1 High and T0 Low) are 700..800ns. T1L and T0H (T1 Low and T0 High) are 350..400nS. The specified timings vary a bit between different versions of the chip, but 700/350ns works well, and so does 800/400ns. The RMT example uses 800/400ns. Some chips will also run at 400KHz, so the delays are doubled and maybe you can use them with slower CPUs. (That's your homework.)

The DI and DO pins of all LEDs are chained together, so the serial data is passed down the chain to each LED until there is a break in the data of at least 50us microseconds. After this break, the LEDs assume the data is ready and will display the colour values they have received.

![Timing](https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/ws2812-timing.png)


## LED 5V Power

The LEDs need 5V DC at up to 60mA for each LED (20mA per colour), depending on the brightness. That's a worst case of 20A for a 5m LED array! So you will usually need a separate 5V power supply for the LEDs. If you have only one or two LEDs then you can use the MCU's 5V supply, which may come directly from the USB or may be passed though a regulator with current limitations, so check the data sheet. The DI data pin only takes a few microamps, so you don't need to worry about that.

NEOPIXEL LEDs for 3D Printers often have an additional connector on the main board for an external 5V power supply.

5V DC at 20 Amps... The best ones have a fan. Power Factor Correction (PFC) is not needed because it's not an inductive load, so cheap PSUs can be used.

![5V PSU](https://github.com/mumanchu/mumanchu/blob/main/assets/SerialRGBLed/5v-20a-psu.jpg)

 
## Library Installation and Usage

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

The answer to that is YES, it is OK! I have never had a problem. The data sheet shows an absolute worst-case minimum switching voltage, which never occurs in reality (unless it's a duff chip). I'm pretty sure they would not develop a product that could not be driven by a 3.3V MCU.

## Tip! ESD Protection

Beware of Electrostatic Discharges. If mounting LEDs on a front panel or using NEOPIXEL LEDs with a 3D Printer etc. ensure the MCU output has good ESD protection! If not, think about adding a suitable TVS diode to GND.

While working with these LEDs, I damaged an unprotected MCU output because I was wearing a fleece. A fleece is like a wearable Van der Graaf Generator or Wimshurst machine. For the same reason, never wear silk pyjamas (or a silk negligee) while working with delicate electronic equipment.


## Data Sheets

These LEDs and driver chips are manufactured by Worldsemi. Their website always seems to download the data sheet onto your computer instead of opening it in the browser. So the Adafruit versions are referenced here.

**WS2811 Driver Chip** \
https://cdn-shop.adafruit.com/datasheets/WS2811.pdf

**WS2812 LED** \
https://cdn-shop.adafruit.com/datasheets/WS2812.pdf


# Revision History

| Date       | Version  | Details |
|:---------- |:---------|:----------- |
| 2026.05.15 | 1.0.0	| Initial release |

<br/>


## Joke of the Week

What makes you get _into_ bed in the morning?

