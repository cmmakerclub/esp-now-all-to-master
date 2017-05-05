#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

#define WIFI_DEFAULT_CHANNEL 1

// SOFTAP_IF
uint8_t slave[] = {0x1A,0xFE,0x34,0xDB,0x3D,0x22};
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

    Serial.print("[HEX] data: ");
    for (int i = 0; i < len; i++) {
      Serial.print(data[i], HEX);
    }
    Serial.println("");
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

  int res = esp_now_add_peer(slave, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
  Serial.printf("Add slave result: %d \r\n", res);
  // res = esp_now_add_peer(bare_up_slave, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
  // res = esp_now_add_peer(bare_no_up_nodht, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);

  // ESP.deepSleep(2.5e6, WAKE_RF_DEFAULT);

//  esp_now_unregister_recv_cb();
//  esp_now_deinit();
}

void loop() {
  yield();
  // message[0] = b;
  // b=!b;
  // // esp_now_send(neo_slave, message, sizeof(message));
  // // esp_now_send(bare_up_slave, message, sizeof(message));
  // esp_now_send(NULL, message, 1);
  // digitalWrite(LED_BUILTIN, b);
  // delay(100);
}
