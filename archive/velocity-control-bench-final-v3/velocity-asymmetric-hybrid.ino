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

const int PIN_ESC_R = 23; const int PIN_ESC_L = 12;
const unsigned long INTERVAL = 200; 
float targetRPM = 0, actualTarget = 0;
float Kp = 0.15, Ki = 0.30, Ksync = 5.0; 
float FF_R = 1085, FF_L = 1105; // Compensation for heavier motor

volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
Servo motorR, motorL;

void IRAM_ATTR isrR() { pulseR++; }
void IRAM_ATTR isrL() { pulseL++; }

void ControlEngine(void * pvParameters) {
  float iR = 0, iSync = 0;
  unsigned long lastT = millis();

  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= INTERVAL) {
      float dt = (now - lastT) / 1000.0; lastT = now;
      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();

      rpmR = (rpmR * 0.8) + (((cR / 90.0) * (60.0 / dt)) * 0.2);
      rpmL = (rpmL * 0.8) + (((cL / 90.0) * (60.0 / dt)) * 0.2);

      if (actualTarget < targetRPM) actualTarget += 1.0;
      else if (actualTarget > targetRPM) actualTarget -= 2.0;

      if (targetRPM > 0) {
        iR = constrain(iR + (actualTarget - rpmR), -50, 50);
        iSync = constrain(iSync + (rpmR - rpmL), -30, 30);

        float outR = FF_R + ((actualTarget - rpmR) * Kp) + (iR * Ki);
        float outL = outR + (FF_L - FF_R) + ((rpmR - rpmL) * Ksync); 

        motorR.writeMicroseconds((int)constrain(outR, 1030, 1350));
        motorL.writeMicroseconds((int)constrain(outL, 1030, 1350));
      } else {
        motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
        iR = 0; iSync = 0; actualTarget = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  motorR.attach(PIN_ESC_R, 1000, 2000); motorL.attach(PIN_ESC_L, 1000, 2000);
  xTaskCreatePinnedToCore(ControlEngine, "Ctrl", 8192, NULL, 3, NULL, 0);
}

void loop() {
  if (Serial.available()) targetRPM = Serial.parseFloat();
  Serial.printf("%.1f,%.1f,%.1f\n", targetRPM, rpmR, rpmL);
  delay(200);
}
