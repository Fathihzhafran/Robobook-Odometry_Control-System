/*
 * Copyright 2026 Fathihzafran
 * Licensed under the Apache License, Version 2.0 (the "License");
 * Project: Robobook Odometry - Conservative Safety
 */

#include <ESP32Servo.h>

const int PIN_ESC_R = 23; const int PIN_ESC_L = 12;
const int HALL_R[] = {25, 26, 27}; const int HALL_L[] = {14, 32, 33};

const unsigned long INTERVAL = 300; 
float Kp = 0.08, Ki = 0.05, FF_PWM = 1070; 

volatile float targetRPM = 0;
volatile float rpmR = 0, rpmL = 0;
volatile long pulseR = 0, pulseL = 0;
int stR = -1, stL = -1;
Servo motorR, motorL;

void IRAM_ATTR isrR() { pulseR++; }
void IRAM_ATTR isrL() { pulseL++; }

void ControlEngine(void * pvParameters) {
  float iR = 0, iL = 0;
  unsigned long lastT = millis();

  for(;;) {
    unsigned long now = millis();
    if (now - lastT >= INTERVAL) {
      float dt = (now - lastT) / 1000.0; lastT = now;
      noInterrupts();
      long cR = pulseR; pulseR = 0; long cL = pulseL; pulseL = 0;
      interrupts();

      rpmR = (cR / 90.0) * (60.0 / dt);
      rpmL = (cL / 90.0) * (60.0 / dt);

      if (targetRPM > 0) {
        iR = constrain(iR + (targetRPM - rpmR), -50, 50);
        iL = constrain(iL + (targetRPM - rpmL), -50, 50);

        float outR = FF_PWM + ((targetRPM - rpmR) * Kp) + (iR * Ki);
        float outL = FF_PWM + ((targetRPM - rpmL) * Kp) + (iL * Ki);

        motorR.writeMicroseconds((int)constrain(outR, 1000, 1300));
        motorL.writeMicroseconds((int)constrain(outL, 1000, 1300));
      } else {
        motorR.writeMicroseconds(1000); motorL.writeMicroseconds(1000);
        iR = 0; iL = 0;
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
  delay(300);
}
