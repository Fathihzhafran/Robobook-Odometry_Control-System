#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> // Wajib untuk paksa ganti channel

// Alamat Broadcast (Kirim ke semua device)
uint8_t broadcastAddress[] = {0xB8, 0xF8, 0x62, 0xF4, 0x54, 0x34};

// Callback saat data terkirim (Kosongkan saja)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Paksa Channel 1 (Biar jodoh sama Receiver)
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) return;
  
  // Fix casting untuk ESP32-S3
  esp_now_register_send_cb((esp_now_send_cb_t)OnDataSent);

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) return;
  Serial.println("TRANSMITTER READY.");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim(); // Hilangkan spasi/enter

    String cmdToSend = "";

    // LOGIKA GANDA:
    // 1. Kalau input cuma 1 huruf (dari Serial Monitor manual) -> Terjemahkan
    if (input.length() == 1) {
      if (input.equalsIgnoreCase("w"))      cmdToSend = "FORWARD";
      else if (input.equalsIgnoreCase("s")) cmdToSend = "STOP";
      else if (input.equalsIgnoreCase("a")) cmdToSend = "LEFT";
      else if (input.equalsIgnoreCase("d")) cmdToSend = "RIGHT";
      else if (input.equalsIgnoreCase("q")) cmdToSend = "ROTATE_Q";
      else if (input.equalsIgnoreCase("e")) cmdToSend = "ROTATE_E";
      else if (input.equalsIgnoreCase("b")) cmdToSend = "BOOST";
    } 
    // 2. Kalau input panjang (dari Python: "FORWARD", "STOP") -> Kirim langsung
    else {
      cmdToSend = input;
    }

    // Kirim via ESP-NOW
    if (cmdToSend.length() > 0) {
      esp_now_send(broadcastAddress, (uint8_t *)cmdToSend.c_str(), cmdToSend.length());
      // Serial.println("Sent: " + cmdToSend); // Debug boleh dimatikan biar gak spam
    }
  }
}
