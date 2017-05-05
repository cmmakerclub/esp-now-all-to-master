#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

#define WIFI_DEFAULT_CHANNEL 1

// SOFTAP_IF
void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("}");
}

void setup() {
  WiFi.disconnect();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Initializing...");
  WiFi.mode(WIFI_STA);

  uint8_t macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);
  Serial.print("[master] address (STATION_IF): ");
  printMacAddress(macaddr);

  wifi_get_macaddr(SOFTAP_IF, macaddr);
  Serial.print("[slave] address (SOFTAP_IF): ");
  printMacAddress(macaddr);

  if (esp_now_init()==0) {
    Serial.println("direct link  init ok");
  } else {
    Serial.println("dl init failed");
    ESP.restart();
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    Serial.print("recv_cb from: ");
    printMacAddress(macaddr);
    uint32_t bigNum;
    bigNum = (bigNum << 8) | data[0];
    bigNum = (bigNum << 8) | data[1];
    bigNum = (bigNum << 8) | data[2];
    bigNum = (bigNum << 8) | data[3];

    Serial.print("value: ");
    Serial.println(bigNum);
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    Serial.print("send_cb to ");
    printMacAddress(macaddr);
    if (status == 0) {
      Serial.print("ESPNOW: SEND_FAILED");
    }
    else {
      Serial.print("ESPNOW: SEND_OK");
    }
  });
}

void loop() {
  yield();
  // delay(100);
}
