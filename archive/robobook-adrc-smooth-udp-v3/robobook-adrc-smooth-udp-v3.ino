/*
 * Copyright 2026 Fathihzafran
 * Licensed under the Apache License, Version 2.0 (Apache 2.0)
 *
 * Project: Robobook Odometry & Control System
 * Component: Smooth ADRC & Anti-Jitter Filter (Phase 5.5)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// --- NETWORK CONFIGURATION (Adjust for your environment) ---
const char* ssid     = "YOUR_SSID"; 
const char* password = "YOUR_PASSWORD";
WiFiUDP udp;
const int udpPort = 4210;

// --- HARDWARE PINOUT ---
const int PIN_ESC_R = 23; 
const int PIN_ESC_L = 13; 
const int HALL_R[] = {25, 26, 27}; 
const int HALL_L[] = {14, 32, 33};

// --- FINE-TUNED ADRC PARAMETERS ---
float b0 = 0.65; 
float wc = 0.5; 
float wo = 0.8;          // Lowered to prevent jitter
float Kyaw = 4.0; 
float Ksync = 3.0; 
float K_FF = 0.45;
float accelRate = 0.15;  // Anti-overshoot ramping rate

// --- STATE VARIABLES ---
volatile float targetRPM = 0;
float actualTarget = 0, targetHeading = 0;
float z1 = 0, z2 = 0, last_u = 0;
bool isFirstRun = true, bnoConnected = false;
int rotateMode = 0; // 0:Normal, 1:Pivot L, 2:Pivot R

volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
volatile int stateR = -1, stateL = -1;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// 14-bit PWM Conversion Helper
int msToDuty(int ms) { return (ms * 16383) / 20000; }

// --- INTERRUPT HANDLERS ---
void IRAM_ATTR isrRight() {
  int cur = (digitalRead(HALL_R[0]) << 2) | (digitalRead(HALL_R[1]) << 1) | digitalRead(HALL_R[2]);
  if (cur != stateR && cur > 0 && cur < 7) { pulseR++; stateR = cur; }
}
void IRAM_ATTR isrLeft() {
  int cur = (digitalRead(HALL_L[0]) << 2) | (digitalRead(HALL_L[1]) << 1) | digitalRead(HALL_L[2]);
  if (cur != stateL && cur > 0 && cur < 7) { pulseL++; stateL = cur; }
}

// --- CORE 1: ADRC ENGINE ---
void ControlEngine(void * pvParameters) {
  unsigned long lastT = millis();
  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= 100) {
      float dt = (now - lastT) / 1000.0; lastT = now;
      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();
      
      rpmR = (rpmR * 0.9) + (((cR / 90.0) * (60.0 / dt)) * 0.1);
      rpmL = (rpmL * 0.9) + (((cL / 90.0) * (60.0 / dt)) * 0.1);

      float curH = 0;
      if (bnoConnected) { sensors_event_t ev; bno.getEvent(&ev); curH = ev.orientation.x; }

      if (targetRPM > 1) {
        if (isFirstRun) { targetHeading = curH; actualTarget = 20.0; isFirstRun = false; }
        
        // --- STEPPED ACCELERATION LOGIC ---
        if (actualTarget < targetRPM) actualTarget += accelRate; 
        else if (actualTarget > targetRPM) actualTarget -= 0.5;

        // ADRC Extended State Observer
        float err_obs = rpmR - z1;
        z1 += (z2 + b0 * last_u + 2 * wo * err_obs) * dt; 
        z2 += (wo * wo * err_obs) * dt;
        float ctrl = (wc * (actualTarget - z1) - z2) / b0; last_u = ctrl;

        float yCorr = (rotateMode != 0) ? 0 : (targetHeading - curH) * Kyaw;
        float sCorr = (rotateMode != 0) ? 0 : (rpmR - rpmL) * Ksync;
        
        int basePWM = 1055 + (int)(actualTarget * K_FF); 
        int pR = basePWM + (int)(ctrl - yCorr - sCorr);
        int pL = basePWM + (int)(ctrl + yCorr + sCorr);

        // --- ANTI-JITTER FILTER ---
        // Forces instantaneous torque if signal is near the stall threshold
        if (pR > 1005 && pR < 1065) pR = 1070;
        if (pL > 1005 && pL < 1065) pL = 1070;

        if (rotateMode == 1) { 
          ledcWrite(PIN_ESC_R, msToDuty(pR + 40)); 
          ledcWrite(PIN_ESC_L, msToDuty(1000)); 
        } else if (rotateMode == 2) { 
          ledcWrite(PIN_ESC_R, msToDuty(1000)); 
          ledcWrite(PIN_ESC_L, msToDuty(pL + 40)); 
        } else { 
          ledcWrite(PIN_ESC_R, msToDuty(constrain(pR, 1000, 1550))); 
          ledcWrite(PIN_ESC_L, msToDuty(constrain(pL, 1000, 1550))); 
        }
      } else {
        ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(1000));
        z1 = 0; z2 = 0; last_u = 0; isFirstRun = true; actualTarget = 0; rotateMode = 0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup() {
  Serial.begin(115200); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  udp.begin(udpPort);

  ledcAttach(PIN_ESC_R, 50, 14); ledcAttach(PIN_ESC_L, 50, 14);
  ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(1000));

  Wire.begin(21, 22); if(bno.begin()) bnoConnected = true;

  for(int i=0; i<3; i++) {
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrRight, CHANGE);
    pinMode(HALL_L[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]), isrLeft, CHANGE);
  }
  delay(2000); 
  xTaskCreatePinnedToCore(ControlEngine, "ADRC_Smooth", 8192, NULL, 3, NULL, 1);
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
  if (millis() - lastSent > 200 && udp.remoteIP()) {
    lastSent = millis();
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    sensors_event_t ev; if(bnoConnected) bno.getEvent(&ev);
    udp.printf("%.1f,%.1f,%.1f,%.1f", rpmR, rpmL, z2, ev.orientation.x);
    udp.endPacket();
  }
}
