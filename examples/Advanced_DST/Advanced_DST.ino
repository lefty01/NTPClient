#include <NTPClient.h>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <WiFiUdp.h>

const char *ssid     = "<SSID>";
const char *password = "<PASSWORD>";

const long gmtOffset = 7200; // sec
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       // Central European Standard Time

WiFiUDP ntpUDP;

// You can specify offset (in seconds, can be changed later with setTimeOffset())
// and time zone.
NTPClient timeClient(ntpUDP, gmtOffset, CEST, CET);

void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  delay(1000);
}
