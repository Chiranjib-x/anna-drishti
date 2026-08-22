/* ============================================================================
   Anna-Drishti - jar reader
   ESP32 + SHT3x (+ optional SSD1306 OLED)

   Reads humidity and temperature from a grain jar headspace and reports water
   activity. Type the RH and temperature into the Bench reading panel in
   index.html; that is where the mould model lives. This firmware deliberately
   does NOT predict mould - one source of truth for the science.

   WIRING (single sensor, which is all Monday needs):
     SHT3x VIN -> 3V3        OLED VCC -> 3V3
     SHT3x GND -> GND        OLED GND -> GND
     SHT3x SDA -> GPIO 21    OLED SDA -> GPIO 21
     SHT3x SCL -> GPIO 22    OLED SCL -> GPIO 22
     SHT3x ADDR-> GND (0x44) or 3V3 (0x45)

   POWER: use the USB cable. If you must use the 24 V adapter, set the LM2596
   output to exactly 5.0 V with a multimeter BEFORE connecting VIN, or you will
   destroy the board.

   OLED is optional - if it is missing or dead, Serial still works.
   ========================================================================== */

#include <Wire.h>

// ---- optional display. Comment this line out if you are not using the OLED,
//      or if the Adafruit libraries are not installed. -----------------------
#define USE_OLED

#ifdef USE_OLED
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 oled(128, 64, &Wire, -1);
  bool oledOK = false;
#endif

const uint8_t  SHT_ADDR   = 0x44;   // 0x45 if ADDR is tied high
const uint8_t  OLED_ADDR  = 0x3C;
const uint32_t READ_MS    = 1000;

// Stability: the reading has settled when the last STAB_N samples span less
// than these thresholds. Record the value only once it says STABLE - headspace
// humidity takes 5-15 minutes to equilibrate after opening a jar.
const int   STAB_N  = 15;
const float STAB_RH = 0.30;   // %RH
const float STAB_T  = 0.15;   // degC

float rhBuf[STAB_N], tBuf[STAB_N];
int   nBuf = 0, iBuf = 0;

/* ---- CRC-8, poly 0x31, init 0xFF (Sensirion). A corrupted I2C read that
   still looks like a plausible humidity would silently corrupt our science,
   so every frame is checked rather than trusted. ------------------------- */
uint8_t crc8(const uint8_t *d, int n){
  uint8_t c = 0xFF;
  for (int i = 0; i < n; i++){
    c ^= d[i];
    for (int b = 0; b < 8; b++) c = (c & 0x80) ? (uint8_t)((c << 1) ^ 0x31) : (uint8_t)(c << 1);
  }
  return c;
}

// ponytail: raw register access instead of a sensor library - it is 20 lines,
// and it removes a dependency that has to be installed on someone else's laptop.
bool readSHT(float &rh, float &t){
  Wire.beginTransmission(SHT_ADDR);
  Wire.write(0x2C); Wire.write(0x06);         // high repeatability, clock stretching
  if (Wire.endTransmission() != 0) return false;
  delay(20);

  if (Wire.requestFrom((int)SHT_ADDR, 6) != 6) return false;
  uint8_t d[6];
  for (int i = 0; i < 6; i++) d[i] = Wire.read();
  if (crc8(d, 2) != d[2] || crc8(d + 3, 2) != d[5]) return false;

  uint16_t rawT = (d[0] << 8) | d[1];
  uint16_t rawH = (d[3] << 8) | d[4];
  t  = -45.0f + 175.0f * (float)rawT / 65535.0f;
  rh = 100.0f * (float)rawH / 65535.0f;
  return true;
}

float spread(const float *b, int n){
  float lo = b[0], hi = b[0];
  for (int i = 1; i < n; i++){ if (b[i] < lo) lo = b[i]; if (b[i] > hi) hi = b[i]; }
  return hi - lo;
}

void setup(){
  Serial.begin(115200);
  delay(300);
  Wire.begin(21, 22);
  Wire.setClock(100000);          // 100 kHz - kinder to long cable runs

#ifdef USE_OLED
  oledOK = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (oledOK){ oled.clearDisplay(); oled.setTextColor(SSD1306_WHITE); oled.display(); }
  else Serial.println("OLED not found - continuing on Serial only");
#endif

  // bus scan, so a wiring fault is obvious instead of looking like a dead sensor
  Serial.println("\nI2C scan:");
  int found = 0;
  for (uint8_t a = 1; a < 127; a++){
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0){ Serial.printf("  found 0x%02X\n", a); found++; }
  }
  if (!found) Serial.println("  NOTHING FOUND - check SDA/SCL/power before anything else");

  Serial.println("\nelapsed_s,rh_pct,temp_c,aw,stable");
}

void loop(){
  static uint32_t last = 0;
  if (millis() - last < READ_MS) return;
  last = millis();

  float rh, t;
  if (!readSHT(rh, t)){
    Serial.println("# read failed (CRC or bus error)");
#ifdef USE_OLED
    if (oledOK){
      oled.clearDisplay(); oled.setTextSize(1); oled.setCursor(0, 28);
      oled.println("SENSOR READ FAILED"); oled.display();
    }
#endif
    return;
  }

  rhBuf[iBuf] = rh; tBuf[iBuf] = t;
  iBuf = (iBuf + 1) % STAB_N;
  if (nBuf < STAB_N) nBuf++;

  bool stable = (nBuf == STAB_N) &&
                spread(rhBuf, nBuf) < STAB_RH && spread(tBuf, nBuf) < STAB_T;

  // Water activity IS equilibrium relative humidity - this is a restatement of
  // the measurement, not a model. Everything predictive stays in index.html.
  float aw = rh / 100.0f;

  Serial.printf("%lu,%.2f,%.2f,%.4f,%s\n",
                millis() / 1000, rh, t, aw, stable ? "STABLE" : "settling");

#ifdef USE_OLED
  if (oledOK){
    oled.clearDisplay();
    oled.setTextSize(1); oled.setCursor(0, 0);
    oled.println(stable ? "STABLE - record it" : "settling...");
    oled.setTextSize(2); oled.setCursor(0, 14);
    oled.printf("%.1f%%\n", rh);
    oled.setCursor(0, 34);
    oled.printf("%.1fC\n", t);
    oled.setTextSize(1); oled.setCursor(0, 54);
    oled.printf("aw %.3f %s", aw, aw >= 0.82f ? "OVER 0.82" : "safe");
    oled.display();
  }
#endif
}
