#include <Arduino.h>
#include <WiFi.h>

#define WIFI_NETWORK "Berkeley-IoT"
#define WIFI_PASSWORD "%d3XCc!2"
#define WIFI_TIMEOUT_MS 20000

void connectToWiFi(){
  Serial.print("Connecting to Wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK,WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();

  while(WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS){
    Serial.print(".");
    delay(100);
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Failed");
  }else{
    Serial.print("\nConnected ");
    Serial.print(WiFi.localIP());

  }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  connectToWiFi();

}

void loop() {
  // put your main code here, to run repeatedly:

}
