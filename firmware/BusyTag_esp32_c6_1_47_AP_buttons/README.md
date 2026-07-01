# BusyTag — ESP32-C6 Arduino sketch

This branch adds an Arduino example sketch to run a small BusyTag on an ESP32-C6 with a 1.47" (172x320) ST7789 display.

What is included
- firmware/BusyTag_esp32_c6_1_47_AP_buttons/BusyTag_esp32_c6_1_47_AP_buttons.ino — main Arduino sketch
- firmware/BusyTag_esp32_c6_1_47_AP_buttons/User_Setup_1_47_ST7789.h — example TFT_eSPI configuration for ST7789 (adjust pins)

Features
- Starts a WPA2 soft AP and shows SSID/password/IP on the display
- Simple web page to upload a JPEG; uploaded image is buffered in RAM and immediately displayed
- Two buttons (default pins 35 and 34) show the AP info page for 15 seconds when pressed

Libraries required
- TFT_eSPI by Bodmer — configure User_Setup.h for your panel (you can copy User_Setup_1_47_ST7789.h into TFT_eSPI configuration)
- TJpg_Decoder — for fast JPEG rendering onto TFT_eSPI

Usage notes
1. Open the sketch in Arduino IDE or PlatformIO. Select the ESP32-C6 board and core.
2. Install the required libraries.
3. Ensure TFT_eSPI is configured for your wiring. You can adapt the included User_Setup_1_47_ST7789.h or use the board's existing configuration.
4. Adjust button pin defines in the sketch if your board uses different pins.
5. Upload sketch. The device will create an AP named BusyTag-XXXX. Connect to it and open http://192.168.4.1 to upload a JPEG.

GIF/WebP support
- This sketch only decodes JPEG. To support GIF (animated) or WebP, consider one of:
  - Client-side conversion: convert GIF/WebP to JPEG in the browser before upload (using canvas.toDataURL('image/jpeg')) — easiest and recommended.
  - Integrate an animated-GIF decoder on the ESP (e.g., AnimatedGIF library) — requires additional memory and coding for frame timing.
  - Port libwebp for WebP decoding — heavy and not recommended for low-RAM devices.

If you want, I can:
- Move the TFT_eSPI config into the repo's proper config location or a library submodule
- Replace RAM buffering with SPIFFS/LittleFS temporary storage
- Add client-side JS to convert GIF/WebP to JPEG automatically in the upload page

Branch
Files were pushed to branch: feature/busytag-esp32-c6

