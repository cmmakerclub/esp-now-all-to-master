#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

#define WIFI_DEFAULT_CHANNEL 1
#define DEBUG_SERIAL 1
#if DEBUG_SERIAL
    #define DEBUG_PRINTER Serial
    #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
    #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
    #define DEBUG_PRINTF(...) { DEBUG_PRINTER.printf(__VA_ARGS__); }
#else
    #define DEBUG_PRINT(...) { }
    #define DEBUG_PRINTLN(...) { }
    #define DEBUG_PRINTF(...) { }
#endif

bool ledState = LOW;
Ticker ticker;
uint8_t slave_mac[6];

// SOFTAP_IF
void printMacAddress(uint8_t* macaddr) {
  DEBUG_PRINT("{");
  for (int i = 0; i < 6; i++) {
    DEBUG_PRINT("0x");
    DEBUG_PRINT(macaddr[i], HEX);
    if (i < 5) DEBUG_PRINT(',');
  }
  DEBUG_PRINTLN("};");
}

void setup() {
  WiFi.disconnect();
  pinMode(LED_BUILTIN, OUTPUT);
  #if DEBUG_SERIAL
    Serial.begin(115200);
  #endif
  DEBUG_PRINTLN("Initializing... Controller..");
  WiFi.mode(WIFI_STA);
  uint8_t macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);
  DEBUG_PRINT("[master] address (STATION_IF): ");
  printMacAddress(macaddr);

  wifi_get_macaddr(SOFTAP_IF, macaddr);
  DEBUG_PRINT("[slave] address (SOFTAP_IF): ");
  printMacAddress(macaddr);

  if (esp_now_init()==0) {
    DEBUG_PRINTLN("direct link  init ok");
  } else {
    DEBUG_PRINTLN("dl init failed");
    ESP.restart();
    return;
  }


  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    Serial.println("RECEIVE... ");
    for (size_t i = 0; i < len; i++) {
      Serial.printf("BYTE[%d] - ", i);
      Serial.println(data[i], HEX);
    }
    printMacAddress(macaddr);
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;


    int add_peer_status = esp_now_add_peer(macaddr, ESP_NOW_ROLE_SLAVE, WIFI_DEFAULT_CHANNEL, NULL, 0);
    uint8_t* first_peer = esp_now_fetch_peer(true);
    Serial.printf("ADD PEER STATUS: [%d] \r\n", add_peer_status);

    uint8_t message[] = {1,1,1,1};
    esp_now_send(first_peer, message, 4);

  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    // DEBUG_PRINT("send_cb to ");
    printMacAddress(macaddr);
    static uint32_t ok = 0;
    static uint32_t fail = 0;
    if (status == 0) {
      DEBUG_PRINTLN("ESPNOW: SEND_OK");
      ok++;
    }
    else {
      DEBUG_PRINTLN("ESPNOW: SEND_FAILED");
      fail++;
    }
    Serial.printf("[SUCCESS] = %lu/%lu \r\n", ok, ok+fail);
  });

  // DEBUG_PRINTF("ADD PEER: %d\r\n", add_peer_status);
}

void loop() {
  yield();

}
