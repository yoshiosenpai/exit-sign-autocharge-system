# Exit Sign Safety System (ESP32 + Buzzer + LED)

This project demonstrates two safety demos using **ROBO ESP32** with built-in LiPo auto-charge, sensors, and Blynk IoT Web Dashboard.

- **Demo 2: Smoke Alarm System**  
  Uses an **MQ-2 smoke sensor**. If smoke is detected, the ESP32 triggers its **buzzer** and **LED**.  
  After an alarm, the system enters a **1-hour cooldown** to avoid repeated triggers.  
  Includes serial commands for testing and manual control.
---

## Hardware Used
- **ROBO ESP32** (Cytron) – with built-in LiPo auto-charge, buzzer and Neopixel LED.
- **MQ-2 Gas/Smoke Sensor** (digital pin used)
- **LiPo Battery 3.7 V** – for backup power
- **Emergency EXIT Sign** (Candlelux CKS-38J)
- **5 V Micro-USB Adapter** – powers and charges ROBO ESP32

---

## Software & Libraries

### Arduino IDE Setup
1. Install **Arduino IDE 2.x** or later
2. Add ESP32 board package:  
   - File → Preferences → Additional Boards Manager URLs:  
     ```
     https://dl.espressif.com/dl/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager → Search “ESP32 by Espressif Systems” → Install
3. Select board: **ESP32 Dev Module**

### Libraries (install via Library Manager)
- [**Adafruit NeoPixel**](https://github.com/adafruit/Adafruit_NeoPixel)

---

## 🔥 Demo 2: Smoke Alarm System

### How It Works
- MQ-2 DO pin goes HIGH when smoke exceeds threshold (set with onboard trimpot).
- ESP32 confirms smoke is real (debounce).
- Alarm triggers:
  - Buzzer ON + LED Blinking
- After alarm → **1-hour cooldown** (ignores further smoke)
- Serial commands allow test alarm, reset, and status check.

### Wiring
- MQ-2 → ROBO ESP32
  - VCC → 5 V
  - GND → GND
  - DO → GPIO27
- Buzzer → built-in on ROBO (GPIO23)
- LiPo + 5 V power as before


### Step-by-Step
4. Upload Demo 2 code
5. Open Serial Monitor:
   - `S` = show status
   - `T` = test alarm (manual)
   - `R` = reset cooldown
   - `L` = toggle live log (see DO state)
6. Expose sensor to smoke/aerosol → Alarm triggers 

---

## 📝 Notes
- Both demos use **ROBO ESP32’s LiPo + charger** → board survives AC outage.  
- Message limit: Blynk free tier = ~30k/month. Demo 1 is throttled to send every 2 s only when live mode ON (press `L`), else silent.  
- For competition demo:  
  - Show Serial Monitor to explain calibration and commands  
  - Show Blynk Web Dashboard as remote monitoring UI  

---

## 👨‍💻 Author
Code and documentation by **Solehin Rizal**
