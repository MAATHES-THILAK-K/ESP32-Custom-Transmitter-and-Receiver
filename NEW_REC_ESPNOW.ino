#include <esp_now.h>         // ESP-NOW communication protocol
#include <WiFi.h>            // Required for setting WiFi mode
#include <Wire.h>            // For I2C communication
#include <U8g2lib.h>         // OLED display library (U8g2)
#include <esp_wifi.h>        // Low-level WiFi functions (used for channel info)

// === Configuration ===
// Timeout duration (in milliseconds) to detect signal loss
#define SIGNAL_TIMEOUT 1000  

// === OLED Display Setup ===
// SH1106 OLED (128x64) on ESP32-CAM using GPIO 15 (SDA), 14 (SCL)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// === Communication Timing ===
unsigned long lastRecvTime = 0;  // Stores the last time a packet was received

// === Data Packet Structure ===
// This structure must match the sender's format exactly
struct PacketData {
  byte lyAxisValue;     // Left Y-axis (Dual Joystick)
  byte rxAxisValue;     // Right X-axis
  byte ryAxisValue;     // Right Y-axis
  byte switch1Value;    // Button 1
  byte switch2Value;    // Button 2
  byte switch3Value;    // Button 3
  byte switch4Value;    // Button 4
  byte posAvalue;       // Slide Switch Position A
  byte posCvalue;       // Slide Switch Position C
  byte posONvalue;      // Power Switch State
};

PacketData receiverData;  // Object to store the received data

// === ESP-NOW Callback ===
// This function is called when data is received
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len != sizeof(receiverData)) return;  // Ignore incorrect packet size

  memcpy(&receiverData, incomingData, sizeof(receiverData));  // Copy data into struct
  lastRecvTime = millis();  // Reset signal timeout

  // Debug output in Serial Monitor
  char inputValuesString[100];
  sprintf(inputValuesString, 
          "%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d",
          receiverData.lyAxisValue,
          receiverData.rxAxisValue,
          receiverData.ryAxisValue,
          receiverData.switch1Value,
          receiverData.switch2Value,
          receiverData.switch3Value,
          receiverData.switch4Value,
          receiverData.posAvalue,
          receiverData.posCvalue,
          receiverData.posONvalue);
  Serial.println(inputValuesString);
}

void setup() {
  Serial.begin(115200);  // Begin serial for debug

  // === Initialize OLED Display ===
  Wire.begin(15, 14);      // I2C configuration: SDA = GPIO15, SCL = GPIO14
  u8g2.begin();            // Start OLED
  u8g2.clearBuffer();      
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 20, "Waiting for Data...");
  u8g2.sendBuffer();

  // === Setup WiFi in Station Mode ===
  WiFi.mode(WIFI_STA);     // Required for ESP-NOW
  WiFi.setSleep(false);    // Disable sleep for consistent reception

  // Optional: Display current WiFi channel for ESP-NOW pairing
  esp_wifi_set_promiscuous(true);
  wifi_second_chan_t second;
  uint8_t primary;
  esp_wifi_get_channel(&primary, &second);
  Serial.print("Receiver Channel: ");
  Serial.println(primary);
  esp_wifi_set_promiscuous(false);

  // === Initialize ESP-NOW ===
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }

  // Register callback function to handle incoming data
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  unsigned long now = millis();

  // === Signal Timeout Check ===
  if (now - lastRecvTime > SIGNAL_TIMEOUT) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 30, "No Signal!");
    u8g2.sendBuffer();
    return;  // Skip rest of loop
  }

  // === OLED Display of Incoming Data ===
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);

  // Line 1: Joystick Values
  u8g2.setCursor(0, 10);   u8g2.print("LY:"); u8g2.print(receiverData.lyAxisValue);
  u8g2.setCursor(64, 10);  u8g2.print("RX:"); u8g2.print(receiverData.rxAxisValue);
  u8g2.setCursor(0, 20);   u8g2.print("RY:"); u8g2.print(receiverData.ryAxisValue);

  // Line 2: Buttons
  u8g2.setCursor(0, 30);   u8g2.print("S1:"); u8g2.print(receiverData.switch1Value);
  u8g2.setCursor(32, 30);  u8g2.print("S2:"); u8g2.print(receiverData.switch2Value);
  u8g2.setCursor(64, 30);  u8g2.print("S3:"); u8g2.print(receiverData.switch3Value);
  u8g2.setCursor(96, 30);  u8g2.print("S4:"); u8g2.print(receiverData.switch4Value);

  // Line 3: Slide and Power Switch
  u8g2.setCursor(0, 40);   u8g2.print("PA:"); u8g2.print(receiverData.posAvalue);
  u8g2.setCursor(48, 40);  u8g2.print("PC:"); u8g2.print(receiverData.posCvalue);
  u8g2.setCursor(96, 40);  u8g2.print("ON:"); u8g2.print(receiverData.posONvalue);

  u8g2.sendBuffer();  // Refresh OLED screen with new data

  delay(100);  // Display update rate (10 FPS)
}
