# Exit Sign Safety System (ESP32 + Blynk IoT)

This project demonstrates two safety demos using **ROBO ESP32** with built-in LiPo auto-charge, sensors, and Blynk IoT Web Dashboard.

- **Demo 1: Exit Sign Health Monitor**  
  Uses an **APDS9960 sensor** to measure the light output of an emergency EXIT sign.  
  If the sign is glowing → considered **FUNCTION**.  
  If no light is detected → considered **NOT FUNCTION (fault/broken)**.  
  Status is sent to **Blynk IoT** for monitoring.
---

## Hardware Used
- **ROBO ESP32** (Cytron) – with built-in LiPo auto-charge and buzzer
- **APDS9960 sensor** (I²C) – ambient light detection for EXIT sign
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
- [**Blynk**](https://github.com/blynkkk/blynk-library) (`<BlynkSimpleEsp32.h>`)
- [**Adafruit APDS9960**](https://github.com/adafruit/Adafruit_APDS9960) (for Demo 1)

---

## ⚡ Demo 1: Exit Sign Health Monitor

### How It Works
- APDS9960 continuously measures ambient brightness (Clear channel `C`).
- Moving average + hysteresis thresholds decide if EXIT sign is **FUNCTION** (lit) or **NOT FUNCTION** (dark).
- Status is pushed to **Blynk IoT** every 2 s when live mode is ON.
- On Web Dashboard:
  - **V0** = raw light value (0–4095)
  - **V1** = LED indicator (Green = Function, Red = Not Function)

### Wiring
- APDS9960 → ROBO ESP32 Grove I²C port (SDA=21, SCL=22, 3V3, GND)
- ROBO ESP32 → Micro-USB power + LiPo battery (optional)

### Step-by-Step
1. Open **Blynk Console** → Developer Zone → Open Project **Safety System Exit Sign**
2. Copy Template ID, Name, and Device Auth Token → paste into Arduino sketch
3. Upload code to ESP32
4. Open Serial Monitor @ 115200
5. Start live sending → type `L` at Serial monitor 
6. Check Dashboard → V0 shows light value, V1 LED turns Green (Function) or Red (Not Function)

---

## 📝 Notes
- Both demos use **ROBO ESP32’s LiPo + charger** → board survives AC outage.  
- Message limit: Blynk free tier = ~30k/month. Demo 1 is throttled to send every 2 s only when live mode ON (press `L`), else silent.  
- For demo:  
  - Show Serial Monitor to explain calibration and commands  
  - Show Blynk Web Dashboard as remote monitoring UI  

---

## 👨‍💻 Author
Code and documentation by **Solehin Rizal**
