#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
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
