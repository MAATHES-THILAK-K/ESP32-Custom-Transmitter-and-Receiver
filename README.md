# 🚁 ESP32-Based Custom Transmitter using ESP-NOW

This project is a **custom transmitter built using ESP32**, re-engineered from an old SJ helicopter IR-based remote. Instead of low-range IR communication, this version leverages **ESP-NOW**, a high-speed, connectionless wireless protocol by Espressif, to transmit joystick and button data with low latency and high reliability.

---

## 🧠 Project Background

I repurposed an **old SJ helicopter transmitter**, reverse-engineering the hardware using a multimeter to identify and **cut traces** leading to outdated ICs and components that were irrelevant to my use-case. The original IR system suffered from extremely **short range and interference**, which motivated me to adopt **ESP-NOW** for wireless communication.

Due to the limited availability of two ESP32 modules, I used one **ESP32 Dev Board** as the transmitter and an **ESP32-CAM** as the receiver. While functional, the ESP32-CAM's power-hungry nature and occasional reset issues **slowed down development**. I recommend using **a good-quality 2A-rated USB cable** for better stability.

---

## 🚀 ESP-NOW Overview

[ESP-NOW](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html) is a **fast, peer-to-peer wireless communication protocol** developed by Espressif. It allows ESP32/ESP8266 devices to exchange data without Wi-Fi association.

**Key Benefits:**
- No router needed
- Ultra-low latency (~1–2ms)
- Low power consumption
- Supports broadcast and unicast
- Highly suitable for remote controls, sensor networks, and mesh systems

For official documentation, visit:  
🔗 [ESP-NOW by Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)

---

## 🧩 Hardware Configuration

### 📟 Transmitter (ESP32 Dev Board)
- **Joystick 1 (Dual Axis)**  
  - X-axis → `GPIO 34` (ADC1_CH6)  
  - Y-axis → `GPIO 35` (ADC1_CH7)  
- **Joystick 2 (Single Axis)**  
  - Axis → `GPIO 32` (ADC1_CH4)  
- **Tactile Buttons**
  - Button 1 → `GPIO 13`  
  - Button 2 → `GPIO 12` *(⚠️ Avoid pulling LOW on boot)*  
  - Button 3 → `GPIO 14`  
  - Button 4 → `GPIO 27`  
- **Slide Switch (3-Position)**
  - Position A → `GPIO 4` (Active-LOW)  
  - Position C → `GPIO 16` *(ADC2, use as digital only)*  
- **Power Switch**
  - On/Off → `GPIO 17` (used for sleep mode)  
- **Output**
  - IR LED → `GPIO 33` (via transistor driver)  
  - Indicator LED → `GPIO 25` *(not used in code yet; 220Ω resistor recommended)*  

---

### 📺 Receiver (ESP32-CAM)
- **OLED Display:** SH1106, I2C via U8g2lib  
  - `SDA` → `GPIO 15`  
  - `SCL` → `GPIO 14`  

The receiver uses the `U8g2` graphics library to visualize incoming control data such as joystick positions and button status.

---

## 🧪 Development Workflow

1. **Hardware Testing**:  
   I began by serial printing joystick and button values to verify connections and signal quality.

2. **OLED Feedback**:  
   Implemented SH1106 OLED support using `U8g2lib` to display live transmitter state on the receiver side.

3. **ESP-NOW Integration**:  
   The transmitter sends a custom data structure (joystick + buttons) via ESP-NOW every few milliseconds. The receiver decodes this and displays the state in real-time.

---

## 💡 Tips for Builders

- Use **ADC1 pins only** for analog reads when using Wi-Fi/ESP-NOW (ADC2 conflicts).
- Avoid using `GPIO 12` pulled LOW during boot; it affects flash mode.
- Always **use pull-up resistors or INPUT_PULLUP mode** for switches.
- Use **short, high-current USB cables (2A rated)** especially for ESP32-CAM boards.
- If facing frequent ESP32-CAM resets, consider adding a **low ESR capacitor** (470µF+) across VCC and GND.

---

## 📈 Future Improvements
- Implement **power-saving modes** using deep sleep and wakeup on GPIO
- Integrate **vibration feedback or buzzer**
- Add **CRC check** for ESP-NOW data integrity
- Create a **bi-directional communication system** (transmit + receive)
- Enclosure design using 3D printing

---

## 👨‍🎓 Author

**KMT**  
ECE Undergraduate – Anna University, MIT Campus, Chennai, Tamil Nadu  
💡 Passionate about Microcontrollers, PCB Design, IoT, and Space Tech

---

## 📜 License

This project is open-source under the [MIT License](LICENSE)

