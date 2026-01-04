# ESP32-S3 High-Performance Retro GIF Player

An optimized, high-speed animated GIF player for ESP32-S3 and ILI9341 displays. It leverages Hardware SPI at 80MHz, PSRAM buffering, and a custom "Scanline" rendering technique to achieve smooth playback with a retro CRT aesthetic.

## Features

* **🚀 80MHz Hardware SPI:** Pushes the ILI9341 display driver to its absolute physical limits using ESP32-S3's dedicated hardware SPI bus.
* **💾 PSRAM Buffering:** Loads the entire GIF into RAM/PSRAM at startup to eliminate filesystem latency and stuttering.
* **📺 Retro Scanline Mode:** Intentionally skips odd lines during rendering. This doubles the effective frame rate and creates a stylish "Scanline" / CRT TV effect.
* **🔒 Locked 30 FPS:** Includes a frame-time limiter to prevent the animation from playing too fast and significantly reduces screen tearing.
* **✨ Smart Transparency:** Custom manual pixel mapping prevents "black box" artifacts often found in optimized GIFs.

## Hardware Requirements

* **Microcontroller:** ESP32-S3 (Boards with PSRAM / OPI PSRAM recommended for large GIFs).
* **Display:** 2.4" or 2.8" TFT LCD with ILI9341 Driver (SPI Interface).

## Software Requirements

* **Arduino IDE** (v2.0+)
* **Libraries:**
    * `Adafruit_GFX`
    * `Adafruit_ILI9341`
    * `AnimatedGIF` (by BitBank)
    * `LittleFS` (Built-in)

## Installation & Setup

1.  **Wiring:**
    * MOSI: 11
    * MISO: 13
    * SCK:  12
    * CS:   10
    * DC:   9
    * RST:  8
    * BLK:  7 (Backlight)

2.  **Arduino IDE Settings:**
    * **Board:** ESP32S3 Dev Module
    * **CPU Frequency:** 240MHz (WiFi)
    * **PSRAM:** OPI PSRAM (or QSPI depending on your board)
    * **Flash Mode:** QIO 80MHz

3.  **Upload Data:**
    * Create a folder named `data` inside your sketch folder.
    * Place your GIF file (rename it to `data.gif`) inside.
    * Use the "ESP32 Sketch Data Upload" tool to flash the filesystem.

## Customization

* **FPS Limit:** Change `#define TARGET_FPS 30` to your desired framerate.
* **Disable Scanlines:** Comment out the line `if (y & 1) return;` in the `GIFDraw` function for full resolution (will reduce FPS).

## License

MIT License.
