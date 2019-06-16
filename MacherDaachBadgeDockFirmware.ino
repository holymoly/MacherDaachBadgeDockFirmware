/*
 * Badge tries to connect on last configured WiFi. 
 * If not succesfull it will open an access point.
 * Acces 192.168.4.1 to configure WiFi.
*/

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "src/WiFiManager/WiFiManager.h"         //https://github.com/tzapu/WiFiManager

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#define MQTT_SERVER      "nerdparty.holzmolz.de"
#define MQTT_SERVERPORT  8002                   // use 8883 for SSL
#define MQTT_USERNAME    "holymoly"
#define MQTT_KEY         "thisisjustatest"

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

Adafruit_MQTT_Subscribe espSub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/esp", MQTT_QOS_1);
Adafruit_MQTT_Publish espPub = Adafruit_MQTT_Publish(&mqtt, "/esp");


void feedcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a slider callback, the slider value is: ");
  Serial.println(data);
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point named MacherDaachDock + ChipID
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect();

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

/************** MQTT SUBS ***************/

 // Setup a feed called 'slider' for subscribing to changes on the slider
 char ChipId[8];
 sprintf(ChipId,"%08X", ESP.getChipId());
 //Adafruit_MQTT_Subscribe feedChipId = Adafruit_MQTT_Subscribe(&mqtt, ChipId, MQTT_QOS_1);
 espSub.setCallback(feedcallback);
 mqtt.subscribe(&espSub);

}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets and callback em' busy subloop
  // try to spend your time here:
  mqtt.processPackets(10000);
  espPub.publish("test");
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
