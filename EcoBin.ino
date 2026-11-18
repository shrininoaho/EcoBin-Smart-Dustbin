
#define BLYNK_TEMPLATE_ID "TMPL3Id3LbJl8"
#define BLYNK_TEMPLATE_NAME "EcoBin"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ---------------- Wi-Fi ----------------
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------------- Pin Configuration ----------------
#define TRIG_PIN 4
#define ECHO_PIN 2
#define IR_PIN 13
#define SERVO_PIN 14

// ---------------- Settings ----------------
#define SERVO_CLOSED_ANGLE 0
#define SERVO_OPEN_ANGLE 90

#define LID_OPEN_DURATION 3000       // 3 seconds
#define FULL_DISTANCE_CM 8           // Adjust based on bin depth
#define SENSOR_INTERVAL 500          // Read sensor every 500 ms

// Most IR obstacle/gesture modules output LOW when triggered.
// Change to HIGH if your particular sensor works the opposite way.
#define IR_ACTIVE_STATE LOW

// ---------------- Objects ----------------
Servo lidServo;
BlynkTimer timer;

// ---------------- Variables ----------------
bool lidOpen = false;
bool manualOverride = false;
bool binFull = false;

unsigned long lidOpenUntil = 0;

// =================================================
// LID CONTROL
// =================================================

void openLid() {
  if (!lidOpen) {
    lidServo.write(SERVO_OPEN_ANGLE);
    lidOpen = true;
    Serial.println("Lid opened");
  }
}

void closeLid() {
  if (lidOpen) {
    lidServo.write(SERVO_CLOSED_ANGLE);
    lidOpen = false;
    Serial.println("Lid closed");
  }
}

// =================================================
// BLYNK MANUAL LID OVERRIDE - V5
// =================================================

BLYNK_WRITE(V5) {
  int switchState = param.asInt();

  manualOverride = (switchState == 1);

  if (manualOverride) {
    openLid();
    Serial.println("Manual override: ON");
  } else {
    Serial.println("Manual override: OFF");
    lidOpenUntil = 0;
  }
}

// =================================================
// ULTRASONIC SENSOR
// =================================================

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    return -1;  // No valid echo
  }

  float distance = duration * 0.0343 / 2.0;

  return distance;
}

// =================================================
// FILL LEVEL MONITORING - V1 AND V0
// =================================================

void monitorBin() {

  float distance = getDistance();

  if (distance < 0) {
    Serial.println("Ultrasonic sensor: No echo received");
    return;
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Display distance on Blynk V1
  Blynk.virtualWrite(V1, distance);

  // Detect whether bin is full
  binFull = (distance <= FULL_DISTANCE_CM);

  // Update Blynk LED on V0
  Blynk.virtualWrite(V0, binFull ? 255 : 0);

  if (binFull) {
    Serial.println("WARNING: BIN FULL - lid locked out");
  }
}

// =================================================
// IR GESTURE DETECTION AND AUTOMATIC LID
// =================================================

void detectGesture() {

  if (manualOverride) {
    return;
  }

  // Lockout mode: when the bin is full, keep the lid closed
  // so no more waste is added until it is emptied.
  if (binFull) {
    if (lidOpen) {
      closeLid();
      lidOpenUntil = 0;
    }
    return;
  }

  int irState = digitalRead(IR_PIN);

  if (irState == IR_ACTIVE_STATE) {

    openLid();

    // Keep lid open for the configured duration
    lidOpenUntil = millis() + LID_OPEN_DURATION;
  }

  // Close lid after the opening duration
  if (lidOpen && lidOpenUntil != 0 &&
      (long)(millis() - lidOpenUntil) >= 0) {

    closeLid();
    lidOpenUntil = 0;
  }
}

// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  // Configure sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // Initialize servo
  lidServo.setPeriodHertz(50);
  lidServo.attach(SERVO_PIN, 500, 2400);

  closeLid();

  Serial.println();
  Serial.println("================================");
  Serial.println("       ECOBIN INITIALIZED       ");
  Serial.println("================================");

  // Connect to Blynk
  Serial.println("Connecting to Blynk...");

  Blynk.begin(
    BLYNK_AUTH_TOKEN,
    ssid,
    pass
  );

  Serial.println("Blynk connected!");

  // Schedule ultrasonic monitoring
  timer.setInterval(SENSOR_INTERVAL, monitorBin);
}

// =================================================
// MAIN LOOP
// =================================================

void loop() {

  Blynk.run();
  timer.run();

  detectGesture();
}
