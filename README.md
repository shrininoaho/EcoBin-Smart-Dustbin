# EcoBin – IoT Smart Dustbin
ESP32-based smart dustbin with gesture-activated lid, fill-level sensing, full-bin lockout and Blynk IoT dashboard.
Published in JIDMIS (Scopus-indexed).

## Hardware
ESP32, HC-SR04 ultrasonic sensor, IR gesture sensor, servo motor

## Pins
Trig 4 | Echo 2 | IR 13 | Servo 14

## Blynk setup
- V0: LED (bin full)
- V1: Value display (distance, cm)
- V5: Switch (manual lid override)

## Run
1. Open EcoBin.ino in Arduino IDE
2. Install ESP32Servo and Blynk libraries
3. Set your Wi-Fi name, password and Blynk auth token
4. Upload to the ESP32
