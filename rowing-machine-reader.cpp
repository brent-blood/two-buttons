#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define WIFI_AP "your stupid wifi network"
#define WIFI_PW "your even stupider wifi password"
#define READER_HOST "IP of your computer"
#define READER_PORT 9876

uint32_t prev_wait;
uint32_t next_wait;

uint32_t now;

uint8_t NEXT = 5;
uint8_t PREV = 4;

WiFiUDP udp;

void setup() {
    //Serial.begin(9600);

    WiFi.begin(WIFI_AP, WIFI_PW);
    //Serial.print("Connecting to wifi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //Serial.print(".");
    }
    //Serial.println();

    //Serial.print("Connected, IP address: ");
    //Serial.println(WiFi.localIP());

    prev_wait = millis();
    next_wait = millis();

    // necessary for no good reason if you're sending... but arduino
    udp.begin(READER_PORT);

    pinMode(NEXT, INPUT_PULLUP);
    pinMode(PREV, INPUT_PULLUP);
}

void loop() {
    now = millis();
    if(next_wait < now && digitalRead(NEXT) == LOW) {
        //Serial.printf("NEXT!\n");
        next_wait = now + 500;
        udp.beginPacket(READER_HOST, READER_PORT);
        udp.write("next\n");
        udp.endPacket();
    }

    if(prev_wait < now && digitalRead(PREV) == LOW) {
        //Serial.printf("PREV!\n");
        prev_wait = now + 500;
        udp.beginPacket(READER_HOST, READER_PORT);
        udp.write("prev\n");
        udp.endPacket();
    }
}
