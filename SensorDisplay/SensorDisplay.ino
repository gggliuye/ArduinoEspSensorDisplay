
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#if CONFIG_IDF_TARGET_ESP32C3
// ESP32-C3 specific
// https://github.com/sidharthmohannair/Tutorial-ESP32-C3-Super-Mini/blob/main/docs/examples/Blink/README.md
#define LED_PIN 8
// turn the LED on (LOW because the LED is inverted)
#define LED_LIGHT_OFF HIGH
#define LED_LIGHT_ON LOW
#else
// Regular ESP32 (e.g. DevKit V1)
#define LED_PIN 2
#define LED_LIGHT_OFF LOW
#define LED_LIGHT_ON HIGH
#endif   // #if CONFIG_IDF_TARGET_ESP32C3


#define SCREEN_WIDTH 128 // OLED display_ width, in pixels
#define SCREEN_HEIGHT 64 // OLED display_ height, in pixels
#define LINE_HEIGHT 10

const char* ssid = "DEEP-RD";
const char* password = "07310731";
const char* serverURL = "http://192.168.10.39:40015/sensor";


// https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
// Declaration for an SSD1306 display_ connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void SetUpDisplay();

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  SetUpDisplay();

  WiFi.begin(ssid, password);
  Serial.println("WiFi connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi connected");
}

static void CleanLine(size_t line_id) {
  display_.fillRect(0, line_id * LINE_HEIGHT, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_BLACK);  // clear line
}

// static int last_button_state_ = LOW;
static int cnt_ = 0;
void loop() {
  CleanLine(1);  // clear line
  display_.setCursor(0, LINE_HEIGHT);
  display_.print("Count ");
  display_.println(cnt_++);
  display_.display();


  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> doc;
    doc["device"] = "esp32_viki";
    doc["temperature"] = 25;
    doc["humidity"] = cnt_;
    String json;
    serializeJson(doc, json);

    int code = http.POST(json);
    if (code > 0) Serial.println("Sent data, response: " + String(code));
    http.end();
  }

  delay(1000);
}

void SetUpDisplay() {
  // Setup the display_
  if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed! Try again...");
    for(;;);
  }
  delay(1000);
  display_.clearDisplay();

  display_.setTextSize(1);
  display_.setTextColor(SSD1306_WHITE);
  display_.setCursor(0, 0);
  display_.print("Mobili Sensor Display");
  display_.display();
}
