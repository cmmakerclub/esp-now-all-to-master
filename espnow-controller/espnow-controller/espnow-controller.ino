#include <Arduino.h>
#include <ESP8266WiFi.h>
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
  DEBUG_PRINTLN("Initializing...");
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
    uint32_t bigNum;
    bigNum = (bigNum << 8) | data[0];
    bigNum = (bigNum << 8) | data[1];
    bigNum = (bigNum << 8) | data[2];
    bigNum = (bigNum << 8) | data[3];

    DEBUG_PRINT("value: ");
    DEBUG_PRINT(bigNum);
    DEBUG_PRINT(" recv_cb from: ");
    printMacAddress(macaddr);
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    DEBUG_PRINT("send_cb to ");
    printMacAddress(macaddr);
    if (status == 0) {
      DEBUG_PRINT("ESPNOW: SEND_FAILED");
    }
    else {
      DEBUG_PRINT("ESPNOW: SEND_OK");
    }
  });
}

void loop() {
  yield();
  // delay(100);
}
