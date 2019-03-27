#include "ihub.h"

#define UPGRADE_JSON_BUFLEN 400

#include <ESP8266httpUpdate.h>

extern PubSubClient mqtt_client;
extern StoreStruct_t settings;
extern SettingsStruct_t stmSettings;
extern int32_t WifiSignalLevel;
extern rtcMem_t rtcMem;
//extern int log_id;
extern int uptime;
//extern int errPush;
extern unsigned short i_mqttPort;
extern bool publish_settings;
extern bool stm_settings_valid;

static int errMqtt = 0;
static char mqttUpgradeResponse[UPGRADE_JSON_BUFLEN];
static int debug_to_mqtt = 0;

void publishSettingsRequest();
void publishMeasurementsRequest();
void publishStatusRequest();
void parseCommunicationSettingsJson(char *json);

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  // Convert the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  debug_print(F("Message arrived on topic: ["));
  debug_print(topic);
  debug_print(F("], "));
  //debug_println(message);
  debug_print(F("Message len:"));
  debug_println(length);

  String S_topic = String(topic);

  if (S_topic.endsWith("/cmd"))
  {
    char responseTopic[40];
    sprintf(responseTopic, "%s/%s/cmd/rsp", settings.mqtt_topic, settings.serial);

    if (message == "debug_stm_on") {
      serial_write("", 0, INTERNAL_TYPE, MQTT_DEBUG_ON);
      wait_for_serial_response(1000);
    }
    else if (message == "debug_stm_off") {
      serial_write("", 0, INTERNAL_TYPE, MQTT_DEBUG_OFF);
      wait_for_serial_response(1000);
    }
    else if (message == "debug_wifi_on") {
      publishToMqtt(responseTopic, "Debug wifi ON");
      debug_to_mqtt = 1;
    }
    else if (message == "debug_wifi_off") {
      publishToMqtt(responseTopic, "Debug wifi OFF");
      debug_to_mqtt = 0;
    }
    else if (message == "info" || message == "get_status") {
      const long minute = 60;
      const long hour = minute * 60;
      const long day = hour * 24;
      char uptimeStr[20];
      char tmpBuffer[100];
      char IP[20];
      char mqtt_buffer[300];

      debug_println(F("Info received"));
      sprintf(responseTopic, "%s/%s/info", settings.mqtt_topic, settings.serial);

      snprintf (uptimeStr, 20, "%ldd%ldh%02ld'", uptime / day, (uptime % day) / hour, (uptime % hour) / minute);

      strcpy(mqtt_buffer, "{"); //start
      sprintf(tmpBuffer, "\"Unit\":\"%s\"", settings.serial);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, ",\"SW_version\":\"%d\"", SW_VERSION);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, ",\"Location\":\"%s\"", settings.location);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, ",\"Description\":\"%s\"", settings.description);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, ",\"Interval\":\"%d\"", settings.mqtt_publish_interval);
      strcat(mqtt_buffer, tmpBuffer);

      time_t UTC_time = time(NULL);
      sprintf(tmpBuffer, ",\"UTC_Time\":\"%ld\"", UTC_time);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, ",\"Uptime\":\"%s\"", uptimeStr);
      strcat(mqtt_buffer, tmpBuffer);

      strcpy(IP, WiFi.localIP().toString().c_str());
      sprintf(tmpBuffer, ",\"IP\":\"%s\"", IP);
      strcat(mqtt_buffer, tmpBuffer);

      sprintf(tmpBuffer, "}"); //end
      strcat(mqtt_buffer, tmpBuffer);

      if (strlen(mqtt_buffer) > MQTT_MAX_PACKET_SIZE)
      {
        debug_print(F("ERROR MQTT oversize: "));
        debug_println(strlen(mqtt_buffer));
        statistics.errors++;
        return;
      }

      publishToMqtt(responseTopic, (char*)mqtt_buffer);
    }//info

    if (message == "measurements") {
      publishMeasurementsRequest();
      //delay(10);
    }

    if (message == "counters") {
      publishCountersRequest();
      //delay(10);
    }

    if (message == "settings" || message == "get_settings") {
      publishSettingsRequest();
      //delay(10);
    }

    if (message == "status") {
      publishStatusRequest();
      //delay(10);
    }

    if (message == "bicom on") {
      serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_ON);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom off") {
      serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_OFF);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom toggle") {
      serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_TOGGLE);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }
    if (message == "bicom 1 on") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_ON);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom 1 off") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_OFF);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom 1 toggle") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_TOGGLE);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }
    if (message == "bicom 2 on") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_ON);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom 2 off") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_OFF);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "bicom 2 toggle") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_TOGGLE);
      wait_for_serial_response(1000);
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }
    if (message == "bicom state") {
      serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE_MQTT);
    }

    if (message == "reset") {
      debug_println(F("Reset received"));
      wifi_restart();
    }//reset
  }//cmd
  else if (S_topic.endsWith("/communication"))
  {
    debug_println(F("settings/communication received"));
    parseCommunicationSettingsJson((char *)payload);
  }
  else if (S_topic.endsWith("/application"))
  {
    debug_println(F("settings/application received"));
    //parseConfigurationJson((char *)payload);
  }
  else if (S_topic.endsWith("/firmware/upgrade"))
  {
    debug_println(F("firmware/upgrade received"));
    mqttUpgrade((char *)payload);
  }
} //mqtt callback   //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=8746#sthash.zSEcRpUF.dpuf

void mqttReconnect() {
  if (settings.mqtt_enabled == 0 || i_mqttPort == 0)
  {
    debug_println(F("MQTT port is 0. MQTT is disabled!"));
    return;
  }
  // Loop until we're reconnected
  //while (!mqtt_client.connected()) {
  //zurb try only once
  if (!mqtt_client.connected()) {
    char clientID[20];
    char subscribeTopic[100];
#ifdef MQTT_TLS_ENABLED
    debug_print(F("Attempting MQTT TLS connection to: "));
#else    
    debug_print(F("Attempting MQTT connection to: "));
#endif    
    debug_print(settings.mqtt_server);
    debug_print(F(" port: "));
    debug_println(i_mqttPort);
    strcpy(clientID, settings.serial);
    debug_print(F("MQTT clientid:"));
    debug_println(clientID);
    // Attempt to connect
#ifdef MQTT_TLS_ENABLED
    if (mqtt_client.connect(clientID)) {
#else
    if (mqtt_client.connect(clientID, settings.mqtt_username, settings.mqtt_password)) {
#endif
      debug_println(F("MQTT connected"));

      // ... and subscribe
      debug_print(F("subscribing to "));
      sprintf(subscribeTopic, "%s/%s/cmd", settings.mqtt_topic, settings.serial);
      debug_println(subscribeTopic);
      mqtt_client.subscribe(subscribeTopic);

      debug_print(F("subscribing to "));
      sprintf(subscribeTopic, "%s/%s/settings/application", settings.mqtt_topic, settings.serial);
      debug_println(subscribeTopic);
      mqtt_client.subscribe(subscribeTopic);

      debug_print(F("subscribing to "));
      sprintf(subscribeTopic, "%s/%s/settings/communication", settings.mqtt_topic, settings.serial);
      debug_println(subscribeTopic);
      mqtt_client.subscribe(subscribeTopic);

      debug_print(F("subscribing to "));
      sprintf(subscribeTopic, "%s/%s/firmware/upgrade", settings.mqtt_topic, settings.serial);
      debug_println(subscribeTopic);
      mqtt_client.subscribe(subscribeTopic);

      //publishSettings(); //TODO
    } else {
      errMqttIncrement();
      debug_print(F("failed, rc="));
      debug_println(mqtt_client.state());
      //debug_println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
  else
    debug_println(F("MQTT Connected"));
} //- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=8746#sthash.zSEcRpUF.dpuf

void publishSettingsRequest()
{
  debug_println(F("Publish settings request"));
  serial_write("", 0, MEASUREMENTS_TYPE, IHUB_SETTINGS_MQTT);
}

void publishMeasurementsRequest()
{
  debug_println(F("Publish measurements request"));
  serial_write("", 0, MEASUREMENTS_TYPE, MEASUREMENTS_MQTT);
}

void publishDemandsRequest()
{
  debug_println(F("Publish demands request"));
  serial_write("", 0, MEASUREMENTS_TYPE, MAXIMUM_DEMANDS_MQTT);
}

void publishCountersRequest()
{
  debug_println(F("Publish counters request"));
  serial_write("", 0, MEASUREMENTS_TYPE, COUNTERS_MQTT);
}

void publishStatusRequest()
{
  //debug_println("Publish counters request");
  serial_write("", 0, MEASUREMENTS_TYPE, STATUS_MQTT);
}

void publishAck(char *mqtt_buffer)
{
  char outTopicDevice[60];
  sprintf(outTopicDevice, "%s/%s/ack", settings.mqtt_topic, settings.serial);
  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

void publishDebugWiFiTx(char *mqtt_buffer)
{
  if (debug_to_mqtt)  
  {
    char outTopicDevice[60];
    sprintf(outTopicDevice, "%s/%s/dbg_wifi_tx", settings.mqtt_topic, settings.serial);
    publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
  }
}

void publishSettings(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/settings", settings.mqtt_topic, settings.serial);

  if (publishToMqtt(outTopicDevice, (char*)mqtt_buffer) == true)
    publish_settings = false; //settings published, clear flag
}

void publishMeasurements(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/measurements", settings.mqtt_topic, settings.serial);

  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

void publishDemands(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/demands", settings.mqtt_topic, settings.serial);

  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

void publishCounters(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/counters", settings.mqtt_topic, settings.serial);

  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

void publishStatus(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/status", settings.mqtt_topic, settings.serial);

  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

void publishBicomsState(char *mqtt_buffer)
{
  char outTopicDevice[50];
  sprintf(outTopicDevice, "%s/%s/bicoms", settings.mqtt_topic, settings.serial);

  publishToMqtt(outTopicDevice, (char*)mqtt_buffer);
}

bool publishToMqtt(char *topic, char *mqtt_buffer)
{
  bool ret = mqtt_client.publish(topic, mqtt_buffer, false);
  if (ret == false)
  {
    errMqttIncrement();
    debug_println(F("Publish failed"));
  }
  else
  {
    errMqtt = 0;
  }
  return ret;
}

int errMqttIncrement()
{
  errMqtt++;
  if (errMqtt > MQTT_MAX_ERR)
  {
    char serial_msg[20];
    debug_print(F("MQTT err > "));
    debug_println(MQTT_MAX_ERR);
    debug_println(F("RESTARTING !!!!!!!"));
    sprintf(serial_msg, "MQTT ERR > %d", MQTT_MAX_ERR);
    serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_RESTARTING); //STM_UART.println("RESTARTING");
    wifi_restart();
  }
  debug_print(F("MQTT err: "));
  debug_println(errMqtt);
  return errMqtt;
}

int errMqttGet()
{
  return errMqtt;
}

void parseCommunicationSettingsJson(char *json)
{
  //int i;

  debug_println(F("parseCommunicationSettingsJson"));
  //https://arduinojson.org/v5/assistant/
  const size_t bufferSize = JSON_OBJECT_SIZE(16) + 360;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  //const char* json = "{\"serial_number\":\"IH000001\",\"description\":\"iHUB PCB prototype 1\",\"location\":\"R&D Iskra Otoce\",\"timezone\":\"60\",\"time_sync_src\":\"1\",\"ntp_server1\":\"ntp1.arnes.si\",\"time_dst\":\"1\",\"mqtt_enabled\":\"1\",\"mqtt_server\":\"10.120.4.160\",\"mqtt_port\":\"1883\",\"mqtt_username\":\"\",\"mqtt_password\":\"\",\"mqtt_topic\":\"dexma\"}";

  JsonObject& root = jsonBuffer.parseObject(json);

  const char* description = root[F("description")]; // "iHUB PCB prototype 1"
  if (description)
    strncpy(stmSettings.description, root[F("description")], 40);
  const char* location = root[F("location")]; // "R&D Iskra Otoce"
  if (location)
    strncpy(stmSettings.location, root[F("location")], 40);
  const char* timezone = root[F("timezone")]; // "60"
  if (timezone)
    stmSettings.timezone = atoi(timezone);
  const char* time_sync_src = root[F("time_sync_src")]; // "1"
  if (time_sync_src)
    stmSettings.time_sync_src = atoi(time_sync_src);
  const char* ntp_server1 = root[F("ntp_server1")]; // "ntp1.arnes.si"
  if (ntp_server1)
    strncpy(stmSettings.ntp_server1, root[F("ntp_server1")], 40);
  const char* ntp_server2 = root[F("ntp_server2")]; // "ntp2.arnes.si"
  if (ntp_server2)
    strncpy(stmSettings.ntp_server2, root[F("ntp_server2")], 40);
  const char* ntp_server3 = root[F("ntp_server3")]; // "ntp3.arnes.si"
  if (ntp_server3)
    strncpy(stmSettings.ntp_server3, root[F("ntp_server3")], 40);
  const char* time_dst = root[F("time_dst")]; // "1"
  if (time_dst)
    stmSettings.time_dst = atoi(time_dst);
  const char* mqtt_enabled = root[F("mqtt_enabled")]; // "1"
  if (mqtt_enabled)
    stmSettings.mqtt_enabled = atoi(mqtt_enabled);
  const char* mqtt_server = root[F("mqtt_server")]; // "10.120.4.160"
  if (mqtt_server)
    strncpy(stmSettings.mqtt_server, root[F("mqtt_server")], 40);
  const char* mqtt_port = root[F("mqtt_port")]; // "1883"
  if (mqtt_port)
    stmSettings.mqtt_port = atoi(mqtt_port);
  const char* mqtt_username = root[F("mqtt_username")]; // ""
  if (mqtt_username)
    strncpy(stmSettings.mqtt_username, root[F("mqtt_username")], 16);
  const char* mqtt_password = root[F("mqtt_password")]; // ""
  if (mqtt_password)
    strncpy(stmSettings.mqtt_password, root[F("mqtt_password")], 16);
  const char* mqtt_topic = root[F("mqtt_topic")]; // "dexma"
  if (mqtt_topic)
    strncpy(stmSettings.mqtt_topic, root[F("mqtt_topic")], 20);
  const char* mqtt_publish_interval = root[F("mqtt_publish_interval")];
  if (mqtt_publish_interval)
    stmSettings.mqtt_publish_interval = atoi(mqtt_publish_interval);
  const char* tcp_port = root[F("tcp_port")];
  if (tcp_port)
    stmSettings.tcp_port = atoi(tcp_port);

  sendSettingsToSTM((char*)&stmSettings);
}

void mqttUpgrade(char *json)
{
  t_httpUpdate_return ret;
  bool secure_upgrade = false;
  int httpsPort;
  //int i;
  String filename;
  //String protocol_type = "http://";
  //String response;
  //char mqttUpgradeResponse[UPGRADE_JSON_BUFLEN];
  char status[3] = "KO";
  char code[5];
  char description[50];
  bool ret_code;

  debug_println(F("parseUpgradeJson"));
  debug_print(F("Free Heap    : "));
  debug_println(system_get_free_heap_size());

  //https://arduinojson.org/v5/assistant/
  const size_t capacity = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(5) + 330;
  DynamicJsonBuffer jsonBuffer(capacity);

  //const char* json = "{\"header\":{\"did\":\"+\",\"ts\":\"2019-01-10T10:21:52Z\",\"requestId\":\"​3b1d98d3-7d17-4cda-97f9-ded7d3352cb3​\",\"version\":\"1.0.0\",\"type\":\"cmd\"},\"data\":{\"cmd\":\"upgrade\",\"host\":\"smokuc47a.synology.me\",\"url\":\"/ihub/iHub_wifi.ino.generic_v13.bin\",\"fingerprint\":\"‎43 07 aa 1f a7 f8 8e 87 19 59 7e ff 05 bc b9 1b 67 47 c1 69\",\"port\":\"80\"}}";

  JsonObject& root = jsonBuffer.parseObject(json);

  JsonObject& header = root[F("header")];
  const char* header_did = header[F("did")]; // "+"
  const char* header_ts = header[F("ts")]; // "2019-01-10T10:21:52Z"
  const char* header_requestId = header[F("requestId")]; // "​3b1d98d3-7d17-4cda-97f9-ded7d3352cb3​"
  const char* header_version = header[F("version")]; // "1.0.0"
  //const char* header_type = header[F("type")]; // "cmd"

  JsonObject& data = root[F("data")];
  const char* data_cmd = data[F("cmd")]; // "upgrade"
  const char* data_host = data[F("host")]; // "smokuc47a.synology.me"
  const char* data_url = data[F("url")]; // "/ihub/iHub_wifi.ino.generic_v13.bin"
  //const char* data_fingerprint = data[F("fingerprint")]; // "‎43 07 aa 1f a7 f8 8e 87 19 59 7e ff 05 bc b9 1b 67 47 c1 69"
  const char* data_port = data[F("port")]; // "80"

  debug_print(F("Free Heap    : "));
  debug_println(system_get_free_heap_size());

  //char debug[100];
  //sprintf(debug, "did:%s ts:%s reqId:%s version:%s type:%s", header_did, header_ts, header_requestId, header_version, header_type);
  //debug_println(debug);
  //publishDebugWiFiTx(debug);

  if (data_port)
    httpsPort = atoi(data_port);
  else
    httpsPort = 80;

  debug_print(F("Port:"));
  debug_println(httpsPort);

#ifdef SECURE_UPGRADE
  if (httpsPort == 443)
  {
    secure_upgrade = true;
    protocol_type = "https://";
    debug_println(F("SECURE UPGRADE"));
    if (!data_fingerprint)
    {
      //response += responseKO + "\"JSON parameters ERROR: fingerprint\" }}}";
      strcpy(status, "ko");
      strncpy(description, "missing sha fingerprint", 50);
      sprintf(code, "%d", UPGRADE_JSON_ERR_NO_FINGERPRINT);
      ret_code = createJsonResponse(mqttUpgradeResponse, UPGRADE_JSON_BUFLEN, header_did, header_ts, header_requestId, header_version, status, code, description);
      debug_println(mqttUpgradeResponse);
      if (ret_code)
        publishAck(mqttUpgradeResponse);
      return;
    }
  }
#endif //#ifdef SECURE_UPGRADE  

  if (data_host && data_url && data_cmd)
  {
    filename = "http://" + String(data_host) + String(data_url);
    debug_print(F("Upgrade received. Filename: "));
    debug_println(filename);
  }
  else
  {
    debug_println(F("Missing host or url"));
    strcpy(status, "ko");
    strncpy(description, "missing host or url", 50);
    sprintf(code, "%d", UPGRADE_JSON_ERR_WRONG_HOST_URL);
    ret_code = createJsonResponse(mqttUpgradeResponse, UPGRADE_JSON_BUFLEN, header_did, header_ts, header_requestId, header_version, status, code, description);
    debug_println(mqttUpgradeResponse);
    if (ret_code)
      publishAck(mqttUpgradeResponse);
    return;
  }

  debug_println(F("*****************************************"));
  debug_println(F("     U P G R A D E   S T A R T E D"));
  debug_println(F("*****************************************"));

  ESPhttpUpdate.rebootOnUpdate(false);

  if (strcmp(data_cmd, "upgrade_app") == 0)
  {
    debug_println(F("Upgrading SPIFFS"));
    publishDebugWiFiTx("Upgrading Application");
    ret = ESPhttpUpdate.updateSpiffs(filename); //ret = ESPhttpUpdate.updateSpiffs(String("http://" + filename));  //http://server/spiffs.bin");
    //ret = ESPhttpUpdate.updateSpiffs(settings.mqtt_server, 80, filename.c_str());
  }
  else if (strcmp(data_cmd, "upgrade_wifi") == 0)
  {
    debug_println(F("Upgrading WiFi APP"));
    publishDebugWiFiTx("Upgrading WiFi");

    /**************/
    if (secure_upgrade)    //secure
    {
#ifdef SECURE_UPGRADE
      //upgrade http://smokuc47a.synology.me/ihub/iHub_wifi.ino.generic_v12.bin
      //upgrade https://storage.googleapis.com/dexma/firmwares/ihub/iHub_wifi.ino.generic_v12.bin
      //const char* host = "storage.googleapis.com";
      //const char* url = "/dexma/firmwares/ihub/iHub_wifi.ino.generic_v12.bin";
      //const char* fingerprint = "‎43 07 aa 1f a7 f8 8e 87 19 59 7e ff 05 bc b9 1b 67 47 c1 69";
      //const uint8_t httpsFingerprint[20] = {0x43, 0x07, 0xaa, 0x1f, 0xa7, 0xf8, 0x8e, 0x87, 0x19, 0x59, 0x7e, 0xff, 0x05, 0xbc, 0xb9, 0x1b, 0x67, 0x47, 0xc1, 0x69};
      //const int httpsPort = 443;

#if 0
      // Use WiFiClientSecure class to create TLS connection
      WiFiClientSecure secure_client;

      //const char* host = "www.howsmyssl.com";
      //const char* fingerprint = "‎7d 49 c8 33 24 39 b4 8b 4d 08 15 e3 47 76 d2 9d 82 80 6b 0c";


      // Use web browser to view and copy
      // SHA1 fingerprint of the certificate

      debug_print(F("connecting to "));
      debug_println(host);
      if (!secure_client.connect(host, httpsPort)) {
        debug_println(F("connection failed"));
        //return;
      }

      if (secure_client.verify(fingerprint, host)) {
        debug_println(F("certificate matches"));
      } else {
        debug_println(F("certificate doesn't match"));
        //return;
      }

#if 1
      //String url = "/a/check";
      String url = "/dexma/firmwares/ihub/";
      debug_print("requesting URL: ");
      debug_println(url);

      secure_client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" +
                          "User-Agent: ESP8266\r\n" +
                          "Connection: close\r\n\r\n");

      debug_println("request sent");
      while (secure_client.connected()) {
        String line = secure_client.readStringUntil('\n');
        if (line == "\r") {
          debug_println("headers received");
          break;
        }
      }
      String line = secure_client.readStringUntil('\n');
      debug_println("reply was:");
      debug_println("==========");
      debug_println(line);
      debug_println("==========");
      debug_println("closing connection");
#endif

#endif

      debug_print(F("Free Heap    : "));
      debug_println(system_get_free_heap_size());

      /***************/

      //ret = ESPhttpUpdate.update(filename, "", "‎43 07 aa 1f a7 f8 8e 87 19 59 7e ff 05 bc b9 1b 67 47 c1 69");
      DEBUG_UART.printf("Upgrade: %s%s, %d, %s\n\r", data_host, data_url, httpsPort, data_fingerprint);
      ret = ESPhttpUpdate.update(String(data_host), httpsPort, String(data_url), String(""), String(data_fingerprint));
      //t_httpUpdate_return ret = ESPhttpUpdate.update("https://username.github.io/firmware/blink.bin", "", "fingerprint_goes_here");
#endif //#ifdef SECURE_UPGRADE      
    } //secure
    else //non secure
    {
      ret = ESPhttpUpdate.update(filename);
      //ret = ESPhttpUpdate.update(settings.mqtt_server, 80, filename.c_str());
      //ret = ESPhttpUpdate.update(String("http://10.120.4.190/" + filename));
      //ret = ESPhttpUpdate.update("https://server/file.bin");
    }

  }//upgrade_wifi_app
  else //wrong command
  {
    strcpy(status, "ko");
    snprintf(description, 50, "wrong command:%s", data_cmd);
    sprintf(code, "%d", UPGRADE_JSON_ERR_WRONG_COMMAND);
    ret_code = createJsonResponse(mqttUpgradeResponse, UPGRADE_JSON_BUFLEN, header_did, header_ts, header_requestId, header_version, status, code, description);
    debug_println(mqttUpgradeResponse);
    if (ret_code)
      publishAck(mqttUpgradeResponse);
    return;
  }

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      debug_println(ESPhttpUpdate.getLastErrorString().c_str());
      strcpy(status, "ko");
      strncpy(description, ESPhttpUpdate.getLastErrorString().c_str(), 50);
      sprintf(code, "%d", UPGRADE_JSON_ERR_UPDATE_FAILED);
      ret_code = createJsonResponse(mqttUpgradeResponse, UPGRADE_JSON_BUFLEN, header_did, header_ts, header_requestId, header_version, status, code, description);
      debug_println(mqttUpgradeResponse);
      if (ret_code)
      {
        if (!mqtt_client.connected()) {
          debug_println(F("MQTT disconnected-reconnect"));
          mqttReconnect();
        }
        publishAck(mqttUpgradeResponse);
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      debug_println(F("HTTP_UPDATE_NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      debug_println(F("HTTP_UPDATE_OK"));
      debug_println(F("Upgrade OK. Restarting"));
      publishDebugWiFiTx("Upgrade OK");
      strcpy(status, "ok");
      ret_code = createJsonResponse(mqttUpgradeResponse, UPGRADE_JSON_BUFLEN, header_did, header_ts, header_requestId, header_version, status, code, description);
      debug_println(mqttUpgradeResponse);
      if (ret_code)
      {
        if (!mqtt_client.connected()) {
          debug_println(F("MQTT disconnected-reconnect"));
          mqttReconnect();
        }
        publishAck(mqttUpgradeResponse);
      }
      wifi_restart();
      break;
  }
}

/*
  {
  "header": {
  "deviceId": "+",
  "ts": "2019-01-10T10:21:52Z",
  "requestId": "3b1d98d3-7d17-4cda-97f9-ded7d3352cb3",
  "version": "1.0.0"
  },
  "data": {
  "status": "ko",
  "reason": {
  "code": "code",
  "description": "human-readable description"
  }
  }
  }
*/

/*
  {
  "header": {
          "did": "+",
          "ts": "2019-01-10T10:21:52Z",
          "requestId": "3b1d98d3-7d17-4cda-97f9-ded7d3352cb3",
          "version": "1.0.0",
          "type": "cmd"
  },
  "data": {
        "cmd": "upgrade_app",
        "host": "smokuc47a.synology.me",
        "url": "/ihub/ihub_v05.27.481-g2acd.bin"
  }
  }
*/
// {"header":{"deviceId":"starting9-01-10T10:21:52Z","ts":"ing9-01-10T10:21:52Z","requestId":"3b1d98d3-7d17-4cda-97f9-ded7d3352cb3","version":"1.0.0"},"data":{"status":"ok"}}

bool createJsonResponse(char *result, int buflen, const char *deviceId, const char *ts, const char *requestId, const char *version, const char *status, const char *code, const char *description)
{
  const size_t capacity = 3 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();

  JsonObject& header = root.createNestedObject("header");
  header["did"] = String(deviceId);
  header["ts"] = String(ts);
  header["requestId"] = String(requestId);
  header["version"] = String(version);

  JsonObject& data = root.createNestedObject("data");
  data["status"] = String(status);

  if (strncmp(status, "ok", 2) != 0) //if status ko, add code and description
  {
    JsonObject& data_reason = data.createNestedObject("reason");
    data_reason["code"] = String(code);
    data_reason["description"] = String(description);
  }

  debug_print(F("JSON Len:"));
  debug_println(root.measureLength());

  if (root.measureLength() > buflen)
  {
    debug_print(F("ERROR JSON Len > buffer, len:"));
    debug_println(root.measureLength());
    publishDebugWiFiTx("ERROR createJsonResponse: Len > buffer");
  }

  root.printTo(result, root.measureLength() + 1);
  return true;
}

void sendSettingsToSTM(char* settingsData)
{
  statistics.web_requests++;
  if (stm_settings_valid)
  {
    debug_println(F("Sending settings to STM"));
    serial_write(settingsData, sizeof(SettingsStruct_t), MEASUREMENTS_TYPE, SET_IHUB_SETTINGS);
  }
  else
    debug_println(F("ERROR cant Send settings to STM because we dont have valid settings!!!!!!!!"));
}

