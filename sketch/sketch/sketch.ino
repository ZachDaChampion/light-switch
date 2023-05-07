/**
   BasicHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#include "auth.h"
#include "cert.h"

#define RTCMEM_MARKER 0xB00B5

#define SERVER_NAME(auth_token) "https://wap.tplinkcloud.com/?token=" auth_token

#define POST_REQUEST(dev_id, req_data)    \
  "{"                                     \
  "\"method\":\"passthrough\","         \
  "\"params\":{"                        \
  "\"deviceId\":\"" dev_id "\","      \
  "\"requestData\":\"" req_data "\""  \
  "}"                                   \
  "}"

#define REQ_DATA_TURN_OFF "{\\\"system\\\":{\\\"set_relay_state\\\":{\\\"state\\\":0}}}"
#define REQ_DATA_TURN_ON  "{\\\"system\\\":{\\\"set_relay_state\\\":{\\\"state\\\":1}}}"
#define REQ_DATA_GET_INFO "{\\\"system\\\":{\\\"get_sysinfo\\\":{}}}"

void wifi_connect() {
  // persistent and autoconnect should be true by default, but lets make sure.
  if (!WiFi.getAutoConnect()) WiFi.setAutoConnect(true);  // autoconnect from saved credentials
  if (!WiFi.getPersistent()) WiFi.persistent(true);     // save the wifi credentials to flash

  const uint8_t mac[6] = WIFI_MAC;
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL, mac, true);


  // now wait for good connection, or reset
  uint16_t counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    if (++counter > 100) {        // allow up to 10-sec to connect to wifi
      Serial.println("wifi timed-out. Rebooting..");
      delay(10);  // so the serial message has time to get sent
      ESP.restart();
    }
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  // a significant part of the speed gain is by using a static IP config
  const IPAddress IPA(192,168,0,235);
  const IPAddress GATE(192,168,0,1);
  const IPAddress MASK(255,255,255,0);
//  WiFi.config(IPA, GATE, MASK);

  Serial.println("connecting");
  wifi_connect();
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("AAAAA NO WIFI HELP");
    wifi_connect();
  }

  WiFiClientSecure client;
  client.setInsecure();
  Serial.printf("HTTP connect %d\n", client.connect(SERVER_NAME(TPLINK_TOKEN), 443));

  // Check for saved state
  uint32_t r;
  ESP.rtcUserMemoryRead(0, &r, sizeof(r));
  if (r != RTCMEM_MARKER) {
    Serial.print("[HTTP] begin...\n");
  if (http.begin(client, SERVER_NAME(TPLINK_TOKEN));) {  // HTTP
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(POST_REQUEST(TPLINK_DEVICE_ID, REQ_DATA_GET_INFO));
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      int idx = payload.indexOf("relay_state");
      bool o = payload.substring(idx + 14, idx + 15) == "1";
      ESP.rtcUserMemoryWrite(1, &o, sizeof(o));
      ESP.rtcUserMemoryWrite(0, &r, sizeof(r));
    }
    
  }

  wifi_set_opmode(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();
  gpio_pin_wakeup_enable(D1, GPIO_PIN_INTR_HILEVEL);
  wifi_fpm_do_sleep(0xFFFFFFF);
  delay(10);
//while (!digitalRead(D1))
//  delay(10);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("AAAAA NO WIFI HELP");
    wifi_connect();
  }

  WiFiClientSecure client;
  client.setInsecure();
  Serial.printf("HTTP connect %d\n", client.connect(SERVER_NAME(TPLINK_TOKEN), 443));

  HTTPClient http;

  Serial.print("[HTTP] begin...\n");
  if (http.begin(client, SERVER_NAME(TPLINK_TOKEN));) {  // HTTP
    http.addHeader("Content-Type", "application/json");


    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header
    Serial.println(POST_REQUEST(TPLINK_DEVICE_ID, REQ_DATA_GET_INFO));
    int httpCode = http.POST(POST_REQUEST(TPLINK_DEVICE_ID, REQ_DATA_GET_INFO));

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      int idx = payload.indexOf("relay_state");
      bool on = payload.substring(idx + 14, idx + 15) == "1";

      if (on)
        http.POST(POST_REQUEST(TPLINK_DEVICE_ID, REQ_DATA_TURN_OFF));
      else
        http.POST(POST_REQUEST(TPLINK_DEVICE_ID, REQ_DATA_TURN_ON));
    }
    else
      Serial.printf("ERROR %d: %s\n", httpCode, http.errorToString(httpCode).c_str());

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  delay(250);
}
