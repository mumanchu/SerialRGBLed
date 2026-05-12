# SerialRGBLed Library for WS2811 and WS2812 RGB LEDs

## *** PRELIMINARY ***

_If it doesn't work, please let me know..._

## Blurb

This library is for WS28xx chips and RGB LEDs. It has an advantage over most other WS28xx libraries in that it is very small (~260 lines, including the comments), making it easy to understand and modify. In comparison, the official Adafruit library is over 4000 lines of code.

However, it works only on fast STM32 MCUs, 64MHz or faster, because the code is (mostly) in C++, and C++ is not fast enough on old and slow MCUs.

Currently, the fast digital I/O works only on STM32s. Use the OptimizedGPIO library if you want to modify this code for other MCUs. (An updated multi-platform version will be available soon.)

A special version will soon be available for ESP32s, which uses the ESP32's RMT (Remote Control Transceiver) feature, so it's all done by the hardware and no software delays are needed.

Software delays are used for the 700us/350us signal timing. To select the timing you must `#define MCU_FREQ_MHZ xxx` with the correct MCU freqeuncy in MHz, e.g 64 or 164.

For different MCU speeds you must adjust the NOP timing (the number of "NOP" instructions) using an oscilloscope to view the pulse widths.

It has been tested on several 800KHz LED strips and matrices, and also on these 3D printer display boards with 3 x RGB LEDs driven by WS2811 chips:
- BIGTREETECH MINI 12864 V2.0
- MAKERBASE MKS MINI 12864 V3

Each LED or chip model has slightly different timing, but the chosen timing (700ns/350ns) should work for most 800KHz devices. For 400KHz devices, just modify the code to double the delays.

Complex color animations can use 'updateLeds(ulong* data)' with pointers to a sequence of ulong RGB (or GBR) arrays.


## Q. Can you drive WS2812 5V LEDs from a 3.3V MCU?

There's a lot of talk on Internut about this. The WS2812 data sheet says the minimum 'high' level is 0.7 x VDD, which is 3.5V. This implies that you cannot drive it reliably directly from a 3.3V MCU. 

The max. current for the LED's DIN is +-1 microamps, which is tiny. This means it will not damage a 3.3V GPIO, even if the MCU does not have 5V-tolerant GPIOs.

But will a 3.3V output be enough to drive the LED's DIN input which needs >= 3.5V? \
The answer to that is YES, it is OK! I have never had a problem. The data sheet shows an absolute worst-case voltage which never occurs in reality (unless it's a bad chip). I'm pretty sure they would not develop a product that could not be driven by a 3.3V MCU.

## NOTE! ESD PROTECTION

If connecting to LEDs on a front panel or NEOPIXEL LEDs in a 3D printer etc. ensure the MCU output has good ESD protection! 

If not, add a suitable TVS diode to GND. I damaged an unprotected MCU input because I was wearing a fleece. A fleece is a wearable Van der Graaf Generator or Wimshurst machine. For the same reason, never wear silk pyjamas while working with delicate electronic equipment.

## Data Sheets

**WS2811 Driver Chip** \
https://cdn-shop.adafruit.com/datasheets/WS2811.pdf

**WS2812 LED** \
https://cdn-shop.adafruit.com/datasheets/WS2812.pdf

**WS2812B LED** \
https://www.mouser.com/pdfDocs/WS2812B-2020_V10_EN_181106150240761.pdf

**ESD Protection** \
https://www.st.com/resource/en/application_note/an5612-esd-protection-of-stm32-mcus-and-mpus-stmicroelectronics.pdf


# Revision History

| Date       | Version  | Details |
|:---------- |:---------|:----------- |
| 2026.05.11 | 0.0.0	| Preliminary |

<br/>


## Joke of the Week

What helps you get _into_ bed in the morning?

