/*
 * Copyright 2026 Fathihzafran
 * Licensed under the Apache License, Version 2.0 (Apache 2.0)
 *
 * Project: Robobook Odometry & Control System - Final Stage 1
 * Component: Slave ESP32 (ADRC Controller)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// --- NETWORK CONFIGURATION ---
const char* ssid     = "PTIO ITB";    // Sesuaikan SSID
const char* password = "wific1t4";    // Sesuaikan Password
const int udpPort    = 4210;
WiFiUDP udp;

// --- HARDWARE PINOUT ---
const int PIN_ESC_R = 23; 
const int PIN_ESC_L = 13; 
const int HALL_R[] = {25, 26, 27}; 
const int HALL_L[] = {14, 32, 33};

// --- ADRC & PHYSICAL PARAMETERS ---
const float FF_R = 1200.0;    // PWM Floor (Right Motor)
const float FF_L = 1220.0;    // PWM Floor (Left Motor - Offset for Weight)
float b0 = 0.65; float wc = 0.5; float wo = 0.8; 
float Kyaw = 3.5; float accelRate = 0.12; 

// --- STATE VARIABLES ---
volatile float targetRPM = 0;
float actualTarget = 0, targetHeading = 0;
bool isFirstRun = true, bnoConnected = false;
int rotateMode = 0; // 0:Normal, 1:Pivot L, 2:Pivot R

// Dual ESO States for independent motor observation
float z1_r = 0, z2_r = 0, last_u_r = 0;
float z1_l = 0, z2_l = 0, last_u_l = 0;

volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
volatile int pR = 1000, pL = 1000;
volatile int stateR = -1, stateL = -1;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// 14-bit PWM Helper
int msToDuty(int ms) { return (ms * 16383) / 20000; }

void IRAM_ATTR isrRight() {
  int cur = (digitalRead(HALL_R[0]) << 2) | (digitalRead(HALL_R[1]) << 1) | digitalRead(HALL_R[2]);
  if (cur != stateR && cur > 0 && cur < 7) { pulseR++; stateR = cur; }
}
void IRAM_ATTR isrLeft() {
  int cur = (digitalRead(HALL_L[0]) << 2) | (digitalRead(HALL_L[1]) << 1) | digitalRead(HALL_L[2]);
  if (cur != stateL && cur > 0 && cur < 7) { pulseL++; stateL = cur; }
}

void ControlEngine(void * pvParameters) {
  unsigned long lastT = millis();
  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= 100) {
      float dt = (now - lastT) / 1000.0; lastT = now;
      noInterrupts(); long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0; interrupts();
      
      rpmR = (rpmR * 0.9) + (((cR / 90.0) * (60.0 / dt)) * 0.1);
      rpmL = (rpmL * 0.9) + (((cL / 90.0) * (60.0 / dt)) * 0.1);

      float curH = 0;
      if (bnoConnected) { sensors_event_t ev; bno.getEvent(&ev); curH = ev.orientation.x; }

      if (targetRPM > 1) {
        if (isFirstRun) { targetHeading = curH; actualTarget = 20.0; isFirstRun = false; }
        
        // Ramping to prevent overshoot
        if (actualTarget < targetRPM) actualTarget += accelRate; 
        else if (actualTarget > targetRPM) actualTarget -= 1.0;

        // ESO Update (Right & Left)
        float eObsR = rpmR - z1_r;
        z1_r += (z2_r + b0 * last_u_r + 2 * wo * eObsR) * dt; z2_r += (wo * wo * eObsR) * dt;
        float u_r = (wc * (actualTarget - z1_r) - z2_r) / b0; last_u_r = u_r;

        float eObsL = rpmL - z1_l;
        z1_l += (z2_l + b0 * last_u_l + 2 * wo * eObsL) * dt; z2_l += (wo * wo * eObsL) * dt;
        float u_l = (wc * (actualTarget - z1_l) - z2_l) / b0; last_u_l = u_l;

        float hErr = targetHeading - curH;
        if (hErr > 180) hErr -= 360; if (hErr < -180) hErr += 360;
        float yCorr = (rotateMode != 0) ? 0 : hErr * Kyaw;

        // Calculate Output
        int outR = (int)(FF_R + u_r - yCorr);
        int outL = (int)(FF_L + u_l + yCorr);

        // Anti-Jitter Filter: Jump to spin floor if in the stutter zone
        if (outR > 1005 && outR < FF_R) outR = (int)FF_R;
        if (outL > 1005 && outL < FF_L) outL = (int)FF_L;

        pR = constrain(outR, 1000, 1550); pL = constrain(outL, 1000, 1550);

        if (rotateMode == 1) { // Pivot Left
            ledcWrite(PIN_ESC_R, msToDuty(pR + 50)); ledcWrite(PIN_ESC_L, msToDuty(1000));
        } else if (rotateMode == 2) { // Pivot Right
            ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(pL + 50));
        } else { // Normal Drive
            ledcWrite(PIN_ESC_R, msToDuty(pR)); ledcWrite(PIN_ESC_L, msToDuty(pL));
        }
      } else {
        ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(1000));
        z1_r=0; z2_r=0; z1_l=0; z2_l=0; last_u_r=0; last_u_l=0; pR=1000; pL=1000;
        actualTarget = 0; isFirstRun = true; rotateMode = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200); WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  udp.begin(udpPort);

  ledcAttach(PIN_ESC_R, 50, 14); ledcAttach(PIN_ESC_L, 50, 14);
  ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(1000));

  Wire.begin(21, 22); if(bno.begin()) { bnoConnected = true; bno.setExtCrystalUse(true); }

  for(int i=0; i<3; i++) {
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrRight, CHANGE);
    pinMode(HALL_L[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]), isrLeft, CHANGE);
  }
  delay(2000); 
  xTaskCreatePinnedToCore(ControlEngine, "ADRC_Core", 8192, NULL, 3, NULL, 1);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buf[255]; int len = udp.read(buf, 255); if (len > 0) buf[len] = 0;
    String cmd = String(buf); cmd.trim();
    if (cmd == "FORWARD")      { targetRPM = 32; rotateMode = 0; }
    else if (cmd == "BOOST")    { targetRPM = 45; rotateMode = 0; }
    else if (cmd == "STOP")     { targetRPM = 0;  rotateMode = 0; }
    else if (cmd == "LEFT")     { targetHeading -= 20; }
    else if (cmd == "RIGHT")    { targetHeading += 20; }
    else if (cmd == "ROTATE_E") { targetRPM = 28; rotateMode = 1; }
    else if (cmd == "ROTATE_Q") { targetRPM = 28; rotateMode = 2; }
    if(targetHeading < 0) targetHeading += 360;
    if(targetHeading >= 360) targetHeading -= 360;
  }
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 150 && udp.remoteIP()) {
    lastSent = millis(); udp.beginPacket(udp.remoteIP(), udp.remotePort());
    sensors_event_t ev; bno.getEvent(&ev);
    // Packet: RPM_R, RPM_L, Disturbance_R, Disturbance_L, PWM_R, PWM_L, Heading
    udp.printf("%.1f,%.1f,%.1f,%.1f,%d,%d,%.1f", rpmR, rpmL, z2_r, z2_l, pR, pL, ev.orientation.x);
    udp.endPacket();
  }
}
