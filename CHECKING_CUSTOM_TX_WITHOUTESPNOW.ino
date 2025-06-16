// === Joystick Pin Definitions (ADC1 only) ===
const int joy1X = 35;
const int joy1Y = 34;
const int joy2  = 32;

// === Buttons / Switches ===
const int btn1 = 13;
const int btn2 = 12;
const int btn3 = 14;
const int btn4 = 27;
const int slideA = 4;
const int slideC = 16;
const int powerSw = 17;

// === Indicator LED ===
const int ledPin = 25;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // ADC 0–4095

  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(btn3, INPUT_PULLUP);
  pinMode(btn4, INPUT_PULLUP);
  pinMode(slideA, INPUT_PULLUP);
  pinMode(slideC, INPUT_PULLUP);
  pinMode(powerSw, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // === Read joystick raw values ===
  int rawX1 = analogRead(joy1X);  // Dual axis X
  int rawY1 = analogRead(joy1Y);  // Dual axis Y
  int rawY2 = analogRead(joy2);   // Single axis

  // === Map to 0–100 range (center = 50) ===
  int mapDualX = mapDualAxisX(rawX1);
  int mapDualY = mapDualAxisY(rawY1);
  int mapSingle = mapSingleAxis(rawY2);

  // === Read digital inputs ===
  bool b1 = digitalRead(btn1) == LOW;
  bool b2 = digitalRead(btn2) == LOW;
  bool b3 = digitalRead(btn3) == LOW;
  bool b4 = digitalRead(btn4) == LOW;
  bool sA = digitalRead(slideA) == LOW;
  bool sC = digitalRead(slideC) == LOW;
  bool pW = digitalRead(powerSw) == LOW;

  // === Indicator LED logic ===
  digitalWrite(ledPin, (b1 || b2 || b3 || b4 || sA || sC || pW) ? HIGH : LOW);

  // === Serial Output ===
  Serial.print("Raw X: "); Serial.print(rawX1);
  Serial.print("  Raw Y: "); Serial.print(rawY1);
  Serial.print("  Raw Single: "); Serial.print(rawY2);
  
  Serial.print("  | Mapped X: "); Serial.print(mapDualX);
  Serial.print("  Y: "); Serial.print(mapDualY);
  Serial.print("  Single: "); Serial.print(mapSingle);

  Serial.print("  Btns: ");
  Serial.print(b1); Serial.print(b2); Serial.print(b3); Serial.print(b4);

  Serial.print("  Slides: A="); Serial.print(sA);
  Serial.print(" C="); Serial.print(sC);
  Serial.print("  Power: "); Serial.print(pW);

  Serial.println();

  delay(300); // Refresh interval
}

// === Single Axis Mapping ===
int mapSingleAxis(int val) {
  const int minVal = 60;
  const int maxVal = 4095;
  const int centerVal = 2245;

  if (val <= centerVal) {
    return map(val, minVal, centerVal, 0, 50);
  } else {
    return map(val, centerVal, maxVal, 50, 100);
  }
}

// === Dual Axis X Mapping ===
int mapDualAxisX(int val) {
  const int minVal = 540;     // Max joystick tilt → 0%
  const int centerVal = 3040;
  const int maxVal = 4095;    // Opposite direction → 100%

  if (val <= centerVal) {
    return map(val, minVal, centerVal, 100, 50); // Left to center
  } else {
    return map(val, centerVal, maxVal, 50, 0);   // Center to right
  }
}

// === Dual Axis Y Mapping ===
int mapDualAxisY(int val) {
  const int minVal = 0;
  const int centerVal = 2630;
  const int maxVal = 4095;

  if (val <= centerVal) {
    return map(val, minVal, centerVal, 100, 50); // Up to center
  } else {
    return map(val, centerVal, maxVal, 50, 0);   // Center to down
  }
}
