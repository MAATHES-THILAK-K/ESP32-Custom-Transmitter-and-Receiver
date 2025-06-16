#include <esp_now.h>         // ESP-NOW communication protocol
#include <WiFi.h>            // WiFi library required for ESP-NOW
#include <esp_wifi.h>        // For setting specific WiFi channel

// === GPIO Definitions ===
#define ind_led 25           // Indicator LED (shows connection status)
#define ir_led 33            // Optional IR LED (you can use it in custom logic)

// === MAC Address of the Receiver ===
// Make sure this matches the receiver's WiFi MAC address
uint8_t receiverMacAddress[] = {0xEC, 0x64, 0xC9, 0xAC, 0xAB, 0x98};

// === Data Structure to Send ===
// This structure must match the receiver's `PacketData` exactly
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

PacketData data;             // Object to hold sensor/button data

// === Connection and Timing Variables ===
bool isConnected = false;
unsigned long lastSendTime = 0;
unsigned long lastStatusCheck = 0;
const unsigned long connectionTimeout = 1000;
bool ledState = false;       // For blinking the indicator LED when disconnected

// === Mapping Functions ===
// These convert raw analog values into usable 0–100 range, with deadzones

// Single-axis joystick (e.g., LY)
int mapSingleAxis(int val) {
  const int minVal = 100;
  const int maxVal = 4095;
  const int deadzoneMin = 1950;
  const int deadzoneMax = 2050;

  if (val > deadzoneMin && val < deadzoneMax) {
    return 50;  // Neutral value
  }

  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);  // Inverted mapping
}

// Dual-axis joystick: X mapping
int mapDualAxisX(int val) {
  const int minVal = 69;     // Observed min value
  const int maxVal = 4095;
  const int deadzoneMin = 2950;
  const int deadzoneMax = 3030;

  if (val >= deadzoneMin && val <= deadzoneMax)
    return 50;

  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);
}

// Dual-axis joystick: Y mapping
int mapDualAxisY(int val) {
  const int minVal = 0;
  const int maxVal = 4095;
  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);
}

// === Callback: Data Sent ===
// Called every time a packet is attempted to be sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  isConnected = (status == ESP_NOW_SEND_SUCCESS);
  if (isConnected) lastSendTime = millis();

  Serial.print("\r\nLast Packet Send Status:\t ");
  Serial.println(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Message sent" : "Message failed");
}

void setup() {
  Serial.begin(115200);                  // Start Serial Monitor
  WiFi.mode(WIFI_STA);                   // Must be in station mode
  WiFi.setSleep(false);                  // Disable power saving

  // Force a known channel (must match the receiver’s channel)
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  pinMode(ind_led, OUTPUT);             // LED for signal indication
  digitalWrite(ind_led, LOW);

  analogReadResolution(12);             // 12-bit ADC for ESP32 (0–4095)

  // === Initialize ESP-NOW ===
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("✅ ESP-NOW initialized");
  }

  esp_now_register_send_cb(OnDataSent);  // Register send callback

  // === Configure Peer ===
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  // Re-add peer if already exists
  if (esp_now_is_peer_exist(receiverMacAddress)) {
    Serial.println("ℹ️ Peer already exists. Deleting and re-adding...");
    esp_now_del_peer(receiverMacAddress);
  }

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Failed to add peer");
    return;
  } else {
    Serial.println("✅ Peer added successfully");
  }

  // === Configure Button & Switch Pins ===
  pinMode(13, INPUT_PULLUP);  // Button 1
  pinMode(12, INPUT_PULLUP);  // Button 2
  pinMode(14, INPUT_PULLUP);  // Button 3
  pinMode(27, INPUT_PULLUP);  // Button 4
  pinMode(4,  INPUT_PULLUP);  // Slide A
  pinMode(16, INPUT_PULLUP);  // Slide C
  pinMode(17, INPUT_PULLUP);  // Power Switch
}

void loop() {
  // === Read Joystick Values ===
  data.lyAxisValue = mapSingleAxis(analogRead(32));
  data.rxAxisValue = mapDualAxisX(analogRead(35));
  data.ryAxisValue = mapDualAxisY(analogRead(34));

  // === Read Button and Switch States ===
  data.switch1Value = digitalRead(13) == LOW ? 1 : 0;
  data.switch2Value = digitalRead(12) == LOW ? 1 : 0;
  data.switch3Value = digitalRead(27) == LOW ? 1 : 0;
  data.switch4Value = digitalRead(14) == LOW ? 1 : 0;
  data.posAvalue    = digitalRead(4)  == LOW ? 1 : 0;
  data.posCvalue    = digitalRead(16) == LOW ? 1 : 0;
  data.posONvalue   = digitalRead(17) == LOW ? 1 : 0;

  // === Debug Print Raw Analog Values ===
  int rawLY = analogRead(32);
  int rawRX = analogRead(35);
  int rawRY = analogRead(34);
  Serial.printf("Raw LY: %d, RX: %d, RY: %d\n", rawLY, rawRX, rawRY);

  // === Debug Print Processed Values ===
  Serial.printf("LY:%3d RX:%3d RY:%3d | SW1:%d SW2:%d SW3:%d SW4:%d | A:%d C:%d ON:%d\n",
                data.lyAxisValue, data.rxAxisValue, data.ryAxisValue,
                data.switch1Value, data.switch2Value, data.switch3Value, data.switch4Value,
                data.posAvalue, data.posCvalue, data.posONvalue);

  // === Send Data Packet ===
  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *)&data, sizeof(data));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

  // === Signal LED Behavior ===
  if (millis() - lastSendTime > connectionTimeout) {
    isConnected = false;  // Consider connection lost
  }

  if (isConnected) {
    digitalWrite(ind_led, HIGH);  // Steady ON when connected
  } else {
    // Blink LED slowly when disconnected
    if (millis() - lastStatusCheck > 300) {
      ledState = !ledState;
      digitalWrite(ind_led, ledState);
      lastStatusCheck = millis();
    }
  }

  delay(50);  // Small delay to prevent flooding
}
