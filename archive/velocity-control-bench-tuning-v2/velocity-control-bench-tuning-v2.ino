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

#include <ESP32Servo.h>

// --- PINOUT ---
const int PIN_ESC_R = 23; 
const int PIN_ESC_L = 12; 
const int HALL_R[] = {25, 26, 27};
const int HALL_L[] = {14, 32, 33};

// --- SYSTEM SETTINGS ---
const int PULSES_PER_REV = 90; 
const unsigned long INTERVAL = 120; // Stable mid-range interval
const float PWM_MIN_RUN = 1055;

// --- TUNING VARIABLES (Volatile for Serial Tuning) ---
volatile float targetRPM = 0;
volatile float actualTarget = 0; // For Ramping
volatile float Kp = 0.20; float Ki = 0.50; float Kd = 1.00;
volatile float Ksync = 6.0; volatile float K_FF = 0.38; 

// --- SHARED DATA ---
volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
volatile int stateR = -1, stateL = -1;
Servo motorR, motorL;

void IRAM_ATTR isrR() {
  int cur = (digitalRead(HALL_R[0]) << 2) | (digitalRead(HALL_R[1]) << 1) | digitalRead(HALL_R[2]);
  if (cur != stateR && cur > 0 && cur < 7) { pulseR++; stateR = cur; }
}
void IRAM_ATTR isrL() {
  int cur = (digitalRead(HALL_L[0]) << 2) | (digitalRead(HALL_L[1]) << 1) | digitalRead(HALL_L[2]);
  if (cur != stateL && cur > 0 && cur < 7) { pulseL++; stateL = cur; }
}

// --- CORE 0: CONTROL ENGINE ---
void ControlEngine(void * pvParameters) {
  float iR = 0, iL = 0;
  float sR = 0, sL = 0;
  unsigned long lastT = millis();

  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= INTERVAL) {
      float dt = (now - lastT) / 1000.0; lastT = now;

      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();

      float rawR = (cR / 90.0) * (60.0 / dt);
      float rawL = (cL / 90.0) * (60.0 / dt);

      // Extreme Smoothing (0.95) for Telemetry Clarity
      sR = (sR * 0.95) + (rawR * 0.05);
      sL = (sL * 0.95) + (rawL * 0.05);
      rpmR = sR; rpmL = sL;

      // Target Ramping (Soft Start)
      if (actualTarget < targetRPM) actualTarget += 1.0;
      else if (actualTarget > targetRPM) actualTarget -= 2.0;
      if (abs(actualTarget - targetRPM) < 1.0) actualTarget = targetRPM;

      if (targetRPM > 0) {
        float errR = actualTarget - rpmR;
        float errL = actualTarget - rpmL;

        // Deadzone Feature (1.5 RPM)
        if (abs(errR) < 1.5) errR = 0; if (abs(errL) < 1.5) errL = 0;

        iR = constrain(iR + (errR * dt), -30, 30);
        iL = constrain(iL + (errL * dt), -30, 30);

        float basePWM = PWM_MIN_RUN + (actualTarget * K_FF);
        float sync = (rpmR - rpmL) * Ksync;

        float outR = basePWM + (errR * Kp) + (iR * Ki) - sync;
        float outL = basePWM + (errL * Kp) + (iL * Ki) + sync;

        motorR.writeMicroseconds((int)constrain(outR, 1000, 1450));
        motorL.writeMicroseconds((int)constrain(outL, 1000, 1450));
      } else {
        motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
        iR = 0; iL = 0; actualTarget = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  for(int i=0; i<3; i++) {
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrR, CHANGE);
    pinMode(HALL_L[i],  INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]),  isrL,  CHANGE);
  }
  motorR.attach(PIN_ESC_R, 1000, 2000); motorL.attach(PIN_ESC_L, 1000, 2000);
  motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
  delay(5000);
  xTaskCreatePinnedToCore(ControlEngine, "Control", 8192, NULL, 3, NULL, 0);
  Serial.println(">>> ROBOBOOK V2 READY: TUNING MODE ENABLED <<<");
}

void loop() {
  if (Serial.available() > 0) {
    String in = Serial.readStringUntil('\n'); in.trim();
    if (in.length() > 1) {
      char cmd = in.charAt(0); float val = in.substring(1).toFloat();
      if (cmd == 'P') Kp = val; else if (cmd == 'I') Ki = val;
      else if (cmd == 'D') Kd = val; else if (cmd == 'T') targetRPM = val;
      else if (cmd == 'S') Ksync = val; else if (cmd == 'F') K_FF = val;
      Serial.printf(">>> SET: P:%.2f I:%.2f D:%.2f T:%.1f S:%.1f\n", Kp, Ki, Kd, targetRPM, Ksync);
    }
  }
  Serial.printf("%.1f,%.1f,%.1f\n", targetRPM, rpmR, rpmL);
  delay(100);
}
