#include <Arduino.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

#include <Ticker.h>

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

// USE STATION_IF
uint8_t master_mac[] = {0x18,0xFE,0x34,0xEE,0xA0,0xF9};
uint32_t counter = 0;
uint32_t send_ok_counter = 0;
uint32_t send_fail_counter = 0;

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

  Serial.begin(115200);
  DEBUG_PRINTLN("Initializing... SLAVE");
  WiFi.mode(WIFI_AP_STA);

  uint8_t macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);
  DEBUG_PRINT("[master] mac address (STATION_IF): ");
  printMacAddress(macaddr);

  wifi_get_macaddr(SOFTAP_IF, macaddr);
  DEBUG_PRINT("[slave] mac address (SOFTAP_IF): ");
  printMacAddress(macaddr);

  if (esp_now_init() == 0) {
    DEBUG_PRINTLN("init");
  } else {
    DEBUG_PRINTLN("init failed");
    ESP.restart();
    return;
  }

  DEBUG_PRINTLN("SET ROLE SLAVE");
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    DEBUG_PRINTLN("recv_cb");
    DEBUG_PRINT("mac address: ");
    printMacAddress(macaddr);
    DEBUG_PRINT("data: ");
    for (int i = 0; i < len; i++) {
      DEBUG_PRINT(" 0x");
      DEBUG_PRINT(data[i], HEX);
    }
    DEBUG_PRINTLN("");
    digitalWrite(LED_BUILTIN, data[0]);
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    DEBUG_PRINT(millis());
    DEBUG_PRINT("send to mac addr: ");
    printMacAddress(macaddr);
    if (status == 0) {
      send_ok_counter++;
      counter++;
      DEBUG_PRINTF("... send_cb OK. [%lu/%lu]\r\n", send_ok_counter, send_ok_counter + send_fail_counter);
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
      send_fail_counter++;
      DEBUG_PRINTF("... send_cb FAILED. [%lu/%lu]\r\n", send_ok_counter, send_ok_counter + send_fail_counter);
    }
  });
}

uint8_t message[] = {0};

void loop() {
  if (digitalRead(13) == LOW) {
    message[3] =  counter & 0xFF;
    message[2] = (counter >> 8)  & 0xFF;
    message[1] = (counter >> 16) & 0xFF;
    message[0] = (counter >> 24) & 0xFF;
    digitalWrite(LED_BUILTIN, LOW);
    DEBUG_PRINTLN(millis());
    esp_now_send(master_mac, message, 4);
    delay(200);
  }
}
