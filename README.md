# 🚨 Exit Sign Safety System (ESP32 + Blynk IoT)

This repository contains two demo projects using **ROBO ESP32** with built-in LiPo auto-charge, sensors, and **Blynk IoT Web Dashboard** for monitoring.

- **Demo 1: Exit Sign Health Monitor**  
  Uses **APDS9960** light sensor to check whether the EXIT sign is glowing.  
  If light is detected → **FUNCTION** ✅, else → **NOT FUNCTION** ❌.  
  Status is pushed to **Blynk Web Dashboard** for remote monitoring.

- **Demo 2: Smoke Alarm System**  
  Uses **MQ-2 smoke sensor**. When smoke is detected, the ESP32 triggers the onboard **buzzer** and **LED**.  
  System enters **1-hour cooldown** after an alarm to avoid repeated triggers.

---

## 🛠 Hardware
- ROBO ESP32 (Cytron)
- APDS9960 Light/Color Sensor (I²C)  
- MQ-2 Smoke Sensor (Digital pin)  
- LiPo Battery 3.7 V (backup)  
- Emergency EXIT Sign (Candlelux CKS-38J)  
- 5 V Micro-USB Adapter  

---

## 📦 Software
- Arduino IDE + **ESP32 board support**  
- Libraries:  
  - [Blynk](https://github.com/blynkkk/blynk-library)  
  - [Adafruit APDS9960](https://github.com/adafruit/Adafruit_APDS9960)  
  - [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) *(optional)*  

---

## 🚀 Quick Start
1. Clone this repo  
2. Install required libraries in Arduino IDE  
3. Open **Demo1.ino** or **Demo2.ino**  
4. Fill in your **WiFi** + **Blynk credentials**  
5. Upload to ROBO ESP32  
6. Monitor status via **Serial Monitor** and **Blynk Web Dashboard**  

---

## 👨‍💻 Author
Code & documentation by **Solehin Rizal**
