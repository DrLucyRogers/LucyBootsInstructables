/* 
 *  RGB LEDS on Adafruit Feather Huzzah using IBM Bluemix
 *
 * RGB LED test using a Adafruit Huzzah ESP-8266 board and Node-RED on IBM Bluemix
 * expects messages of the form: "#rrggbb" and sets the LEDs to that colour
 * 
 * By Andy Stanford-Clark and James Macfarlane - with embellishments by Lucy Rogers
 * May-Dec 2016
 *
 *
 * Copyright 2016 IBM Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * use board "Adafruit HUZZAH ESP8266"
 * CPU 160MHz
 * 4M (3M SPIFFS)
 * upload speed 115200
 *
 */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// remember to change MQTT_KEEPALIVE to 60 in the header file {arduino installation}/libraries/PubSubClient/src/PubsSubClient.h


/////////////////////////////////////////////////////////////////////////////////////////////

// Update these with values suitable for your network.
const char* wifi_ssid = "****";
const char* wifi_password = "****";


// update this with the Broker address from IBM Watson IoT Platform
#define BROKER "{org_id}.messaging.internetofthings.ibmcloud.com"
// update this with the Client ID in the format d:{org_id}:{device_type}:{device_id}
#define CLIENTID "d:{org_id}:{device_type}:{device_id}"
// update this with the authentcation token
#define PASSWORD "****"

/////////////////////////////////////////////////////////////////////////////////////////////

// RGB pins for the LEDs
#define GPIN 12
#define RPIN 13
#define BPIN 14


// subscribe to this for commands:
#define COMMAND_TOPIC "iot-2/cmd/command/fmt/text"
  
WiFiClient espClient;
PubSubClient client(espClient);

// flashes this colour when connecting to wifi:
static uint32_t wifi_colour = 0x400040; // magenta
// flashes this colour when connecting to MQTT:
static uint32_t mqtt_colour = 0x004040; // cyan


void setup() {
  
  Serial.begin(9600);

  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);
  
  setup_wifi();
  client.setServer(BROKER, 1883);
  client.setCallback(callback);
}


void setup_wifi() {
  
  // connecting to wifi
  set_colour(wifi_colour);
  
  // Start by connecting to the WiFi network   
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  wait_for_wifi();

}


void wait_for_wifi()
{
  
  Serial.println("waiting for Wifi");
  
  // connecting to wifi colour
  set_colour(wifi_colour);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    toggle_colour();
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}


void callback(char* topic, byte* payload, unsigned int length) {
  char content[10];

  // #rrggbb
  
  Serial.print("Message arrived: ");

  if (length != 7)
  {
    Serial.print("expected 7 bytes, got ");
    Serial.println(length);
  }
  else
  {
    content[7] = '\0';

    Serial.print("'");
    Serial.print((char *)payload);
    Serial.println("'");

    // "+1" to skip over the '#'
    strcpy(content, (char *)(payload+1));

    // convert the hex number to decimal   
    uint32_t value = strtol(content, 0, 16);
        
    set_colour(value);
  }
}


void reconnect() {
  boolean first = true;
  
  // Loop until we're reconnected to the broker
  while (!client.connected()) {

    if (WiFi.status() != WL_CONNECTED) {
      wait_for_wifi();
      first = true;
    }
    
    Serial.print("Attempting MQTT connection...");
    if (first) {
      // now we're on wifi, show connecting to MQTT colour
      set_colour(mqtt_colour);
      first = false;
    }
    
    // Attempt to connect
    if (client.connect(CLIENTID, "use-token-auth", PASSWORD)) {
      Serial.println("connected");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
 
      toggle_colour();
    }
  }
  
  set_colour(0); // Turn off LEDs when connected (black)
 
  // subscribe to the command topic
  client.subscribe(COMMAND_TOPIC);
}


void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  // service the MQTT client
  client.loop();

}

// We use this variable to save the current LED colour
// so we can read it back with get_rgb. This is used
// when toggling to find out if the LED is black or
// a colour so it can be set to the opposite thing.
static uint32_t colour_now;

/*
 * Set colour of RGB LED connected to PWM pins.
 */
void set_rgb(uint32_t colour)
{
  uint32_t r,g,b;

  colour_now = colour;
  
  r = (colour & 0xFF0000) >> 16;
  g = (colour & 0x00FF00) >> 8;
  b = (colour & 0x0000FF);

  // Huzzah PWM goes to 1024, not 255, so multiply values by 4
  analogWrite(RPIN, r*4);
  analogWrite(GPIN, g*4);
  analogWrite(BPIN, b*4);
}

uint32_t get_rgb(void)
{
  return colour_now;
}

// We use this for toggling. It stores what colour
// the LED used to be before we set it to black (off)
static uint32_t current_LED = 0x000000; // black


void set_colour(uint32_t colour)
{

  current_LED = colour;
  set_rgb(colour);
}


void toggle_colour() {

  // find out what colour it is
  if (get_rgb() == 0) 
  {
    // if it's black, set it to the stored colour
    set_rgb(current_LED);
  }
  else
  {
    // otherwise set it to black
    set_rgb(0);
  }
}


