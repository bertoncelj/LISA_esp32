#include "ihub.h"
#ifdef MDNS_DISCOVERY_ENABLED

#include <ESP8266HTTPClient.h>
#define MAX_WiFi_DEVICES 6

int check_if_device_exists(const char *ip_addr);
int numberOfWiFiDevices;

wifiTempSensor_t tempSensor[MAX_WiFi_DEVICES];

void mdns_discover()
{
  int mdns_retry = 5;      
  numberOfWiFiDevices = 0;
  
  memset(tempSensor, 0, sizeof(tempSensor));
  
  while (mdns_retry--)
  {
    debug_println("Sending mDNS query");
    int n = MDNS.queryService("http", "tcp"); // Send out query for esp tcp services
    debug_println("mDNS query done");
    if (n == 0) {
      debug_println("no services found");
    } else {
      debug_print(n);
      debug_println(" service(s) found");
      for (int i = 0; i < n; ++i) {
        // Print details for each service found
        debug_print(i + 1);
        debug_print(": ");
        debug_print(MDNS.hostname(i));
        debug_print(" (");
        debug_print(MDNS.IP(i));
        debug_print(":");
        debug_print(MDNS.port(i));
        debug_println(")");

        if ((MDNS.hostname(i).startsWith("wts") || MDNS.hostname(i).startsWith("mctemp")) && check_if_device_exists(MDNS.IP(i).toString().c_str()) == 0)
        {
          debug_print("Found new WTS device:");
          debug_println(MDNS.hostname(i));
          strcpy(tempSensor[numberOfWiFiDevices].IP, MDNS.IP(i).toString().c_str());
          //j++;
          numberOfWiFiDevices++;
          if (numberOfWiFiDevices >= MAX_WiFi_DEVICES) //todo setting for number of devices
            mdns_retry = 0; //exit flag
        }
      }
    }
    delay(1000);
  }//retry

  getDataFromWifiTempSensors();
}

int getDataFromWifiTempSensors()
{
  HTTPClient http;
  char url[100];
  int j;
  int ret = 0;  
  
  for (j = 0; j < numberOfWiFiDevices; j++)
  {
    strcpy(tempSensor[j].temperature, "--- ");
    strcpy(tempSensor[j].humidity, "--- ");
    strcpy(tempSensor[j].pressure, "--- ");
    strcpy(tempSensor[j].timestamp, "");
    
    debug_print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS

    sprintf(url, "http://%s/measurements", tempSensor[j].IP);
    http.begin(url); //HTTP//http.begin("http://192.168.1.12/test.html"); //HTTP

    debug_print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      DEBUG_UART.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String json = http.getString();
        debug_println(json);
        const size_t bufferSize = JSON_OBJECT_SIZE(13) + 260;
        DynamicJsonBuffer jsonBuffer(bufferSize);

        //const char* json = "{\"Unit\":\"WTS00009\",\"Location\":\"WTS00009\",\"Description\":\"Marjan123456798912345678\",\"Temperature\":\"24.4\",\"Temperature_min\":\"24.1\",\"Temperature_max\":\"24.4\",\"Humidity\":\"53\",\"Pressure\":\"1024\",\"Wifi_signal_level\":\"-86\",\"Interval\":\"1800\",\"Timestamp\":\"1541668524\",\"Uptime\":\"0d0h35'\"}";

        JsonObject& root = jsonBuffer.parseObject(json);

        const char* Unit = root["Unit"];
        if (Unit)
          strcpy(tempSensor[j].serial, root["Unit"]); // "WTS00009"
        const char* Type = root["Type"];
        if (Type)
          strcpy(tempSensor[j].device_type, root["Type"]);
        const char* Location = root["Location"];
        if (Location)
          strcpy(tempSensor[j].location, root["Location"]); // "WTS00009"
        const char* Description = root["Description"];
        if (Description)
          strcpy(tempSensor[j].description, root["Description"]); // "Marjan123456798912345678"
        const char* Temperature = root["Temperature"];
        if (Temperature)
          strcpy(tempSensor[j].temperature, root["Temperature"]); // "24.4"
        //const char* Temperature_min = root["Temperature_min"]; // "24.1"
        //const char* Temperature_max = root["Temperature_max"]; // "24.4"
        const char* Humidity = root["Humidity"];
        if (Humidity)
          strcpy(tempSensor[j].humidity, root["Humidity"]); // "53"
        const char* Pressure = root["Pressure"];
        if (Pressure)
          strcpy(tempSensor[j].pressure, root["Pressure"]); // "1024"

        //const char* Wifi_signal_level = root["Wifi_signal_level"]; // "-86"
        //const char* Interval = root["Interval"]; // "1800"
        const char* Timestamp = root["Timestamp"]; // "1541668524"
        if (Timestamp)
          strcpy(tempSensor[j].timestamp, root["Timestamp"]);
        //const char* Uptime = root["Uptime"]; // "0d0h35'"
      }
      else
      {
        DEBUG_UART.printf("[HTTP] httpCode... failed, error: %s\n", http.errorToString(httpCode).c_str());
        ret = 1;
      }
    } else {
      DEBUG_UART.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      ret = 2;
    }

    http.end();
    yield();
  }//for
  return ret;
}

int check_if_device_exists(const char *ip_addr)
{
  int i;

  for (i = 0; i < numberOfWiFiDevices; i++)
  {
    if (strcmp(tempSensor[i].IP, ip_addr) == 0)
      return 1;
  }
  return 0;
}
#endif
