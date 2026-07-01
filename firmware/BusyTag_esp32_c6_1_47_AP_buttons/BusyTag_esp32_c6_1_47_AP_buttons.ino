/*
  BusyTag for ESP32-C6 (Arduino)
  - Display: 1.47" 172x320 (ST7789 example) via TFT_eSPI
  - SoftAP (WPA2) with password
  - Web upload page for images (JPEG). Uploaded file is NOT stored persistently,
    it's buffered in RAM and immediately decoded/displayed.
  - Two buttons: on press -> show AP info page for 15 seconds, then restore.
  Requirements:
    - TFT_eSPI (configure User_Setup.h or use provided User_Setup_1_47_ST7789.h)
    - TJpg_Decoder
    - Arduino core for ESP32-C6

  Notes:
    - Default button pins are 35 and 34. Change to match your board if needed.
    - Default AP password is "busytag123" — change before production.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include <vector>

TFT_eSPI tft = TFT_eSPI();
WebServer server(80);

// Settings (adjust pins/password as needed)
const char *apPassword = "busytag123";
String apSSID;
const unsigned long HOME_SCREEN_TIMEOUT = 15000; // ms, 15s

// Buttons — change to the real pins on your board if needed
const int BUTTON1_PIN = 35; // button A
const int BUTTON2_PIN = 34; // button B
const bool BUTTON_ACTIVE_LOW = true; // true if buttons pull to GND when pressed

// Temporary upload buffer in RAM
std::vector<uint8_t> uploadBuffer;
const size_t MAX_UPLOAD_SIZE = 512 * 1024; // maximum 512 KB

// Home screen state
bool homeShowing = false;
unsigned long homeShowUntil = 0;

// Simple HTML page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html>
  <head><meta charset="utf-8"><title>BusyTag Upload</title></head>
  <body>
    <h3>BusyTag — Upload image (JPEG)</h3>
    <form method="POST" action="/upload" enctype="multipart/form-data">
      <input type="file" name="file" accept="image/jpeg,image/jpg"><br><br>
      <input type="submit" value="Upload">
    </form>
    <p>Note: GIF/WebP not supported in this build (see device notes).</p>
  </body>
</html>
)rawliteral";

// TJpg_Decoder callback for drawing to TFT_eSPI
bool tft_output(int16_t xpos, int16_t ypos, uint16_t w, uint16_t h, uint8_t *bitmap) {
  tft.pushImage(xpos, ypos, w, h, (uint16_t*)bitmap);
  return true;
}

void showAPInfoOnScreen(const String &ssid, const char *password, IPAddress ip) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.println("BusyTag AP");
  tft.setTextSize(1);
  tft.setCursor(8, 48);
  tft.printf("SSID: %s\n", ssid.c_str());
  tft.setCursor(8, 68);
  tft.printf("Password: %s\n", password);
  tft.setCursor(8, 92);
  tft.printf("Upload page:\nhttp://%s/\n", ip.toString().c_str());
}

void drawJpegFromBuffer(const uint8_t *data, size_t len) {
  if (!data || len == 0) return;
  TJpgDec.setJpgScale(1); // scale 1..8
  TJpgDec.setCallback(tft_output);
  if (!TJpgDec.drawJpg((uint8_t*)data, (int)len, 0, 0)) {
    Serial.println("TJpgDec: failed to draw JPG");
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 120);
    tft.println("Invalid JPG");
  } else {
    Serial.println("Image drawn");
  }
}

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  static bool uploadStarted = false;

  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Upload start: %s\n", upload.filename.c_str());
    uploadBuffer.clear();
    uploadBuffer.reserve(100 * 1024);
    uploadStarted = true;
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!uploadStarted) {
      server.send(500, "text/plain", "Upload error");
      return;
    }
    if (uploadBuffer.size() + upload.currentSize > MAX_UPLOAD_SIZE) {
      Serial.println("Upload too large, aborting");
      uploadBuffer.clear();
      uploadStarted = false;
      server.send(413, "text/plain", "File too large");
      return;
    }
    uploadBuffer.insert(uploadBuffer.end(), upload.buf, upload.buf + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!uploadStarted) {
      server.send(500, "text/plain", "Upload error at end");
      return;
    }
    Serial.printf("Upload complete: %u bytes\n", upload.totalSize);
    uploadStarted = false;
    drawJpegFromBuffer(uploadBuffer.data(), uploadBuffer.size());
    server.sendHeader("Location", "/");
    server.send(303);
  }
}

bool isButtonPressed(int pin) {
  bool v = digitalRead(pin);
  return BUTTON_ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  char ssidbuf[32];
  snprintf(ssidbuf, sizeof(ssidbuf), "BusyTag-%02X%02X", mac[4], mac[5]);
  apSSID = String(ssidbuf);

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(apSSID.c_str(), apPassword);
  if (!ok) {
    Serial.println("softAP failed");
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED);
    tft.setCursor(8, 8);
    tft.println("AP start failed");
    return;
  }
  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP %s started, IP: %s\n", apSSID.c_str(), ip.toString().c_str());

  showAPInfoOnScreen(apSSID, apPassword, ip);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, [](){ server.send(200); }, handleUpload);
  server.begin();

  TJpgDec.setCallback(tft_output);
}

unsigned long lastButtonCheck = 0;
const unsigned long BUTTON_POLL_INTERVAL = 50;
bool lastButtonState1 = false;
bool lastButtonState2 = false;

void loop() {
  server.handleClient();

  unsigned long now = millis();

  if (homeShowing && now >= homeShowUntil) {
    homeShowing = false;
    Serial.println("Home screen timeout expired, restoring view");
  }

  if (now - lastButtonCheck >= BUTTON_POLL_INTERVAL) {
    lastButtonCheck = now;
    bool b1 = isButtonPressed(BUTTON1_PIN);
    bool b2 = isButtonPressed(BUTTON2_PIN);

    if (b1 && !lastButtonState1) {
      Serial.println("Button 1 pressed");
      homeShowing = true;
      homeShowUntil = now + HOME_SCREEN_TIMEOUT;
      IPAddress ip = WiFi.softAPIP();
      showAPInfoOnScreen(apSSID, apPassword, ip);
    }
    if (b2 && !lastButtonState2) {
      Serial.println("Button 2 pressed");
      homeShowing = true;
      homeShowUntil = now + HOME_SCREEN_TIMEOUT;
      IPAddress ip = WiFi.softAPIP();
      showAPInfoOnScreen(apSSID, apPassword, ip);
    }
    lastButtonState1 = b1;
    lastButtonState2 = b2;
  }
}
