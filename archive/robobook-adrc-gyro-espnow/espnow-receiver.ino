#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// --- PINOUT ---
const int PIN_ESC1 = 13; // Motor Kiri
const int PIN_ESC2 = 3;  // Motor Kanan
const int HALL1[] = {12, 11, 10}; 
const int HALL2[] = {18, 17, 16};

// --- PARAMETER FISIK (HASIL EKSPERIMEN) ---
// Dari data: 1055 PWM = 29 RPM | 1150 PWM = 116 RPM
float ff_slope = 1.1;       // Kenaikan PWM per 1 RPM
int ff_intercept = 1023;    // Titik nol teoritis
int static_limit = 1055;    // PWM minimal agar roda mulai berputar

// --- PARAMETER ADRC (TUNING) ---
float b0 = 4.0;  // Inersia/Gain sistem (Semakin besar = respon makin lembut)
float wc = 0.5;  // Bandwidth Controller (Respon Gas)
float wo = 1.0;  // Bandwidth Observer (Kepekaan terhadap beban/tanjakan)

// --- NAVIGASI & SYNC ---
float Kyaw = 2.5;   // Kekuatan koreksi arah (Kompas)
float Ksync = 4.0;  // Kekuatan sinkronisasi roda kiri-kanan
int right_bias = 0; // Tambahan PWM manual untuk roda kanan (jika masih berat sebelah)

// --- VARIABLES SISTEM ---
float accelRate = 3.0; // Soft Start (RPM per cycle)
float targetRPM = 0;
float actualTarget = 0, targetHeading = 0;
bool isFirstRun = true, bnoConnected = false;
int rotateMode = 0; 

// --- ADRC STATE ---
float z1_1 = 0, z2_1 = 0, last_u_1 = 0; // Kiri
float z1_2 = 0, z2_2 = 0, last_u_2 = 0; // Kanan

// --- SENSOR DATA ---
volatile float rpm1 = 0, rpm2 = 0;
volatile long pulse1 = 0, pulse2 = 0;
volatile int st1 = -1, st2 = -1;
unsigned long lastCurveTime = 0;

String incomingCmd = "";
bool newDataReceived = false;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);

// Konversi ms (1000-2000) ke Duty Cycle 14-bit
int msToDuty(int ms) { return (ms * 16383) / 20000; }

// --- ISR ENCODER ---
void IRAM_ATTR isr1() {
  int cur = (digitalRead(HALL1[0]) << 2) | (digitalRead(HALL1[1]) << 1) | digitalRead(HALL1[2]);
  if (cur != st1 && cur > 0 && cur < 7) { pulse1++; st1 = cur; }
}
void IRAM_ATTR isr2() {
  int cur = (digitalRead(HALL2[0]) << 2) | (digitalRead(HALL2[1]) << 1) | digitalRead(HALL2[2]);
  if (cur != st2 && cur > 0 && cur < 7) { pulse2++; st2 = cur; }
}

// --- ESP-NOW CALLBACK ---
void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  char buf[len + 1];
  memcpy(buf, incomingData, len);
  buf[len] = 0; 
  incomingCmd = String(buf);
  newDataReceived = true; 
}

// --- TASK KONTROL (CORE 1) ---
void ControlTask(void * pvParameters) {
  unsigned long lastTime = millis();
  
  for(;;) {
    unsigned long now = millis();
    if (now - lastTime >= 100) { // Loop 10Hz (100ms)
      float dt = (now - lastTime) / 1000.0; 
      lastTime = now;
      
      // 1. HITUNG RPM (FILTERED)
      noInterrupts(); 
      long c1 = pulse1; pulse1 = 0; 
      long c2 = pulse2; pulse2 = 0; 
      interrupts();
      
      // Filter 85% lama, 15% baru (Agar tidak jitter)
      rpm1 = (rpm1 * 0.85) + (((c1 / 90.0) * (60.0 / dt)) * 0.15);
      rpm2 = (rpm2 * 0.85) + (((c2 / 90.0) * (60.0 / dt)) * 0.15);

      // 2. BACA KOMPAS
      float curH = 0;
      if (bnoConnected) { 
        sensors_event_t ev; 
        bno.getEvent(&ev); 
        curH = ev.orientation.x; 
      }

      // 3. LOGIKA KONTROL
      if (targetRPM > 1) {
        if (isFirstRun) { 
          targetHeading = curH; 
          actualTarget = 0; 
          z1_1 = 0; z2_1 = 0; last_u_1 = 0; 
          z1_2 = 0; z2_2 = 0; last_u_2 = 0;
          isFirstRun = false; 
        }
        
        // Soft Start (Ramp Up)
        if (abs(actualTarget - targetRPM) > accelRate) {
           if (actualTarget < targetRPM) actualTarget += accelRate;
           else actualTarget -= accelRate;
        } else {
           actualTarget = targetRPM; 
        }

        // --- A. FEED FORWARD (The Magic Formula) ---
        // Hitung kebutuhan dasar PWM berdasarkan regresi linear
        int basePWM = ff_intercept + (int)(actualTarget * ff_slope);
        
        // Anti-Stiction: Jangan kirim PWM di bawah ambang batas gerak
        if (basePWM < static_limit) basePWM = static_limit;

        // --- B. ADRC CORE ---
        // Roda Kiri
        float err_1 = rpm1 - z1_1;
        z1_1 += (z2_1 + b0 * last_u_1 + 2 * wo * err_1) * dt; 
        z2_1 += (wo * wo * err_1) * dt;
        z2_1 *= 0.90; // Leakage (Anti-Windup)
        float ctrl1 = (wc * (actualTarget - z1_1) - z2_1) / b0; last_u_1 = ctrl1;

        // Roda Kanan
        float err_2 = rpm2 - z1_2;
        z1_2 += (z2_2 + b0 * last_u_2 + 2 * wo * err_2) * dt; 
        z2_2 += (wo * wo * err_2) * dt;
        z2_2 *= 0.90; // Leakage
        float ctrl2 = (wc * (actualTarget - z1_2) - z2_2) / b0; last_u_2 = ctrl2;

        // --- C. MIXER OUTPUT ---
        if (rotateMode == 1) { // PIVOT KIRI
            int pwr = basePWM + ctrl1 + 20;
            ledcWrite(PIN_ESC1, msToDuty(constrain(pwr, 1050, 1450))); 
            ledcWrite(PIN_ESC2, msToDuty(1000)); 
            
        } else if (rotateMode == 2) { // PIVOT KANAN
            ledcWrite(PIN_ESC1, msToDuty(1000));
            int pwr = basePWM + ctrl2 + 20 + right_bias;
            ledcWrite(PIN_ESC2, msToDuty(constrain(pwr, 1050, 1450)));
            
        } else { // MAJU / CURVE
            
            // Heading Correction
            float headingError = targetHeading - curH;
            if (headingError < -180) headingError += 360;
            if (headingError > 180) headingError -= 360;

            float yCorr = headingError * Kyaw;
            yCorr = constrain(yCorr, -50, 50); 
            
            // Sync Correction (Agar roda saling tunggu)
            float sCorr = (rpm1 - rpm2) * Ksync;
            sCorr = constrain(sCorr, -30, 30);

            // Mixer
            int outLeft  = basePWM + ctrl1 - yCorr - sCorr;
            int outRight = basePWM + ctrl2 + yCorr + sCorr + right_bias;

            // Safety Limit
            if (outLeft < static_limit) outLeft = static_limit;
            if (outRight < static_limit) outRight = static_limit;

            ledcWrite(PIN_ESC1, msToDuty(constrain(outLeft, 1000, 1500)));
            ledcWrite(PIN_ESC2, msToDuty(constrain(outRight, 1000, 1500)));
        }

      } else {
        // STOP CONDITION
        ledcWrite(PIN_ESC1, msToDuty(1000)); 
        ledcWrite(PIN_ESC2, msToDuty(1000));
        
        // Reset State ADRC agar tidak 'kaget' saat start lagi
        z1_1 = 0; z2_1 = 0; last_u_1 = 0; 
        z1_2 = 0; z2_2 = 0; last_u_2 = 0;
        isFirstRun = true; actualTarget = 0; rotateMode = 0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting System...");
  delay(2000); 
  
  // --- WIFI SETUP ---
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) { Serial.println("Error ESP-NOW"); return; }
  esp_now_register_recv_cb(OnDataRecv);

  // --- ESC SETUP (50Hz, 14-bit) ---
  ledcAttach(PIN_ESC1, 50, 14); 
  ledcAttach(PIN_ESC2, 50, 14);
  ledcWrite(PIN_ESC1, msToDuty(1000)); 
  ledcWrite(PIN_ESC2, msToDuty(1000));

  // --- SENSOR SETUP ---
  Wire.begin(8, 9); 
  Serial.println("Init BNO055...");
  if(!bno.begin()) {
    Serial.println("BNO Error! Cek kabel.");
    bnoConnected = false;
  } else {
    Serial.println("BNO OK!");
    bnoConnected = true;
    bno.setExtCrystalUse(true);
  }

  for(int i=0; i<3; i++) {
    pinMode(HALL1[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL1[i]), isr1, CHANGE);
    pinMode(HALL2[i], INPUT_PULLUP); attachInterrupt(digitalPinToInterrupt(HALL2[i]), isr2, CHANGE);
  }

  delay(1000); 
  xTaskCreatePinnedToCore(ControlTask, "NavTask", 8192, NULL, 1, NULL, 1);
  Serial.println("SYSTEM READY. Waiting for Python Command...");
}

void loop() {
  // Loop utama hanya untuk parsing data dari Python/ESP-NOW
  if (newDataReceived) {
      String cmd = incomingCmd;
      cmd.trim();
      newDataReceived = false; 

      int separatorIndex = cmd.indexOf(':');
      
      if (separatorIndex != -1) {
          String type = cmd.substring(0, separatorIndex);
          String valStr = cmd.substring(separatorIndex + 1);
          float inputRPM = valStr.toFloat();

          targetRPM = inputRPM;

          if (type == "M") { rotateMode = 0; }
          else if (type == "CL") { 
             rotateMode = 0; 
             if (millis() - lastCurveTime > 100) {
                targetHeading -= 2.0; 
                lastCurveTime = millis();
             }
          }
          else if (type == "CR") { 
             rotateMode = 0;
             if (millis() - lastCurveTime > 100) {
                targetHeading += 2.0; 
                lastCurveTime = millis();
             }
          }
          else if (type == "RL") { rotateMode = 1; }
          else if (type == "RR") { rotateMode = 2; }
      }
      else if (cmd == "STOP") { 
          targetRPM = 0; rotateMode = 0; 
      }
  }
}
