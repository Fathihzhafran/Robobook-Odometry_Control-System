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

// --- HARDWARE CONFIG ---
const int PIN_ESC_A = 23; const int PIN_ESC_B = 13;
const int HALL1[] = {25, 26, 27}; const int HALL2[] = {14, 32, 33};

// --- ADRC & CONTROL PARAMETERS ---
float b0 = 0.65; float wc = 0.6; float wo = 1.2; 
float Kyaw = 5.0; float Ksync = 3.0; float K_FF = 0.42;
float minStartRPM = 28.0; [cite: 71]

// --- STATE VARIABLES ---
volatile float targetRPM = 0;
float actualTarget = 0, targetHeading = 0;
volatile float rpm1 = 0, rpm2 = 0;
volatile long pulse1 = 0, pulse2 = 0;
bool bnoConnected = false, isFirstRun = true;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
float z1 = 0, z2 = 0, last_u = 0;

// Helper 14-bit PWM Conversion
int msToDuty(int ms) { return (ms * 16383) / 20000; } [cite: 77]

void IRAM_ATTR isr1() { pulse1++; }
void IRAM_ATTR isr2() { pulse2++; }

void ControlTask(void * pvParameters) {
  unsigned long lastTime = millis();
  for(;;) {
    unsigned long now = millis();
    if (now - lastTime >= 100) {
      float dt = (now - lastTime) / 1000.0; lastTime = now;

      noInterrupts();
      long c1 = pulse1; pulse1 = 0; long c2 = pulse2; pulse2 = 0;
      interrupts();

      rpm1 = (rpm1 * 0.9) + (((c1 / 90.0) * (60.0 / dt)) * 0.1); [cite: 88]
      rpm2 = (rpm2 * 0.9) + (((c2 / 90.0) * (60.0 / dt)) * 0.1);

      float currentHeading = 0;
      if (bnoConnected) {
        sensors_event_t event;
        bno.getEvent(&event); currentHeading = event.orientation.x;
      }

      if (targetRPM > 1) {
        if (isFirstRun) { 
          targetHeading = currentHeading;
          actualTarget = minStartRPM; [cite: 91]
          isFirstRun = false; 
        }
        
        if (actualTarget < targetRPM) actualTarget += 0.4;
        else if (actualTarget > targetRPM) actualTarget -= 1.0;

        // --- ADRC ESO UPDATE ---
        float error_obs = rpm1 - z1;
        z1 += (z2 + b0 * last_u + 2 * wo * error_obs) * dt; [cite: 95]
        z2 += (wo * wo * error_obs) * dt; [cite: 96]
        float u0 = wc * (actualTarget - z1);
        float ctrl = (u0 - z2) / b0; last_u = ctrl; [cite: 97]

        // --- HEADING & SYNC CORRECTION ---
        float hErr = targetHeading - currentHeading;
        if (hErr > 180) hErr -= 360; if (hErr < -180) hErr += 360;
        float yCorr = hErr * Kyaw;
        float sCorr = (rpm1 - rpm2) * Ksync; [cite: 101]

        // --- MIXER (Inverse A, Normal B) ---
        int basePWM = 1125 + (int)(actualTarget * K_FF);
        int outA = basePWM + (int)(ctrl - yCorr - sCorr);
        int outB = basePWM + (int)(ctrl + yCorr + sCorr);

        ledcWrite(PIN_ESC_A, msToDuty(constrain(outA, 1000, 1500)));
        ledcWrite(PIN_ESC_B, msToDuty(constrain(outB, 1000, 1500)));
      } else {
        ledcWrite(PIN_ESC_A, msToDuty(1000)); ledcWrite(PIN_ESC_B, msToDuty(1000));
        z1 = 0; z2 = 0; last_u = 0; isFirstRun = true; actualTarget = 0;
      }
    }
    vTaskDelay(1);
  }
}

void setup() {
  Serial.begin(115200);
  ledcAttach(PIN_ESC_A, 50, 14); ledcAttach(PIN_ESC_B, 50, 14);
  
  ledcWrite(PIN_ESC_A, msToDuty(1500)); 
  ledcWrite(PIN_ESC_B, msToDuty(1000)); 
  delay(5000);

  Wire.begin(21, 22);
  if(bno.begin()) bnoConnected = true;

  for(int i=0; i<3; i++) {
    pinMode(HALL1[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL1[i]), isr1, CHANGE);
    pinMode(HALL2[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL2[i]), isr2, CHANGE);
  }
  
  xTaskCreatePinnedToCore(ControlTask, "ADRC_Core", 8192, NULL, 3, NULL, 1);
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read(); float val = Serial.parseFloat();
    switch(cmd) {
      case 'T': targetRPM = val; break;
      case 'B': b0 = val; break;
      case 'O': wo = val; break;
      case 'Y': Kyaw = val; break;
    }
  }
  Serial.printf("%.1f,%.1f,%.1f,%.1f\n", targetRPM, rpm1, rpm2, z2);
  delay(100);
}
