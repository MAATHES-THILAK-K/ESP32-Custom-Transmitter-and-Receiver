#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <esp_wifi.h>

// Define signal timeout
#define SIGNAL_TIMEOUT 1000  // in milliseconds

// OLED display (SH1106) using I2C on GPIO 15 (SDA), 14 (SCL)
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset =*/ U8X8_PIN_NONE);

unsigned long lastRecvTime = 0;

struct PacketData {
  byte lyAxisValue;
  byte rxAxisValue;
  byte ryAxisValue;
  byte switch1Value;
  byte switch2Value;
  byte switch3Value;
  byte switch4Value;
  byte posAvalue;
  byte posCvalue;
  byte posONvalue;
};

PacketData receiverData;

// Callback function for received data
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len)
 {
  if (len != sizeof(receiverData)) return;
  memcpy(&receiverData, incomingData, sizeof(receiverData));

  lastRecvTime = millis();

  // Debug print in serial
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
  Serial.begin(115200);
  
  // Init OLED
  Wire.begin(15, 14);  // SDA = GPIO15, SCL = GPIO14 for ESP32-CAM
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 20, "Waiting for Data...");
  u8g2.sendBuffer();

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  esp_wifi_set_promiscuous(true);
  wifi_second_chan_t second;
  uint8_t primary;
  esp_wifi_get_channel(&primary, &second);
  Serial.print("Receiver Channel: ");
  Serial.println(primary);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastRecvTime > SIGNAL_TIMEOUT) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 30, "No Signal!");
    u8g2.sendBuffer();
    return;
  }

  // Display received values on OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.setCursor(0, 10);  u8g2.print("LY:"); u8g2.print(receiverData.lyAxisValue);
  u8g2.setCursor(64, 10); u8g2.print("RX:"); u8g2.print(receiverData.rxAxisValue);
  u8g2.setCursor(0, 20);  u8g2.print("RY:"); u8g2.print(receiverData.ryAxisValue);

  u8g2.setCursor(0, 30);  u8g2.print("S1:"); u8g2.print(receiverData.switch1Value);
  u8g2.setCursor(32, 30); u8g2.print("S2:"); u8g2.print(receiverData.switch2Value);
  u8g2.setCursor(64, 30); u8g2.print("S3:"); u8g2.print(receiverData.switch3Value);
  u8g2.setCursor(96, 30); u8g2.print("S4:"); u8g2.print(receiverData.switch4Value);

  u8g2.setCursor(0, 40);  u8g2.print("PA:"); u8g2.print(receiverData.posAvalue);
  u8g2.setCursor(48, 40); u8g2.print("PC:"); u8g2.print(receiverData.posCvalue);
  u8g2.setCursor(96, 40); u8g2.print("ON:"); u8g2.print(receiverData.posONvalue);
  u8g2.sendBuffer();

  delay(100);  // Update OLED every 100ms
}