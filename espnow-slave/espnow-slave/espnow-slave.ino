#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

#define WIFI_DEFAULT_CHANNEL 1
// USE STATION_IF
uint8_t master_mac[] = {0x5C,0xCF,0x7F,0x9,0xDA,0xD2};


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
  Serial.println("Initializing... SLAVE");

  WiFi.mode(WIFI_AP_STA);
//  WiFi.softAP("foobar", "12345678", 1, 0);

  uint8_t macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);
  Serial.print("[master] mac address (STATION_IF): ");
  printMacAddress(macaddr);

  wifi_get_macaddr(SOFTAP_IF, macaddr);
  Serial.print("[slave] mac address (SOFTAP_IF): ");
  printMacAddress(macaddr);

  if (esp_now_init() == 0) {
    Serial.println("init");
  } else {
    Serial.println("init failed");
    ESP.restart();
    return;
  }

  Serial.println("SET ROLE SLAVE");
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    Serial.println("recv_cb");

    Serial.print("mac address: ");
    printMacAddress(macaddr);
    Serial.print("data: ");
    for (int i = 0; i < len; i++) {
      Serial.print(" 0x");
      Serial.print(data[i], HEX);
    }
    Serial.println("");
    digitalWrite(LED_BUILTIN, data[0]);
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    Serial.print("send to mac addr: ");
    printMacAddress(macaddr);
    Serial.println(String("status = ") + status);
  });

  // int res = esp_now_add_peer(master_mac, (uint8_t)ESP_NOW_ROLE_CONTROLLER,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
  // Serial.printf("ADD PEER SLAVE RESULT = %d\r\n", res);
//  esp_now_unregister_recv_cb();
//  esp_now_deinit();
}

uint8_t message[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08 };
void loop() {
  if (millis() % 1000 == 0) {
    Serial.printf("[%lu] sending...\r\n", millis());
    esp_now_send(master_mac, message, 7);
    delay(1);
  }
}
