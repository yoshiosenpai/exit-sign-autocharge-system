/********  Demo 2: Exit Sign Smoke Alarm System (MQ-2 + Buzzer + 12V Lamp via Relay)  ********
 * - Replaces NeoPixel with a 12V lamp switched by a 5V relay module
 * - MQ-2 DO pin used (digital threshold set by module trimpot)
 * - On smoke: buzzer + lamp blink for ALARM_MS, then 1-hour cooldown
 * - Serial: L=live log, S=status, R=reset cooldown, T=test alarm
 * Hardware:
 *   - Relay IN -> ESP32 GPIO15 (configurable)
 *   - Relay VCC -> 5V, GND -> GND (common GND with 12V adapter)
 *   - COM -> +12V, NO -> Lamp +, Lamp - -> 12V GND
 * Code by Solehin Rizal
 *******************************************************************************/

#include <Arduino.h>

// ---------- Pins ----------
#define MQ2_DO         27      // MQ-2 digital output
#define BUZZER_PIN     23      // ROBO ESP32 buzzer
#define RELAY_PIN      15      // Relay IN pin (controls 12V lamp)

// ---------- Relay polarity ----------
// If your relay turns ON when you write LOW, set to 0 (active-LOW).
// If it turns ON when you write HIGH, set to 1 (active-HIGH).
#define RELAY_ACTIVE_HIGH 0

// ---------- Tunables ----------
#define WARMUP_MS       60000UL   // sensor warmup after boot
#define SAMPLE_MS         100UL   // polling period
#define TRIP_HOLD_MS     2000UL   // DO must stay HIGH this long to confirm smoke
#define ALARM_MS        20000UL   // alarm duration on real smoke (test uses 5s)
#define COOLDOWN_MS  3600000UL    // 1 hour cooldown after confirmed alarm

// ---------- Relay helpers ----------
inline void lampOn()  { digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? HIGH : LOW); }
inline void lampOff() { digitalWrite(RELAY_PIN, RELAY_ACTIVE_HIGH ? LOW  : HIGH); }

// ---------- Buzzer helpers ----------
inline void buzzerStart(uint32_t freq){ tone(BUZZER_PIN, freq); }
inline void buzzerStop(){ noTone(BUZZER_PIN); }

// ---------- RTOS queues ----------
QueueHandle_t evtQueue;   // EventType
QueueHandle_t cmdQueue;   // CmdType

enum EventType : uint8_t { EVT_SMOKE, EVT_TEST };
enum CmdType   : uint8_t { CMD_RESET, CMD_STATUS };

// Exposed status
volatile bool g_inCooldown = false;
volatile unsigned long g_cooldownUntil = 0;
volatile bool g_warmed = false;
volatile bool g_liveLog = false;  // toggle with 'L'

// ---------- Tasks ----------
void StatusTask(void*){
  // Optional heartbeat: blink lamp very briefly every 10s when idle (non-intrusive)
  const TickType_t beat = pdMS_TO_TICKS(10000);
  for(;;){
    if (!g_inCooldown && g_warmed){
      lampOn(); vTaskDelay(pdMS_TO_TICKS(40)); lampOff();
    }
    vTaskDelay(beat);
  }
}

void SensorTask(void*){
  pinMode(MQ2_DO, INPUT);
  const unsigned long tBoot = millis();

  bool lastLevel = false;
  unsigned long highSince = 0;

  for(;;){
    unsigned long now = millis();
    if (!g_warmed && now - tBoot >= WARMUP_MS) g_warmed = true;

    // Commands
    CmdType cmd;
    while (xQueueReceive(cmdQueue, &cmd, 0) == pdTRUE){
      if (cmd == CMD_RESET){
        g_inCooldown = false;
        g_cooldownUntil = 0;
        Serial.println(F("[CMD] Cooldown cleared."));
      } else if (cmd == CMD_STATUS){
        Serial.printf("[STATUS] warmed:%d cooldown:%d ms_left:%ld\n",
          g_warmed, g_inCooldown, g_inCooldown ? (long)(g_cooldownUntil - now) : 0L);
      }
    }

    // Exit cooldown
    if (g_inCooldown && now >= g_cooldownUntil){
      g_inCooldown = false;
      Serial.println(F("[INFO] Cooldown finished."));
    }

    // Read sensor after warmup and only if not in cooldown
    if (g_warmed && !g_inCooldown){
      bool level = digitalRead(MQ2_DO); // HIGH when smoke > threshold (module comparator)
      if (level){
        if (!lastLevel) highSince = now;
        if (now - highSince >= TRIP_HOLD_MS){
          EventType e = EVT_SMOKE;
          xQueueSend(evtQueue, &e, 0);
          g_inCooldown = true;
          g_cooldownUntil = now + COOLDOWN_MS;
          Serial.println(F("[ALARM] Smoke confirmed. Entering cooldown."));
        }
      } else {
        highSince = now;
      }
      lastLevel = level;
    }

    vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
  }
}

void AlarmTask(void*){
  for(;;){
    EventType e;
    if (xQueueReceive(evtQueue, &e, portMAX_DELAY) == pdTRUE){
      unsigned long tStart = millis();
      unsigned long duration = (e==EVT_TEST) ? 5000UL : ALARM_MS;

      Serial.printf("[ALARM] Start (%s) for %lu ms\n", e==EVT_TEST?"TEST":"SMOKE", duration);
      while (millis() - tStart < duration){
        // buzzer + lamp blink
        buzzerStart(3000);
        lampOn();
        vTaskDelay(pdMS_TO_TICKS(180));

        buzzerStop();
        lampOff();
        vTaskDelay(pdMS_TO_TICKS(120));
      }
      buzzerStop(); lampOff();
      Serial.println(F("[ALARM] Done."));
    }
  }
}

void SerialTask(void*){
  for(;;){
    while (Serial.available()){
      char c = toupper(Serial.read());
      if (c=='R'){ CmdType cmd=CMD_RESET;  xQueueSend(cmdQueue,&cmd,0); }
      else if (c=='S'){ CmdType cmd=CMD_STATUS; xQueueSend(cmdQueue,&cmd,0); }
      else if (c=='T'){ EventType e=EVT_TEST;   xQueueSend(evtQueue,&e,0); }
      else if (c=='L'){ g_liveLog = !g_liveLog; Serial.printf("[LOG] live=%d\n", g_liveLog); }
    }

    if (g_liveLog){
      int doState = digitalRead(MQ2_DO);  // 0=clean air, 1=above threshold
      Serial.printf("[LIVE] DO=%d  warmed=%d  cooldown=%d\n",
                    doState, g_warmed, g_inCooldown);
    }
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

// ---------- Setup/Loop ----------
void setup(){
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT); buzzerStop();

  pinMode(RELAY_PIN, OUTPUT);
  // Ensure lamp is OFF on boot
  lampOff();

  // Queues
  evtQueue = xQueueCreate(6, sizeof(EventType));
  cmdQueue = xQueueCreate(6, sizeof(CmdType));

  // Tasks
  xTaskCreatePinnedToCore(StatusTask, "Status", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(SensorTask, "Sensor", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(AlarmTask , "Alarm" , 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(SerialTask, "Serial", 2048, NULL, 2, NULL, 1);

  Serial.println(F("Ready. Commands: R=reset cooldown, T=test alarm, S=status, L=live log"));
}

void loop(){ /* all work in tasks */ }
