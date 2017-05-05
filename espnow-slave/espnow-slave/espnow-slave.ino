#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}
#define WIFI_DEFAULT_CHANNEL 1

// USE STATION_IF
uint8_t master_mac[] = {0x18,0xFE,0x34,0xEE,0xA0,0xF9};
uint32_t counter = 0;
uint32_t send_ok_counter = 0;
uint32_t send_fail_counter = 0;

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
    if (status == 0) {
      send_ok_counter++;
      counter++;
      Serial.printf("... send_cb OK. [%lu/%lu]\r\n",
                    send_ok_counter, send_fail_counter);
    }
    else {
      send_fail_counter++;
      Serial.printf("... send_cb FAILED. [%lu/%lu]\r\n",
                    send_ok_counter, send_fail_counter);
    }
  });
}

uint8_t message[] = {0};
void loop() {
  if (millis() % 1000 == 0) {
    Serial.printf("[%lu] sending...\r\n", millis());
    message[0] = (counter >> 24) & 0xFF;
    message[1] = (counter >> 16) & 0xFF;
    message[2] = (counter >> 8) & 0xFF;
    message[3] =  counter & 0xFF;
    Serial.printf("Counter = %lu \r\n", counter);
    esp_now_send(master_mac, message, 4);
    delay(1);
  }
}
