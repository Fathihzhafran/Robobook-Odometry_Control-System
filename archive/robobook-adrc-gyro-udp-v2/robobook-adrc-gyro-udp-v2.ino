/*
 * Copyright 2026 Fathihzafran
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project: Robobook Odometry & Control System
 * Component: Advanced Velocity Tuning (Phase 2 - Bench Test)
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// --- NETWORK CONFIG ---
const char* ssid     = "YOUR_SSID"; 
const char* password = "YOUR_PASSWORD";
WiFiUDP udp;
const int udpPort = 4210;

// --- HARDWARE CONFIG ---
const int PIN_ESC_R = 23; 
const int PIN_ESC_L = 13;
const int HALL_R[] = {25, 26, 27}; 
const int HALL_L[] = {14, 32, 33};

// --- ADRC PARAMETERS ---
float b0 = 0.65; float wc = 0.8; float wo = 1.2;
float Kyaw = 5.5; float Ksync = 3.5; float K_FF = 0.45;
float minStartRPM = 28.0;

// --- STATE VARIABLES ---
volatile float targetRPM = 0;
float actualTarget = 0, targetHeading = 0;
bool isFirstRun = true, bnoConnected = false;
int rotateMode = 0; // 0:Normal, 1:Pivot L, 2:Pivot R

volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
float z1 = 0, z2 = 0, last_u = 0;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// 14-bit PWM Helper
int msToDuty(int ms) { return (ms * 16383) / 20000; }

void IRAM_ATTR isrR() { pulseR++; }
void IRAM_ATTR isrL() { pulseL++; }

// --- CONTROL TASK (CORE 1) ---
void ControlEngine(void * pvParameters) {
  unsigned long lastTime = millis();
  for(;;) {
    unsigned long now = millis();
    if (now - lastTime >= 100) {
      float dt = (now - lastTime) / 1000.0; lastTime = now;

      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();

      rpmR = (rpmR * 0.9) + (((cR / 90.0) * (60.0 / dt)) * 0.1);
      rpmL = (rpmL * 0.9) + (((cL / 90.0) * (60.0 / dt)) * 0.1);

      float curH = 0;
      if (bnoConnected) { sensors_event_t ev; bno.getEvent(&ev); curH = ev.orientation.x; }

      if (targetRPM > 1) {
        if (isFirstRun) { targetHeading = curH; actualTarget = minStartRPM; isFirstRun = false; }
        
        if (actualTarget < targetRPM) actualTarget += 0.4;
        else if (actualTarget > targetRPM) actualTarget -= 1.0;

        // --- ADRC ESO ALGORITHM ---
        float err_obs = rpmR - z1;
        z1 += (z2 + b0 * last_u + 2 * wo * err_obs) * dt;
        z2 += (wo * wo * err_obs) * dt;
        float ctrl = (wc * (actualTarget - z1) - z2) / b0;
        last_u = ctrl;

        // Corrections (only in Normal mode)
        float yCorr = (rotateMode != 0) ? 0 : (targetHeading - curH) * Kyaw;
        float sCorr = (rotateMode != 0) ? 0 : (rpmR - rpmL) * Ksync;

        int basePWM = 1125 + (int)(actualTarget * K_FF);

        if (rotateMode == 1) { // Pivot Left
            ledcWrite(PIN_ESC_R, msToDuty(constrain(basePWM + 60, 1000, 1400)));
            ledcWrite(PIN_ESC_L, msToDuty(1000));
        } else if (rotateMode == 2) { // Pivot Right
            ledcWrite(PIN_ESC_R, msToDuty(1000));
            ledcWrite(PIN_ESC_L, msToDuty(constrain(basePWM + 60, 1000, 1400)));
        } else { // Normal Straight
            ledcWrite(PIN_ESC_R, msToDuty(constrain(basePWM + ctrl - yCorr - sCorr, 1000, 1550)));
            ledcWrite(PIN_ESC_L, msToDuty(constrain(basePWM + ctrl + yCorr + sCorr, 1000, 1550)));
        }
      } else {
        ledcWrite(PIN_ESC_R, msToDuty(1000)); ledcWrite(PIN_ESC_L, msToDuty(1000));
        z1 = 0; z2 = 0; last_u = 0; isFirstRun = true; actualTarget = 0; rotateMode = 0;
      }
    }
    vTaskDelay(1);
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
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrR, CHANGE);
    pinMode(HALL_L[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]), isrL, CHANGE);
  }
  delay(2000); 
  xTaskCreatePinnedToCore(ControlEngine, "ADRC_Engine", 8192, NULL, 3, NULL, 1);
}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buf[255]; int len = udp.read(buf, 255); if (len > 0) buf[len] = 0;
    String cmd = String(buf); cmd.trim();

    if (cmd == "FORWARD")      { targetRPM = 32; rotateMode = 0; }
    else if (cmd == "BOOST")    { targetRPM = 50; rotateMode = 0; }
    else if (cmd == "STOP")     { targetRPM = 0;  rotateMode = 0; }
    else if (cmd == "LEFT")     { targetHeading -= 20; }
    else if (cmd == "RIGHT")    { targetHeading += 20; }
    else if (cmd == "ROTATE_E") { targetRPM = 28; rotateMode = 1; }
    else if (cmd == "ROTATE_Q") { targetRPM = 28; rotateMode = 2; }
  }

  // Wireless Telemetry
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 200 && udp.remoteIP()) {
    lastSent = millis();
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    sensors_event_t ev; if(bnoConnected) bno.getEvent(&ev);
    udp.printf("%.1f,%.1f,%.1f,%.1f", rpmR, rpmL, z2, ev.orientation.x);
    udp.endPacket();
  }
}
