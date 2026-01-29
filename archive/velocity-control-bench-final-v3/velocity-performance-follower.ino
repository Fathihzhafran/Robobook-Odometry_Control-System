/*
 * Copyright 2026 Fathihzafran
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Project: Robobook Odometry - Performance Follower Logic
 */

#include <ESP32Servo.h>

const int PIN_ESC_R = 23; const int PIN_ESC_L = 12;
const int HALL_R[] = {25, 26, 27}; const int HALL_L[] = {14, 32, 33};

const unsigned long INTERVAL = 100; 
const float PWM_MIN = 1055;

volatile float targetRPM = 0;
volatile float actualTarget = 0; 
float Kp = 0.15, Ki = 0.40, Kd = 0.50, Ksync = 8.0, K_FF = 0.38; 

volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
int stR = -1, stL = -1;
Servo motorR, motorL;

void IRAM_ATTR isrR() {
  int cur = (digitalRead(HALL_R[0]) << 2) | (digitalRead(HALL_R[1]) << 1) | digitalRead(HALL_R[2]);
  if (cur != stR && cur > 0 && cur < 7) { pulseR++; stR = cur; }
}
void IRAM_ATTR isrL() {
  int cur = (digitalRead(HALL_L[0]) << 2) | (digitalRead(HALL_L[1]) << 1) | digitalRead(HALL_L[2]);
  if (cur != stL && cur > 0 && cur < 7) { pulseL++; stL = cur; }
}

void ControlEngine(void * pvParameters) {
  float iR = 0, iSync = 0, sR = 0, sL = 0;
  unsigned long lastT = millis();

  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= INTERVAL) {
      float dt = (now - lastT) / 1000.0; lastT = now;

      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();

      sR = (sR * 0.98) + (((cR / 90.0) * (60.0 / dt)) * 0.02);
      sL = (sL * 0.98) + (((cL / 90.0) * (60.0 / dt)) * 0.02);
      rpmR = sR; rpmL = sL;

      if (actualTarget < targetRPM) actualTarget += 0.5;
      else if (actualTarget > targetRPM) actualTarget -= 2.0;

      if (targetRPM > 0) {
        float errR = actualTarget - rpmR;
        float errSync = rpmR - rpmL;

        iR = constrain(iR + (errR * dt), -20, 20);
        iSync = constrain(iSync + (errSync * dt), -10, 10);

        float outR = (PWM_MIN + (actualTarget * K_FF)) + (errR * Kp) + (iR * Ki);
        float outL = outR + (errSync * Ksync) + (iSync * 2.0); 

        motorR.writeMicroseconds((int)constrain(outR, 1050, 1450));
        motorL.writeMicroseconds((int)constrain(outL, 1050, 1450));
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
  for(int i=0; i<3; i++) {
    pinMode(HALL_R[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_R[i]), isrR, CHANGE);
    pinMode(HALL_L[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL_L[i]), isrL, CHANGE);
  }
  motorR.attach(PIN_ESC_R, 1000, 2000); motorL.attach(PIN_ESC_L, 1000, 2000);
  xTaskCreatePinnedToCore(ControlEngine, "Ctrl", 8192, NULL, 3, NULL, 0);
}

void loop() {
  if (Serial.available() > 0) targetRPM = Serial.readStringUntil('\n').toFloat();
  Serial.printf("%.1f,%.1f,%.1f\n", targetRPM, rpmR, rpmL);
  delay(100);
}
