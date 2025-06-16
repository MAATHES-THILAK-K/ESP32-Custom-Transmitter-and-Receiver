#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define ind_led 25
#define ir_led 33

uint8_t receiverMacAddress[] = {0xEC,0x64,0xC9,0xAC,0xAB,0x98};

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

PacketData data;

bool isConnected = false;
unsigned long lastSendTime = 0;
unsigned long lastStatusCheck = 0;
const unsigned long connectionTimeout = 1000;
bool ledState = false;

int mapSingleAxis(int val) {
  const int minVal = 100;
  const int maxVal = 4095;
  const int deadzoneMin = 2000 - 50;
  const int deadzoneMax = 2000 + 50;

  if (val > deadzoneMin && val < deadzoneMax) {
    return 50;  // Neutral midpoint (0–100 scale)
  }

  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);  // instead of 0 to 100
}



// === Dual Axis X Mapping ===
int mapDualAxisX(int val) {
  const int minVal = 69;       // Real minimum (left)
  const int maxVal = 4095;     // Full right
  const int deadzoneMin = 2950;
  const int deadzoneMax = 3030;

  if (val >= deadzoneMin && val <= deadzoneMax)
    return 50;  // Center deadzone output

  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);
}

// === Dual Axis Y Mapping ===
int mapDualAxisY(int val) {
  const int minVal = 0;
  const int maxVal = 4095;
  val = constrain(val, minVal, maxVal);
  return map(val, minVal, maxVal, 100, 0);  
}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  isConnected = (status == ESP_NOW_SEND_SUCCESS);
  if (isConnected) lastSendTime = millis();

  Serial.print("\r\nLast Packet Send Status:\t ");
  Serial.println(status);
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Message sent" : "Message failed");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  pinMode(ind_led, OUTPUT);
  digitalWrite(ind_led, LOW);
  analogReadResolution(12);  // Ensure 0–4095 resolution
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("✅ ESP-NOW initialized");
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

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

  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(27, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
}

void loop() {
  data.lyAxisValue = mapSingleAxis(analogRead(32));
  data.rxAxisValue = mapDualAxisX(analogRead(35));
  data.ryAxisValue = mapDualAxisY(analogRead(34));

  data.switch1Value = digitalRead(13) == LOW ? 1 : 0;
  data.switch2Value = digitalRead(12) == LOW ? 1 : 0;
  data.switch3Value = digitalRead(27) == LOW ? 1 : 0;
  data.switch4Value = digitalRead(14) == LOW ? 1 : 0;

  data.posAvalue = digitalRead(4) == LOW ? 1 : 0;
  data.posCvalue = digitalRead(16)  == LOW ? 1 : 0;
  data.posONvalue= digitalRead(17) == LOW ? 1 : 0;

  int rawLY = analogRead(32);
  int rawRX = analogRead(35);
  int rawRY = analogRead(34);

  Serial.printf("Raw LY: %d, RX: %d, RY: %d\n", rawLY, rawRX, rawRY);

  // 🧪 Debug Print for Button States
  Serial.printf("LY:%3d RX:%3d RY:%3d | SW1:%d SW2:%d SW3:%d SW4:%d | A:%d C:%d ON:%d\n",
                data.lyAxisValue, data.rxAxisValue, data.ryAxisValue,
                data.switch1Value, data.switch2Value, data.switch3Value, data.switch4Value,
                data.posAvalue, data.posCvalue, data.posONvalue);

  esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *) &data, sizeof(data));
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  } else {
    Serial.println("Error sending the data");
  }

  if (millis() - lastSendTime > connectionTimeout) {
    isConnected = false;
  }

  if (isConnected) {
    digitalWrite(ind_led, HIGH);
  } else {
    if (millis() - lastStatusCheck > 300) {
      ledState = !ledState;
      digitalWrite(ind_led, ledState);
      lastStatusCheck = millis();
    }
  }

  delay(50);
}