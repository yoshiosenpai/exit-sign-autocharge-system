/********  Demo 1: Exit Sign Health Monitor (APDS9960 + Blynk IoT Web)  ********
 * - Sends to Blynk only when live mode is ON (toggle by pressing 'L' in Serial)
 * - Push interval = 2000 ms (2 s)
 * Blynk Datastreams:
 *   V0 (Integer 0..4095): Light value (avg of APDS9960 'Clear')
 *   V1 (Integer 0/1): 1=Exit Sign: FUNCTION (green), 0=NOT FUNCTION (red)
 * Code by Solehin Rizal
 *******************************************************************************/
 
#define BLYNK_TEMPLATE_ID "AMBIK DARI BLYNK"
#define BLYNK_TEMPLATE_NAME "AMBIK DARI BLYNK"
#define BLYNK_AUTH_TOKEN "AMBIK DARI BLYNK"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_APDS9960.h>

// ===== WiFi =====
char ssid[] = "GANTI DGN NAMA WIFI/HOTSPOT";
char pass[] = "PASSWORD WIFI/HOTSPOT";

// ===== APDS9960 (I2C) =====
Adafruit_APDS9960 apds;   // SDA=21, SCL=22 on ESP32 by default

// ===== Light processing =====
const int    AVG_WIN = 8;     // moving average window
uint32_t     acc = 0;
int          accCount = 0;
uint16_t     avgC = 0;        // averaged 'clear' channel
bool         isLit = false;   // decision

// Thresholds (set via serial 1/2/C or tweak manually)
uint16_t TH_ON  = 1200;       // avgC >= TH_ON  => LIT (FUNCTION)
uint16_t TH_OFF =  900;       // avgC <= TH_OFF => DARK (NOT FUNCTION)

// Baselines for quick auto-cal
int baseDark = -1, baseLit = -1;

// ===== Blynk rate control =====
BlynkTimer timer;
const uint32_t PUSH_MS = 2000;       // 2 seconds
int  timerId = -1;                    // active timer id, -1 when stopped
bool liveMode = false;                // toggled by 'L' in Serial

// Reduce noise: only send V0 if changed enough; V1 only when state changes
#define V0_DEADBAND  25
uint16_t lastV0 = 65535;
int      lastV1 = -1;

// ---------- Blynk push ----------
void pushToBlynk(){
  // V0: light (with deadband)
  if (abs((int)avgC - (int)lastV0) >= V0_DEADBAND){
    Blynk.virtualWrite(V0, avgC);
    lastV0 = avgC;
  }

  // V1: status (only on change) + set label/color
  int v1 = isLit ? 1 : 0;
  if (v1 != lastV1){
    Blynk.virtualWrite(V1, v1);
    if (v1 == 1){
      Blynk.setProperty(V1, "label", "Exit Sign: FUNCTION");
      Blynk.setProperty(V1, "color", "#00C851"); // green
    } else {
      Blynk.setProperty(V1, "label", "Exit Sign: NOT FUNCTION");
      Blynk.setProperty(V1, "color", "#ff4444"); // red
    }
    lastV1 = v1;
  }
}

// Start/stop the periodic push
void setLiveMode(bool on){
  if (on && timerId < 0){
    timerId = timer.setInterval(PUSH_MS, pushToBlynk);
    Serial.println("[BLYNK] Live mode ON (2 s interval)");
  } else if (!on && timerId >= 0){
    timer.deleteTimer(timerId);
    timerId = -1;
    Serial.println("[BLYNK] Live mode OFF");
  }
  liveMode = on;
}

void setup() {
  Serial.begin(115200);

  // I2C + APDS9960
  Wire.begin(); // (defaults SDA=21, SCL=22)
  if (!apds.begin()){
    Serial.println("APDS9960 not found. Check I2C wiring (SDA=21, SCL=22, 3V3, GND).");
    while(1) delay(1000);
  }
  apds.enableColor(true);
  Serial.println("APDS9960 OK (color engine enabled).");

  // WiFi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.printf("WiFi connected: %s\n", WiFi.localIP().toString().c_str());

  // Start with live mode OFF (you control it with 'L')
  setLiveMode(false);
}

void loop() {
  Blynk.run();
  timer.run();

  // ---- Read APDS9960 (use Clear channel 'c') ----
  while (!apds.colorDataReady()) delay(5);
  uint16_t r,g,b,c;
  apds.getColorData(&r,&g,&b,&c);

  // Moving average
  acc += c; accCount++;
  if (accCount >= AVG_WIN){
    avgC = acc / AVG_WIN;
    acc = 0; accCount = 0;

    // Decision with hysteresis
    if (!isLit && avgC >= TH_ON)  isLit = true;
    if ( isLit && avgC <= TH_OFF) isLit = false;

    // (No NeoPixel output per request)
    // Serial live view (optional) when NOT sending to Blynk
    // (We’ll keep it quiet unless you ask for S)
  }

  // ---- Serial commands ----
  while (Serial.available()){
    char k = toupper(Serial.read());
    if (k=='L'){ setLiveMode(!liveMode); }               // toggle 2s push
    else if (k=='S'){
      Serial.printf("[STATUS] avgC=%u  LIT=%d  TH_ON=%u TH_OFF=%u  baseDark=%d baseLit=%d  live=%d\n",
        avgC, isLit, TH_ON, TH_OFF, baseDark, baseLit, liveMode);
    }
    else if (k=='1'){ baseDark = avgC; Serial.printf("[CAL] DARK=%d captured\n", baseDark); }
    else if (k=='2'){ baseLit  = avgC; Serial.printf("[CAL] LIT =%d captured\n",  baseLit ); }
    else if (k=='C'){
      if (baseDark>=0 && baseLit>=0 && baseLit>baseDark+50){
        int mid = (baseLit + baseDark)/2;
        int margin = max(30, (baseLit - baseDark)/10); // 10% or >=30
        TH_ON  = mid + margin;
        TH_OFF = mid - margin;
        Serial.printf("[CAL] TH_ON=%u  TH_OFF=%u (mid=%d, margin=%d)\n", TH_ON, TH_OFF, mid, margin);
      } else {
        Serial.println("[CAL] Capture DARK(1) then LIT(2) first.");
      }
    }
  }
}
