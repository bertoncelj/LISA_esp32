/*
 * Sketch: ESP8266_LED_Control_04_Station_Mode
 * Control an LED from a web browser
 * Intended to be run on an ESP8266
 * 
 * Wait for the ESP8266 to connect to the local wifi
 * then use a web broswer to go it's ip address
 * 
 */
 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK  "thereisnospoon"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;
 
 
ESP8266WebServer server(80);
 
const int led = 13;
const int LED_Pin = 13;

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/><meta charset='utf-8'><style>body {font-size:140%;} #main {display: table; margin: auto;  padding: 0 10px 0 10px; } h2,{text-align:center; } .button { padding:10px 10px 10px 10px; width:100%;  background-color: #4CAF50; font-size: 120%;}</style><title>LED Control</title></head><body><div id='main'><h2>LED Control</h2>";
String html_2 = "";
String html_4 = "</div></body></html>";
 
String request = "";
 
void setup() 
{
        pinMode(led, OUTPUT); 
        digitalWrite(led, 0);
 
        Serial.begin(115200);
        delay(500);
        Serial.println(F("Serial started at 115200"));
        Serial.println();
 
        /* You can remove the password parameter if you want the AP to be open. */
        WiFi.softAP(ssid, password);
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(myIP);
        Serial.print("AP name (ssid): ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password);

      while (WiFi.status() != WL_CONNECTED) 
      {
          Serial.print(".");    delay(500);
      }
      Serial.println("");
      Serial.println(F("[CONNECTED]"));
      Serial.print("[IP ");              
      Serial.print(WiFi.localIP()); 
      Serial.println("]");
 
      // start a server
      server.begin();
      Serial.println("Server started");
 
} // void setup()
 
 
 
void loop() 
{
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client)  {  return;  }
 
    // Read the first line of the request
    request = client.readStringUntil('\r');
 
    if       ( request.indexOf("LEDON") > 0 )  { digitalWrite(LED_Pin, HIGH);  }
    else if  ( request.indexOf("LEDOFF") > 0 ) { digitalWrite(LED_Pin, LOW);   }
 
 
    // Get the LED pin status and create the LED status message
    if (digitalRead(LED_Pin) == HIGH) 
    {
        // the LED is on so the button needs to say turn it off
       html_2 = "<form id='F1' action='LEDOFF'><input class='button' type='submit' value='Turn off the LED' ></form><br>";
    }
    else                              
    {
        // the LED is off so the button needs to say turn it on
        html_2 = "<form id='F1' action='LEDON'><input class='button' type='submit' value='Turn on the LED' ></form><br>";
    }
 
 
    client.flush();
 
    client.print( header );
    client.print( html_1 );    
    client.print( html_2 );
    client.print( html_4);
 
    delay(5);
  // The client will actually be disconnected when the function returns and 'client' object is detroyed
 
} // void loop()
