/*
  Special thanks to https://github.com/cho45/esp8266-esp-now-sample/blob/master/slave/src/main.cpp
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "ESPERT_OLED.hpp"
#include "CMMC_Interval.hpp"

extern "C" {
  #include <espnow.h>
  #include <user_interface.h>
}

ESPert_OLED oled;
CMMC_Interval senderInterval;
CMMC_Interval counterInterval;
uint16_t msg_sent_cb_counter = 0;
uint16_t msg_sent_timer_counter = 0;

#define RESET_TIMER(_var) (_var = 0)
#define TIMER_COUNT(_var) (_var = _var+1)


#define WIFI_DEFAULT_CHANNEL 9
// neo = {0x1A,0xFE,0x34,0xEE,0xCA,0xED}

//uint8_t neo_slave[] = {0x1A,0xFE,0x34,0xEE,0xCA,0xED};
//uint8_t bare_up_slave[] = {0x5E,0xCF,0x7F,0x9,0x98,0x4E};
//uint8_t bare_no_up_nodht[] = {0x1A,0xFE,0x34,0xDA,0xEA,0xD0};
//uint8_t no[2][6]= {
//    {0x1A,0xFE,0x34,0xDA,0xEF,0x5F},
//    {0x1A,0xFE,0x34,0xDB,0x32,0xEB},
// };
uint8_t slave001[]= {0x1A,0xFE,0x34,0xDB,0x3D,0x22};
// {0x1A,0xFE,0x34,0xDA,0xEF,0x5F};
// uint8_t no2[]= {0x1A,0xFE,0x34,0xDB,0x32,0xEB};


void printMacAddress(uint8_t* macaddr) {
  Serial.print("{");
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    Serial.print(macaddr[i], HEX);
    if (i < 5) Serial.print(',');
  }
  Serial.println("}");
}

static bool _status;
static uint8_t message[] = { };

void setup() {
  _status = false;
  WiFi.disconnect();
  oled.init();
  delay(1000);
  oled.clear();
  // oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.print("OK :)");
  oled.update();

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Initializing...");
  WiFi.mode(WIFI_STA);

  uint8_t macaddr[6];
  wifi_get_macaddr(STATION_IF, macaddr);
  Serial.print("mac address (STATION_IF): ");
  printMacAddress(macaddr);

  wifi_get_macaddr(SOFTAP_IF, macaddr);
  Serial.print("mac address (SOFTAP_IF): ");
  printMacAddress(macaddr);

  if (esp_now_init()==0) {
    Serial.println("direct link  init ok");
    oled.println("direct link  init ok");
    oled.update();
  } else {
    Serial.println("dl init failed");
    oled.println("dl init failed");
    oled.update();
    ESP.restart();
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_recv_cb([](uint8_t *macaddr, uint8_t *data, uint8_t len) {
    // Serial.println("recv_cb");
    //
    // Serial.print("mac address: ");
    // printMacAddress(macaddr);
    //
    // Serial.print("data: ");
    // for (int i = 0; i < len; i++) {
    //   Serial.print(data[i], HEX);
    // }
    // Serial.println("");
    // oled.print("recv_cb");
    // oled.update();
  });

  esp_now_register_send_cb([](uint8_t* macaddr, uint8_t status) {
    TIMER_COUNT(msg_sent_cb_counter);
    /*
    Serial.println("send_cb");
    Serial.print("mac address: ");
    printMacAddress(macaddr);
    Serial.print("status = "); Serial.println(status);
    */
  });

  int res = esp_now_add_peer(slave001, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
  // int res2 = esp_now_add_peer(no2, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
//  int res2 = esp_now_add_peer(no[1], (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);

//  res = esp_now_add_peer(bare_up_slave, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);
//  res = esp_now_add_peer(bare_no_up_nodht, (uint8_t)ESP_NOW_ROLE_SLAVE,(uint8_t)WIFI_DEFAULT_CHANNEL, NULL, 0);

  // ESP.deepSleep(2.5e6, WAKE_RF_DEFAULT);

//  esp_now_unregister_recv_cb();
//  esp_now_deinit();
}

uint16_t _local_timer = 0;
void loop() {
  senderInterval.every_ms(1, []() {
    message[0] = _status;
    _status=!_status;
    // esp_now_send(bare_up_slave, message, sizeof(message));



    esp_now_send(NULL, message, 1);
    TIMER_COUNT(msg_sent_timer_counter);
    TIMER_COUNT(_local_timer);

    if (_local_timer == 1000) {
      oled.clear();
      oled.printf("TIMER SENT = %lu/s\r\n\r\n", msg_sent_timer_counter);
      oled.printf("MSG CB SENT = %lu/s\r\n", msg_sent_cb_counter);
      RESET_TIMER(msg_sent_cb_counter);
      RESET_TIMER(msg_sent_timer_counter);
      RESET_TIMER(_local_timer);
      oled.update();
    }
  });

  // counterInterval.every_ms(1000, []() {
  // });

}
