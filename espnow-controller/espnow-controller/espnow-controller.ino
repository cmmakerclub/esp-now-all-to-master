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
bool tickerFlag = 0;
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

bool send_done = false;

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

  // ticker.attach_ms(1500, [&]() {
  //   tickerFlag = 1;
  // });

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    Serial.println("RECEIVE... ");

    for (size_t i = 0; i < len; i++) {
      Serial.println(data[i], HEX);
    }

    DEBUG_PRINT("COUNTER: ");
    DEBUG_PRINTLN(data[0], DEC);
    if (data[0] == 0xff && data[1] == 0xfa) {
      if (data[2] == 0x10) {
        Serial.println("VALID MSG");
        tickerFlag = 1;
        memcpy(slave_mac, macaddr, 6);
      }
      else {
        Serial.println("VALID MSG");
      }
    }
    // uint32_t bigNum;
    // bigNum = (bigNum << 8) | data[0];
    // bigNum = (bigNum << 8) | data[1];
    // bigNum = (bigNum << 8) | data[2];
    // bigNum = (bigNum << 8) | data[3];
    //
    // DEBUG_PRINT("value: ");
    // DEBUG_PRINT(bigNum);
    // DEBUG_PRINT(" recv_cb from: ");
    printMacAddress(macaddr);
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    // DEBUG_PRINT("send_cb to ");
    printMacAddress(macaddr);
    static uint32_t ok = 0;
    static uint32_t fail = 0;
    if (status == 0) {
      // DEBUG_PRINTLN("ESPNOW: SEND_OK");
      ok++;
    }
    else {
      // DEBUG_PRINTLN("ESPNOW: SEND_FAILED");
      fail++;
    }
    // Serial.printf("[SUCCESS] = %lu/%lu \r\n", ok, ok+fail);
    if (send_done) {
      ok =0;
      fail = 0;
    }
  });

  // int add_peer_status = esp_now_add_peer(slave_mac, ESP_NOW_ROLE_SLAVE, WIFI_DEFAULT_CHANNEL, NULL, 0);
  // DEBUG_PRINTF("ADD PEER: %d\r\n", add_peer_status);
}

void loop() {
  yield();
  if (tickerFlag==1) {
    tickerFlag = 0;
    uint8_t message[4];
    send_done = false;
    for (size_t i = 1; i <= 100 ; i++) {
      message[0] = 0xff;
      message[1] = 0xfa;
      message[2] = i;
      message[3] = i;
      esp_now_send(slave_mac, message, 4);
      delay(10);
    }
    delay(50);
    message[0] = 0xff;
    message[1] = 0xfa;
    message[2] = 0x00;
    message[3] = 0x00;
    send_done = true;
    esp_now_send(slave_mac, message, 4);
    delay(50);
  }
  // delay(100);
}
