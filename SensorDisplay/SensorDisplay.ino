
#include <Wire.h>
// #include <WiFi.h>
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

#define ZE08_CH20_RX_PIN 16   // your actual RX pin
#define ZE08_CH20_TX_PIN -1   // unused
#define BOOT_BUTTON_PIN 0  // ESP32 Dev Kit 上 BOOT 按钮通常接 GPIO0


#define SCREEN_WIDTH 128 // OLED display_ width, in pixels
#define SCREEN_HEIGHT 64 // OLED display_ height, in pixels
#define TEXT_SIZE 2
#define LINE_HEIGHT (10 * 2)
#define PACKET_SIZE 9


const char* ssid = "DEEP-RD";
const char* password = "07310731";
const char* serverURL = "http://192.168.10.39:40015/sensor";

volatile bool bootPressed = false;
RTC_DATA_ATTR bool deviceOn = true;  // remember state across deep sleep

// 中断服务函数（ISR）
void IRAM_ATTR bootButtonISR() {
  bootPressed = true;  // 设置标志位
}

// https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
// Declaration for an SSD1306 display_ connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display_(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void SetUpDisplay();

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP); // 按钮一般是低电平按下
  attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON_PIN), bootButtonISR, FALLING);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // wakeup on LOW

  // 降到 80MHz（默认是 240MHz）
  setCpuFrequencyMhz(80);
  Serial.println("Begin");

  Serial.print("CPU Freq set to: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");


  SetUpDisplay();

  Serial1.begin(9600, SERIAL_8N1, ZE08_CH20_RX_PIN, ZE08_CH20_TX_PIN);
  Serial.println("Listening to UART sensor...");

  // WiFi.begin(ssid, password);
  // Serial.println("WiFi connecting");
  // while(WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  // }
  // Serial.println("WiFi connected");
}

static void CleanLine(size_t line_id) {
  display_.fillRect(0, line_id * LINE_HEIGHT, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_BLACK);  // clear line
}

static void LoopZE08CH2O() {
  static uint8_t buffer[PACKET_SIZE];
  static uint8_t index = 0;

  while (Serial1.available() > 0) {
    uint8_t b = Serial1.read();

    // Start of packet: first byte should be 0xFF
    if (index == 0 && b != 0xFF) continue;
    buffer[index++] = b;
    if (index == PACKET_SIZE) {
      // Print raw hex
      // for (int i = 0; i < PACKET_SIZE; i++) {
      //   if (b < 0x10) display_.print("0");
      //   display_.print(b, HEX);
      //   display_.print(" ");
      // }

      // Optionally, parse gas concentration
      uint16_t concentration = ((uint16_t)buffer[4] << 8) | buffer[5];
      uint16_t full_scale = ((uint16_t)buffer[6] << 8) | buffer[7];
      uint8_t gas_name = buffer[1];

      double ppb = concentration;
      double mg_m3 = (ppb  * 1.25) * 0.001;
      // double mg_m3 = ppm * 1.25;

      // Serial.print(" | Gas: 0x");
      // if (gas_name < 0x10) Serial.print("0");
      // Serial.print(gas_name, HEX);
      // Serial.print(" ppb: ");
      // Serial.print(ppb, 4);
      // Serial.print(" mg_m3: ");
      // Serial.print(mg_m3, 4);
      // Serial.print(" Concentration: ");
      // Serial.print(concentration);
      // Serial.print(" / Full Scale: ");
      // Serial.println(full_scale);


      CleanLine(1);
      display_.setCursor(0, 1 * LINE_HEIGHT);
      display_.print(mg_m3, 3);
      display_.print("mg/m3");
      index = 0; // Reset for next packet
    }

  }
}

// static int last_button_state_ = LOW;
static int cnt_ = 0;
void loop() {
  if (bootPressed) {
    Serial.println("BOOT button pressed!");
    bootPressed = false;
    if (deviceOn) {
      Serial.println("System Sleep.");
      // Turn off display
      display_.clearDisplay();
      display_.display();
      display_.ssd1306_command(SSD1306_DISPLAYOFF);
      Serial.println("Display turned off");
      esp_deep_sleep_start();
      deviceOn = false;
    }
  }

  CleanLine(0);  // clear line
  display_.setCursor(0, 0);
  display_.print("CH2O ");
  display_.print(cnt_++);

  LoopZE08CH2O();
  display_.display();

  // if (WiFi.status() == WL_CONNECTED) {
  //   HTTPClient http;
  //   http.begin(serverURL);
  //   http.addHeader("Content-Type", "application/json");
  //
  //   StaticJsonDocument<200> doc;
  //   doc["device"] = "esp32_viki";
  //   doc["temperature"] = 25;
  //   doc["humidity"] = cnt_;
  //   String json;
  //   serializeJson(doc, json);
  //
  //   int code = http.POST(json);
  //   if (code > 0) Serial.println("Sent data, response: " + String(code));
  //   http.end();
  // }

  delay(500);
}

void SetUpDisplay() {
  // Setup the display_
  if (!display_.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println("SSD1306 allocation failed! Try again...");
    for(;;);
  }
  delay(1000);
  display_.clearDisplay();

  display_.ssd1306_command(SSD1306_SETCONTRAST);
  display_.ssd1306_command(0x5F); // 0x00~0xFF，越小越省电

  display_.setTextSize(TEXT_SIZE);
  display_.setTextColor(SSD1306_WHITE);
  display_.display();
}
