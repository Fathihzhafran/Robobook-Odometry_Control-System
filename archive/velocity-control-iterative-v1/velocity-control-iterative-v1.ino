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
 * Component: Velocity Control (Iterative Phase 1)
 */

#include <ESP32Servo.h>

// --- PIN CONFIGURATION ---
const int PIN_ESC_R = 23; 
const int PIN_ESC_L = 12; 
const int HALL_R[] = {25, 26, 27};
const int HALL_L[] = {14, 32, 33};

// --- SYSTEM CONSTANTS ---
const int PULSES_PER_REV = 90; 
const unsigned long INTERVAL = 120; // Sampling rate optimized for stability
const float PWM_MIN_IDLE = 1055; 

// --- VOLATILE SHARED VARIABLES ---
volatile float targetRPM = 0;
volatile float currentRPM_R = 0, currentRPM_L = 0;
volatile long pulseCountR = 0, pulseCountL = 0;
volatile int hallStateR = -1, hallStateL = -1;

// --- TUNING PARAMETERS (Consolidated V1.5) ---
float K_FF  = 0.38; // Feed-Forward Baseline
float Kp    = 0.40; // Proportional Gain
float Ki    = 1.20; // Integral Gain
float Kd    = 0.45; // Derivative Damping
float Ksync = 3.50; // Synchronization Gain

Servo motorR, motorL;

// --- INTERRUPT HANDLERS ---
void IRAM_ATTR isrRight() {
  int cur = (digitalRead(HALL_R[0]) << 2) | (digitalRead(HALL_R[1]) << 1) | digitalRead(HALL_R[2]);
  if (cur != hallStateR && cur > 0 && cur < 7) { pulseCountR++; hallStateR = cur; }
}
void IRAM_ATTR isrLeft() {
  int cur = (digitalRead(HALL_LEFT[0]) << 2) | (digitalRead(HALL_LEFT[1]) << 1) | digitalRead(HALL_LEFT[2]);
  if (cur != hallStateL && cur > 0 && cur < 7) { pulseCountL++; hallStateL = cur; }
}

// --- CORE 0: CONTROL ENGINE ---
void ControlEngine(void * pvParameters) {
  float integralR = 0, integralL = 0;
  float prevRPM_R = 0, prevRPM_L = 0;
  float filteredR = 0, filteredL = 0;
  unsigned long lastCycle = millis();

  for(;;) {
    unsigned long now = millis();
    if (now - lastCycle >= INTERVAL) {
      float dt = (now - lastCycle) / 1000.0; lastCycle = now;

      noInterrupts();
      long cR = pulseCountR; pulseCountR = 0;
      long cL = pulseCountL; pulseCountL = 0;
      interrupts();

      // Calculation with 0.9 Low-Pass Filtering
      float rawR = (cR / (float)PULSES_PER_REV) * (60.0 / dt);
      float rawL = (cL / (float)PULSES_PER_REV) * (60.0 / dt);
      filteredR = (filteredR * 0.9) + (rawR * 0.1);
      filteredL = (filteredL * 0.9) + (rawL * 0.1);
      currentRPM_R = filteredR; currentRPM_L = filteredL;

      if (targetRPM > 0) {
        float errR = targetRPM - currentRPM_R;
        float errL = targetRPM - currentRPM_L;

        // Anti-Windup Integral
        integralR = constrain(integralR + (errR * dt), -45, 45);
        integralL = constrain(integralL + (errL * dt), -45, 45);

        // Derivative Calculations
        float dR = (currentRPM_R - prevRPM_R) / dt;
        float dL = (currentRPM_L - prevRPM_L) / dt;
        prevRPM_R = currentRPM_R; prevRPM_L = currentRPM_L;

        // Control Signal Fusion
        float basePWM = PWM_MIN_IDLE + (targetRPM * K_FF);
        float sync = (currentRPM_R - currentRPM_L) * Ksync;

        float outR = basePWM + (errR * Kp) + (integralR * Ki) - (dR * Kd) - sync;
        float outL = basePower + (errL * Kp) + (integralL * Ki) - (dL * Kd) + sync;

        motorR.writeMicroseconds((int)constrain(outR, 1000, 1450));
        motorL.writeMicroseconds((int)constrain(outL, 1000, 1450));
      } else {
        motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
        integralR = 0; integralL = 0; prevRPM_R = 0; prevRPM_L = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);

  for(int i=0; i<3; i++) {
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrRight, CHANGE);
    pinMode(HALL_L[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]), isrLeft, CHANGE);
  }

  motorR.attach(PIN_ESC_R, 1000, 2000); motorL.attach(PIN_ESC_L, 1000, 2000);
  motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
  delay(5000);
  xTaskCreatePinnedToCore(ControlEngine, "Control", 8192, NULL, 3, NULL, 0);
}

void loop() {
  if (Serial.available() > 0) targetRPM = Serial.readStringUntil('\n').toFloat();
  Serial.printf("%.1f,%.1f,%.1f\n", targetRPM, currentRPM_R, currentRPM_L);
  delay(50);
}
