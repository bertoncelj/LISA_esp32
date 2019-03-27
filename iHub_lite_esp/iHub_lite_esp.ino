#include "ihub.h"
//todo:naredi globalne counterje: vpisi v EEPROM...
#warning "CHECK MQTT BUFFER SIZE IN PUBSUBCLIENT"

extern int serial_semaphore;

static int debug_publish = 1;

String upgradeFile = "";
int32_t WifiSignalLevel;
int STM_bootloader_mode = 0;
int error_ntp = 0;

#ifdef MQTT_TLS_ENABLED
WiFiClientSecure client;
#else
WiFiClient client;
#endif //mqtt tls

PubSubClient mqtt_client(client);

Ticker ticker_1s; //1 second ticker
Ticker ticker_1m; //1 minute ticker

bool ticker_1s_Occured = false;
bool ticker_1m_Occured = false;
bool ticker_push_Occured = false; //it is a flag, not a ticker

#ifdef UDP_ENABLED
WiFiUDP UdpMiQen;
#endif

#ifdef TCP_ENABLED
// TCP Server on port 10001
WiFiServer TcpMiQen(MIQEN_TCP_PORT);

// Modbus TCP Server on port 502
WiFiServer ModbusTcp(MODBUS_TCP_PORT);
#endif

//even if WEB server is disabled, web upgrade is supported on: localhost/update
ESP8266WebServer HTTP(80);
ESP8266HTTPUpdateServer httpUpdater(true);

StoreStruct_t settings;
SettingsStruct_t stmSettings;
#if 0
StoreStruct_t settings = {
  CONFIG_VERSION,
  "iHubLite",
  DEFAULT_SERIAL,
  "Description",
  "Location",
  "10.96.0.149",
  "10.120.4.160",
  "1883",
  "600",
};
#endif
/****************************/
//MC globals
char device_type[16] = DEVICE_TYPE;
#ifdef PUSH_ENABLED
int log_id = 0;
int errPush = 0;
#endif
int push_interval;
unsigned short i_mqttPort;
int uptime = 0;
bool reset_after_receive_settings = false;
int ntp_status;
unsigned long NtpRefreshTime = 0;
unsigned long ntp_timestamp = 0;
long publishMeasureTime = 0;
bool stm_settings_valid = false;
int stm_setting_timestamp = 0;
bool publish_settings = false;
//=====================================
// RTC memory variables; retain values in sleep and restart
//=====================================
rtcMem_t rtcMem;
time_t UTC_time; //utc time or uptime when no ntp
time_t local_time;

int version_filesystem;

// Setup simpleDSTadjust Library rules
struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0};       // Central European Time = UTC/GMT +1 hour
simpleDSTadjust dstAdjusted(StartRule, EndRule);

statisticsStruct_t statistics;

/***********************************************************************************************/
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  debug_println(F("Should save config"));
  shouldSaveConfig = true;
}

void ticker_1m_handler() {
  ticker_1m_Occured = true;
}

void ticker_1s_handler() {
  ticker_1s_Occured =  true;
  //epoch++;
  uptime++;
}
/***********************************************************************************************/
void setup() {
  char serial_msg[20];
  bool production_mode = false;
  char apName[20];
  //startTime = millis();

  pinMode(D7, INPUT_PULLUP); //for PRODUCTION MODE
  //pinMode(D8, INPUT); //D8 Battery charging
  //pinMode(A0, INPUT);//batt voltage

  memset(&stmSettings, 0, sizeof(stmSettings));
  stm_settings_valid = false;

  STM_UART.begin(STM_UART_BAUDRATE);
  DEBUG_UART.begin(115200); //debug port

  //debug_print(F("Serial port 1 on "));
  //debug_println(ARDUINO_BOARD);

  //Serial.swap(); //na pine D7 in D8(GPIO 13-Rx in GPIO15-Tx)

  debug_println();
  debug_println();
  debug_print(F("Starting on "));
  debug_println(ARDUINO_BOARD);
  //Wire.begin(D1, D2); // sda, scl

  debug_print(F("SW Version: "));
  debug_println(SW_VERSION);
  for (int i = 0; i < 3; i++)
  {
    sprintf(serial_msg, "%d", SW_VERSION);
    serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_STARTED); //STM_UART.println(SW_VERSION);
    wait_for_serial_response(100);
    //delay(20);
  }

  DEBUG_UART.printf("Built on:%s %s\n\r", __DATE__, __TIME__);

  debug_print(F("ESP Core Version: "));
  debug_println(ESP.getCoreVersion());

  debug_print(F("ESP8266 Chip id = "));
  debug_println(ESP.getChipId());

  /* RTC memory */
  /*  for (unsigned int i = 0; i < sizeof(rtcData) / 4; i++) {
      rtcData[i] = i;
    }
    ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));
  */
#ifdef RTC_MEMORY
  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcMem, sizeof(rtcMem));
  uint32_t crcOfData = calculateCRC32(((uint8_t*) &rtcMem) + 4, sizeof(rtcMem) - 4);
  debug_print(F("CRC32 of data: "));
  DEBUG_UART.println(crcOfData, HEX); //debug_hexln(crcOfData);
  debug_print(F("CRC32 read from RTC: "));
  DEBUG_UART.println(rtcMem.crc32, HEX); //debug_hexln(rtcMem.crc32);
  if (crcOfData != rtcMem.crc32)
  {
    debug_println(F("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!"));
    debug_println(F("RTC Memory Read: "));
    printRTCMemory();
    memset((uint32_t*) &rtcMem, 0, sizeof(rtcMem));
    //InitRingBuffer();
    //resetMinMaxTemperature();//reset min, max values
    debug_println();
  }
  else {
    debug_println(F("CRC32 check ok, data is probably valid."));
    debug_println(F("RTC Memory Read: "));
    printRTCMemory();
    debug_println();
  }
#endif //RTC_MEMORY

  //read settings from EEPROM
  EEPROM.begin(512);
  int retry = 3;
  bool eepromConfig;
  while (retry--)
  {
    eepromConfig = loadConfig();
    if (eepromConfig == false)
    {
      debug_println(F("Settings in EEPROM are not valid. Sending request for settings!!!")); //just print, we will setup settings a little later
      serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_GET_WIFI_SETTINGS); //STM_UART.println(SW_VERSION);
      wait_for_serial_response(1000);
    }
    else
      break;//exit
  }

  reset_after_receive_settings = true; //from now on, reset after settings change

  //settings.serial[8] = '\0'; //just in case...
  debug_print(F("Settings in EEPROM "));
  debug_println(CONFIG_VERSION);
  debug_println(F("------------------------------------"));
  printSettings();
  debug_println(F("------------------------------------"));

  /* check if UPGRADE file is present */
  SPIFFS.begin();
  debug_println(F("Listing SPIFFS:"));
  version_filesystem = getFileSystemVersion();
  sprintf(serial_msg, "%d", version_filesystem);
  debug_print(F("Filesystem version: "));
  debug_println(serial_msg);
  serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_FILESYSTEM);
  wait_for_serial_response(100);

  listFilesystem(); //find upgrade file
  if (rtcMem.upgrade_counter < 5 && rtcMem.upgrade_counter > 0)
    upgradeMaster(upgradeFile.c_str());//do upgrade
  else
  {
    rtcMem.upgrade_counter = 0; //for next time
    saveRtcMem();
  }

  /* set AP name */
  if (strlen(settings.serial) == 8)
    sprintf(apName, "%s", settings.serial);
  else
    sprintf(apName, "IHL%08X", ESP.getChipId());

  /*****************************TLS*************************************************************************/
#ifdef MQTT_TLS_ENABLED
  // Load Certificates
  debug_print(F("Loading Certificates ..."));
  if (loadcerts() == 0)
    debug_println(F("OK"));
  else
    debug_println(F("ERROR"));
#endif
  /*********************************************************************************************************/

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
#ifdef MQTT_TLS_ENABLED
  WiFiManagerParameter custom_mqtt_server  ("mqtt_server", "MQTT server", settings.mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port    ("mqtt_port", "MQTT port", settings.strMqttPort, 8);
  WiFiManagerParameter custom_mqtt_topic   ("mqtt_topic", "MQTT topic", settings.mqtt_topic, 20);
#endif

  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();

  wifiManager.setAPCallback(configModeCallback);

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
#ifdef MQTT_TLS_ENABLED
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_topic);
#endif

  //zurb set timeout waiting for WiFi configuration
  wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT);
  wifiManager.setConfigPortalTimeout(WIFI_ACCESS_POINT_DURATION);

#ifdef D6_RESET_SETTINGS_INPUT
  pinMode(D6, INPUT_PULLUP); //key for reset settings

  //int buttonState = digitalRead(D6);
  //bool reset_settings = false;
  // check if the pushbutton is pressed.
  //if (digitalRead(D6) == LOW || eepromConfig == false || strlen(settings.wifi_ssid) == 0 ) {
  if (digitalRead(D6) == LOW || eepromConfig == false) {
    //char apName[20];
    debug_println(F("Reset Settings Button Pushed or wrong config"));
    //reset_settings = true;
    wifiManager.resetSettings();
    strcpy(settings.wifi_ssid, "");
    strcpy(settings.wifi_password, "");
    WiFi.hostname(apName);
    wifiManager.startConfigPortal(apName);
    yield(); //delay(100);
  }
#endif //d6pin   

  /* start with wifi connect procedure */
  unsigned char mac[6];
  WiFi.macAddress(mac);
  //sprintf(serial_msg,"%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  memcpy(serial_msg, mac, 6);
  serial_write(serial_msg, 6, WIFI_STATE_TYPE, STATE_CONNECTING); //STM_UART.println("CONNECTING");
  wait_for_serial_response(100);
  //delay(10);

  //Manual Wifi
  //WiFi.begin(WIFI_SSID, WIFI_PWD);
  //try to connect with ssid and password from settings

  debug_println(F("***********************************"));
  if (digitalRead(D7) == LOW) //production mode: connecting to predefined router
  {
    production_mode = true;
    debug_println(F("Connecting in Production MODE"));
    //WiFi.mode(WIFI_STA); //only station
    WiFi.mode(WIFI_AP_STA); //station and AP
    WiFi.hostname(apName);
    WiFi.begin(PRODUCTION_SSID, PRODUCTION_PASSWORD);
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      debug_print(F("Connected to "));
      debug_println(WiFi.SSID());
    }
    else
    {
      debug_println(F("Connection Timeout"));
      wifi_restart();
    }
  }
  else //normal connection
  {
    debug_println(F("Connecting with wifi manager")); //debug_println(F("Connect failed...let wifi manager take from here"));
    bool Connected;
    WiFi.hostname(apName);
    Connected = wifiManager.autoConnect(apName);
    if (Connected == false)
    {
      debug_println(F("Wifi Setup Timeout"));
      debug_println(F("RESTARTING !!!!!!!"));
      sprintf(serial_msg, "CONNECT TIMEOUT");
      serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_CONNECT_TOUT); //STM_UART.println("CONNECT TIMEOUT");
      wait_for_serial_response(100);
      delay(10);
      sprintf(serial_msg, "RESTARTING");
      serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_RESTARTING); //STM_UART.println("RESTARTING");
      wait_for_serial_response(100);

      wifi_restart();
    }
  }

  debug_println(F("***********************************"));
  WiFi.printDiag(DEBUG_UART);

  //if you get here you have connected to the WiFi
  delay(100);
  sprintf(serial_msg, "%s", WiFi.SSID().c_str());//send ssid
  serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_CONNECTED_SSID); //STM_UART.println("CONNECTED");
  wait_for_serial_response(1000);
  if (strcmp(WiFi.SSID().c_str(), settings.wifi_ssid))
  {
    debug_println("Saving wifi SSID:" + WiFi.SSID() + "settings:" + settings.wifi_ssid);
    //DEBUG_UART.printf("SSID len: %d settings len : %d\n\r", WiFi.SSID().length(), strlen(settings.wifi_ssid));
    strncpy(settings.wifi_ssid, WiFi.SSID().c_str(), 20);
    saveConfig();
  }
  delay(100);

  debug_print(F("PSK:"));
  sprintf(serial_msg, "%s", WiFi.psk().c_str());//send password
  debug_println(serial_msg);
  serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_CONNECTED_PASS); //STM_UART.println("CONNECTED");
  wait_for_serial_response(1000);
  if (strcmp(WiFi.psk().c_str(), settings.wifi_password))
  {
    debug_println(F("Saving wifi password!!!!!"));
    strncpy(settings.wifi_password, WiFi.psk().c_str(), 20);
    saveConfig();
  }
  delay(100);

  //read updated parameters
#ifdef MQTT_TLS_ENABLED
  strncpy(settings.mqtt_server, custom_mqtt_server.getValue(), 40);
  strncpy(settings.strMqttPort, custom_mqtt_port.getValue(), 8);
  strncpy(settings.mqtt_topic, custom_mqtt_topic.getValue(), 20);
#endif

  debug_println(F("Settings after connect:"));
  debug_println(F("------------------------------------"));
  printSettings();
  debug_println(F("------------------------------------"));

  if (shouldSaveConfig) {
    debug_println(F("Saving settings"));
    saveConfig();    
  }

  debug_print(F("WiFi SSID     : "));
  debug_println(WiFi.SSID());
  // Print the IP address
  debug_print(F("IP address    : "));
  debug_println(WiFi.localIP());

  //here was bug in version 10, push_interval was set only in wake mode
  //push_interval = atoi(settings.S_push_interval);
  push_interval = settings.mqtt_publish_interval;
  if (push_interval < 60 || push_interval > 7200)
  {
    debug_print(F("ERROR Push interval is: "));
    debug_println(push_interval);
    debug_println(F("Setting interval to 10 minutes!!!!!!!!!!"));
    push_interval = 600;
    statistics.errors++;
  }

  publishMeasureTime = rtcMem.last_timestamp + push_interval; //set publish time

  printTimestamp("Publish time  : ", publishMeasureTime);
  debug_println(F("------------------------------------"));

#ifdef UDP_ENABLED
  // Start the UDP server
  UdpMiQen.begin(MIQEN_UDP_PORT);
#endif

#ifdef TCP_ENABLED
  // Start the MODBUS TCP server
  ModbusTcp.begin(MODBUS_TCP_PORT);

  // Start the TCP server with port from settings
  debug_print(F("Starting Modbus TCP Server at port:"));
  if (settings.tcp_port == 0)
    settings.tcp_port = MIQEN_TCP_PORT; //use default MiQen port
  debug_println(settings.tcp_port);
  TcpMiQen.begin(settings.tcp_port); //this functionality is supported in bsp trunk after 2.4.0-rc2
#endif

  // Set Hostname.
  WiFi.hostname(apName);

  /* MQTT *************************************************************************/
  i_mqttPort = atoi(settings.strMqttPort);
#ifdef MQTT_TLS_ENABLED
  if (settings.mqtt_enabled == 0 && i_mqttPort != 0)
    settings.mqtt_enabled = 1; //enable MQTT in TLS version, because we can't change settings on WEB page

  if (i_mqttPort == 1883)
    i_mqttPort = 8883; //set port to default MQTT TLS port
#endif

  if (settings.mqtt_enabled)
  {
#ifdef MQTT_TLS_ENABLED
    //verifytls(); //verify if server is secure. Disable it for now, since it establishes connection to MQTT.
#endif
    //sprintf(inTopic, "%s/%s/cmd", settings.mqtt_topic, settings.serial);
    //i_mqttPort = atoi(settings.strMqttPort);
    mqtt_client.setServer(settings.mqtt_server, i_mqttPort);
    mqtt_client.setCallback(mqtt_callback);

    //debug_println(F("MQTT connecting..."));
    mqttReconnect();
  }
  /* MQTT *************************************************************************/

#ifdef WEB_SERVER_ENABLED
  setup_web_server();
#else //minimum web server handler
  HTTP.on ( "/", []() { HTTP.send ( 200, "text/html", SITE_index );  } );
  HTTP.on ( "/settings", getSettingsData );
  HTTP.on ( "/mqtt_settings_parse", HTTP_GET, mqtt_settings_parse);  
  HTTP.on ( "/reset_settings", []() {
    HTTP.send ( 200, "text/html", clear_wifi_settings_page );
  } );
  HTTP.on ( "/clear_wifi_settings", clear_wifi_settings );
  //HTTP.on ( "/reboot", wifi_restart );
  HTTP.on ( "/reboot", []() {
    HTTP.sendHeader("Location", "/update", true); //Redirect to update page
    HTTP.send(302, "text/plane", "");
    wifi_restart();
  } );
  HTTP.onNotFound([]() {
    HTTP.sendHeader("Location", "/update", true); //Redirect to update page
    HTTP.send(302, "text/plane", "");
  });

  httpUpdater.setup(&HTTP); //web upgrade
  HTTP.begin();
#endif //WEB_SERVER_ENABLED

  debug_println(F("Starting SSDP..."));
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(settings.serial);
  SSDP.setSerialNumber(settings.serial);
#ifdef MQTT_TLS_ENABLED
  SSDP.setURL("/update");
#else
  SSDP.setURL("index.html");
#endif
  SSDP.setModelName("iHUB-L1");
  SSDP.setModelNumber("IHL100");
  SSDP.setModelURL("http://www.iskra.eu");
  SSDP.setManufacturer("Iskra d.d.");
  SSDP.setManufacturerURL("http://www.iskra.eu");
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.begin();

  debug_println(F("HTTP Ready!"));

  if (!MDNS.begin(apName)) {             // Start the mDNS responder for serial_number.local
    debug_println(F("Error setting up MDNS responder!"));
    statistics.errors++;
  }
  else
  {
    debug_println(F("mDNS responder started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "iHUB-Lite", "HTTP Server");
    MDNS.addServiceTxt("http", "tcp", "Type", String(DEVICE_TYPE).c_str());
    MDNS.addServiceTxt("http", "tcp", "SW Version", String(SW_VERSION).c_str());
    MDNS.addServiceTxt("http", "tcp", "HW Version", String(HW_VERSION).c_str());
#ifndef MQTT_TLS_ENABLED
    MDNS.addService("modbusTCP", "tcp", 502);
    MDNS.addServiceTxt("modbusTCP", "tcp", "iHUB-Lite", "Modbus TCP Server");
#endif

    //28.1.2019: dont start discover at startup, because it takes too much time (10 seconds)
    //mdns_discover();//discover wifi temp sensors on local network
  }

  //yield(); //delay(200);

  debug_println(F("Gettings STM Settings"));
  serial_write("", 0, WIFI_STATE_TYPE, STATE_GET_STM_SETTINGS); //todo:naredi brez state, ker stm nima tega state
  wait_for_serial_response(1000);
  delay(300);

  String ipaddress = WiFi.localIP().toString();
  sprintf(serial_msg, "%s", ipaddress.c_str() );
  debug_println(F("Sending IP"));
  if (production_mode == false)
    serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_READY);  //send IP address
  else
    serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_READY_PRODUCTION);  //send IP address and production mode

  wait_for_serial_response(1000);
  //delay(10);

  if (settings.time_sync_src == NTP_ENABLED)
    ntp_status = NTP_NOT_SYNC;
  else
    ntp_status = NTP_DISABLED;

  //ticker.attach(10, ticker_handler);
  ticker_1m.attach(60, ticker_1m_handler);
  ticker_1s.attach(1, ticker_1s_handler);

  delay(100);

  //try to connecto to mqtt again if it fails for the 1st time
  if (settings.mqtt_enabled && !mqtt_client.connected()) {
    debug_println(F("MQTT disconnected-reconnect"));
    mqttReconnect();
    yield(); //delay(100);
  }

  sprintf(serial_msg, "Started v.%d", SW_VERSION);
  publishDebugWiFiTx(serial_msg);
}//end setup

/***********************************************************************************************/
/****************       M A I N     L O O P     ************************************************/
/***********************************************************************************************/
void loop()
{
  if (STM_UART.available() )
  {
    //debug_println("Serial receive");
    parse_serial();
  }

  //get STM settings in case we dont have valid stm settings
  if (stm_settings_valid == false && (millis() - stm_setting_timestamp > 30000)) //prevent getting settings too frequently if there is no regular response
  {
    stm_setting_timestamp = millis();
    debug_println(F("Gettings STM Settings"));
    serial_write("", 0, WIFI_STATE_TYPE, STATE_GET_STM_SETTINGS);
    wait_for_serial_response(1000);
  }

#ifdef MQTT_TLS_ENABLED
  //send settings to STM because MQTT settings was changed on Captive portal
  if(shouldSaveConfig && stm_settings_valid && (millis() - stm_setting_timestamp > 5000))
  {
    stm_setting_timestamp = millis(); //we can reuse timestamp
    strcpy(stmSettings.mqtt_server, settings.mqtt_server);
    stmSettings.mqtt_port = i_mqttPort;
    strcpy(stmSettings.mqtt_topic, settings.mqtt_topic);
    sendSettingsToSTM((char*)&stmSettings);
    wait_for_serial_response(1000);    
  }
#endif  

  if (STM_bootloader_mode == 0)//if STM is in bootloader, dont do TCP, UDP communications
  {
#ifdef TCP_ENABLED
    TCPServer();
    TCPServer_502();
#endif

#ifdef UDP_ENABLED
    int UDPpacketSize = UdpMiQen.parsePacket();
    if (UDPpacketSize)
      UDP_Reply();
#endif
  }

  if (settings.mqtt_enabled)
  {
    if (mqtt_client.connected())
      mqtt_client.loop();
  }

  HTTP.handleClient(); //if web server is disabled, then it is only for updater

  if (serial_semaphore && (millis() - serial_semaphore > 2000))//millis rollover fixed
    serial_semaphore = 0;

  /************************every second ************************************/
  if (ticker_1s_Occured)
  {
    ticker_1s_Occured = false;

    //start ntp
    if (settings.time_sync_src == NTP_ENABLED && (NtpRefreshTime == 0 || millis() - NtpRefreshTime > (NTP_REFRESH_PERIOD * 1000)))
    {
      debug_println(F("Updating time ..."));
      updateNTP();    //start ntp
      if (ntp_status != NTP_NOT_SYNC) //dont update timestamp if previous ntp update failed.
        ntp_timestamp = millis();
      yield(); //delay(100);
      //debug_println("Done");

      NtpRefreshTime = millis();
      ntp_status = NTP_NOT_SYNC;
    }

    if (ntp_status == NTP_NOT_SYNC && (time(nullptr) > 1500000000)) //ntp time received, 5.9.2018:time needs to be checked for reasonable value
    {
      int i;
      debug_print(F("NTP received after: "));
      debug_println(millis() - ntp_timestamp);
      ntp_timestamp = 0; //we have ntp time
      ntp_status = NTP_SYNC;

      //send time to STM
      UTC_time = time(NULL);
      if (settings.time_dst) //daylight saving time
        local_time = dstAdjusted.time(NULL) + settings.timezone * 60;
      else
        local_time = time(NULL) + settings.timezone * 60;

      char *utc_time_to_send = (char *)&UTC_time;  //char *time_to_send = (char *)&local_time; //zurb 25.10.2018: send UTC time to STM
      char *local_time_to_send = (char *)&local_time;
      char time_to_send[8];
      for (i = 0; i < 4; i++)
        time_to_send[i] = utc_time_to_send[i];
      for (i = 0; i < 4; i++)
        time_to_send[i + 4] = local_time_to_send[i];
      //sprintf(time_to_send, "%x%x",UTC_time,local_time);
      serial_write(time_to_send, 8, INTERNAL_TYPE, INTERNAL_TIME);  //send time to STM

      //here we get time for the first time. Check if last_timestamp is not in the future
      if (rtcMem.last_timestamp > UTC_time) {
        debug_print(F("Last timestamp is in future: "));
        debug_println(rtcMem.last_timestamp);
        rtcMem.last_timestamp = UTC_time;
        saveRtcMem();
        publishMeasureTime = rtcMem.last_timestamp + push_interval; //update publish time
        yield();
      }
    }

    //ntp timeout
    if (ntp_status == NTP_NOT_SYNC && millis() - ntp_timestamp > (3 * NTP_REFRESH_PERIOD * 1000)) //ntp timeout after 3 ntp requests failed
    {
      char serial_msg[20];
      debug_println(F("NTP TIMEOUT"));
      debug_println(F("RESTARTING !!!!!!!"));
      sprintf(serial_msg, "RESTARTING:NTP");
      serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_RESTARTING);
      wifi_restart();
    }

    //update time
    if (time(nullptr)) //if we have ntp time, use it
    {
      UTC_time = time(NULL);
      if (settings.time_dst) //daylight saving time
        local_time = dstAdjusted.time(NULL) + settings.timezone * 60;
      else
        local_time = time(NULL) + settings.timezone * 60;
    }
    else
    {
      UTC_time++; //just add one second
      local_time = UTC_time  + settings.timezone * 60;;
    }

    //check if measure interval is expired
    if (( UTC_time >= publishMeasureTime || (publishMeasureTime > UTC_time + push_interval * 2) ) && uptime > 60) //5.9.2018:wait one minute for valid time
    {
      if (settings.mqtt_enabled)
        ticker_push_Occured = true;      //triger publish
    }

    yield();

  }//ticker 1s
  /************************every second ************************************/

  /************************every minute ************************************/
  if (ticker_1m_Occured)
  {
    char serial_msg[20];
    char mqtt_msg[100];

    ticker_1m_Occured = false;

    //zurb mqtt mqttReconnect every minute
    if (settings.mqtt_enabled)
    {
      if (!mqtt_client.connected()) {
        errMqttIncrement();
        debug_println(F("MQTT disconnected-reconnect"));
        mqttReconnect();
        yield(); //delay(100);
      }
      //publish settings after power up and after change of settings
      if (publish_settings == true && stm_settings_valid == true && mqtt_client.connected())
      {
        debug_println(F("Publish Settings"));
        //delay(1000);
        publishSettingsRequest();
        wait_for_serial_response(1000);
      }
    }

    if (debug_publish)
    {
      debug_println(F("----------------- 1 minute ticker ----------------------"));
      printTimestamp("Local time  : ", local_time);
      printTimestamp("UTC time    : ", UTC_time);
      debug_print(F("Free Heap: "));
      debug_println(system_get_free_heap_size());
      debug_print(F("Serial RX : ")); debug_println(statistics.serial_rx_packets);
      debug_print(F("Serial TX : ")); debug_println(statistics.serial_tx_packets);
      debug_print(F("TCP    RX : ")); debug_println( statistics.tcp_rx_packets);
      debug_print(F("TCP    TX : ")); debug_println( statistics.tcp_tx_packets);
      debug_print(F("Web Req.  : ")); debug_println( statistics.web_requests);
      debug_print(F("Bootloader: ")); debug_println( STM_bootloader_mode);
      debug_print(F("ERRORS    : ")); debug_println( statistics.errors);
    }

    memset(serial_msg, 0, 20);
    debug_print(F("Uptime : "));
    printUptime(serial_msg);
    sprintf(mqtt_msg, "%s Uptime: %s heap: %d WiFi signal level: ", settings.serial, serial_msg, system_get_free_heap_size());

    if (debug_publish)
    {
      debug_println(serial_msg);
    }
    //serial_write(serial_msg, strlen(serial_msg), INTERNAL_TYPE, INTERNAL_UPTIME); //STM_UART.println(uptime_str);

    WifiSignalLevel = getWifiQuality();
    sprintf(serial_msg, "%d", WifiSignalLevel);
    strcat(mqtt_msg, serial_msg);
    if (debug_publish)
    {
      debug_print(F("WiFi signal level: "));
      debug_println(serial_msg);
    }

    serial_write(serial_msg, strlen(serial_msg), INTERNAL_TYPE, INTERNAL_WIFI_SIGNAL);
    publishDebugWiFiTx(mqtt_msg);

#ifdef MDNS_DISCOVERY_ENABLED
    int ret = getDataFromWifiTempSensors();
    if (ret)
    {
      debug_println(F("Reading WiFi Temp Sensors Failed."));
      //numberOfWiFiDevices = 0;
    }
#endif

  }//ticker_1m
  /************************every minute ************************************/

  //***************** push interval *************************************
  if (ticker_push_Occured) {
    ticker_push_Occured = false;

    if (debug_publish)
      debug_println(F("----------------- Publish Measurements -----------------"));

    //struct tm * timeinfo = localtime(&UTC_time);
    if (debug_publish)
    {
      char mqtt_msg[100];
      sprintf(mqtt_msg, "Publish after: %ld", UTC_time - rtcMem.last_timestamp);
      publishDebugWiFiTx(mqtt_msg);

      printTimestamp("Last publish: ", rtcMem.last_timestamp);
      debug_print(F("Seconds since last publish: "));
      debug_println(UTC_time - rtcMem.last_timestamp);
    }

    rtcMem.last_timestamp = UTC_time;

    publishMeasureTime += push_interval;
    if (publishMeasureTime < UTC_time || publishMeasureTime > UTC_time + push_interval)
      publishMeasureTime = UTC_time + push_interval;

    if (debug_publish)
    {
      printTimestamp("Local time  : ", local_time);
      printTimestamp("UTC time    : ", UTC_time);
      printTimestamp("Publish time: ", publishMeasureTime);
    }

#ifdef PUSH_ENABLED
    push();
    log_id++;
#endif //PUSH

    //zurb 21.9.2017: moved mqtt out of valid temp, so it can connect to mqtt
    if (settings.mqtt_enabled)
    {
      if (mqtt_client.connected())
      {
        publishCountersRequest();
        wait_for_serial_response(1000);
        publishMeasurementsRequest();
        wait_for_serial_response(1000);
        publishStatusRequest();
        wait_for_serial_response(1000);
      }//connected
      else
        debug_println(F("MQTT disconnected-publish"));
    }

    saveRtcMem(); //update rtc memory
  }//ticker_push

  yield(); //delay(100);
} //loop

//- See more at: http://www.esp8266.com/viewtopic.php?f=29&t=10698#sthash.RreQIVwN.dpuf

/************************* END OF LOOP **************************************************/

// Called if WiFi has not been configured yet
void configModeCallback (WiFiManager *myWiFiManager) {
  debug_println(F("Wifi Manager"));
  debug_print(F("Please connect to: "));
  debug_println(myWiFiManager->getConfigPortalSSID());
  debug_println(F("To setup Wifi"));
  serial_write("AP MODE", 7, WIFI_STATE_TYPE, STATE_AP_MODE);
}

void updateNTP() {

  //int retry = 10;
  //configTime(UTC_OFFSET * 3600, 0, settings.NtpServer);
  //configTime(settings.timezone * 60, 0, settings.NtpServer1, settings.NtpServer2, settings.NtpServer3);
  configTime(0, 0, settings.NtpServer1, settings.NtpServer2, settings.NtpServer3);  //25.10.2018: We need UTC time
}

/* ****************** EEPROM FUNCTIONS ***************/
bool loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
  {
    for (unsigned int t = 0; t < sizeof(settings); t++)
      *((char*)&settings + t) = EEPROM.read(CONFIG_START + t);
    return true;
  }
  else
  {
    debug_print(F("Wrong config version: "));
    debug_println(CONFIG_VERSION);
    DEBUG_UART.printf("Read: %x %x %x\n\r", EEPROM.read(CONFIG_START + 0), EEPROM.read(CONFIG_START + 1), EEPROM.read(CONFIG_START + 2));
    return false;
  }
}

void saveConfig() {
  settings.version[0] = CONFIG_VERSION[0];
  settings.version[1] = CONFIG_VERSION[1];
  settings.version[2] = CONFIG_VERSION[2];
  settings.version[3] = 0;

  settings.crc = calculateCRC32((uint8_t*)&settings, sizeof(settings) - 4); //todo:crc

  for (unsigned int t = 0; t < sizeof(settings); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&settings + t));

  EEPROM.commit();
}
/* ****************** EEPROM FUNCTIONS ***************/

void printRTCMemory() {
  //char buf[6];

  debug_print(F("CRC32: "));
  DEBUG_UART.println(rtcMem.crc32, HEX); //debug_hexln(rtcMem.crc32);
#if 0
  debug_print(F("sRingBufCnt: "));
  debug_println(rtcMem.sRingBufCnt);

  debug_print(F("sRingBufPut: "));
  debug_println(rtcMem.sRingBufPut);

  debug_print(F("sRingBufGet: "));
  debug_println(rtcMem.sRingBufGet);
#endif

  debug_print(F("Last timestamp : "));
  debug_println(rtcMem.last_timestamp);

  debug_print(F("Upgrade counter : "));
  debug_println(rtcMem.upgrade_counter);
}

unsigned short swap16(unsigned short num)
{
  return (num >> 8) | (num << 8);
}

unsigned long swap32(unsigned long num)
{
  unsigned long  swapped =
    ((num >> 24) & 0xff) | // move byte 3 to byte 0
    ((num << 8) & 0xff0000) | // move byte 1 to byte 2
    ((num >> 8) & 0xff00) | // move byte 2 to byte 1
    ((num << 24) & 0xff000000); // byte 0 to byte 3
  return swapped;
}

void saveRtcMem()
{
  rtcMem.crc32 = calculateCRC32(((uint8_t*) &rtcMem) + 4, sizeof(rtcMem) - 4);
  //debug_print("Save RtcMem CRC : ");
  //debug_hexln(rtcMem.crc32);
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcMem, sizeof(rtcMem));
}

void printTimestamp(const char *string, unsigned long timestamp)
{
  char time_str[50];
  time_t time_to_string = timestamp;
  struct tm * timeinfo = localtime(&time_to_string);
  sprintf(time_str, "%s%d-%02d-%02dT%02d:%02d:%02d", string, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  debug_println(time_str);
}

void setStringTerminator(char *str, int len)
{
  int i;

  str[len] = 0; //all settings has 1 extra byte for terminator, so start wih it

  for (i = len - 1; i >= 0; i--)
  {
    if (str[i] == ' ' || str[i] == 0) //go from end and replace spaces with 0
      str[i] = 0;
    else
      break;    //exit at nonspace char
  }
}

void printSettings()
{
  setStringTerminator(settings.wifi_ssid, 20);
  debug_print(F("Wifi SSID     : "));
  debug_println(settings.wifi_ssid);

  setStringTerminator(settings.wifi_password, 20);
  debug_print(F("Wifi password : "));
  debug_println(settings.wifi_password);

  setStringTerminator(settings.device_type, 16);
  debug_print(F("Device type   : "));
  debug_println(settings.device_type);

  setStringTerminator(settings.serial, 8);
  debug_print(F("Serial number : "));
  debug_println(settings.serial);

  setStringTerminator(settings.location, 40);
  debug_print(F("Location      : "));
  debug_println(settings.location);

  setStringTerminator(settings.description, 40);
  debug_print(F("Description   : "));
  debug_println(settings.description);

  debug_print(F("TCP port      : "));
  debug_println(settings.tcp_port);

  setStringTerminator(settings.NtpServer1, 40);
  debug_print(F("NTP server 1  : "));
  debug_println(settings.NtpServer1);

  setStringTerminator(settings.NtpServer2, 40);
  debug_print(F("NTP server 2  : "));
  debug_println(settings.NtpServer2);

  setStringTerminator(settings.NtpServer3, 40);
  debug_print(F("NTP server 3  : "));
  debug_println(settings.NtpServer3);

  setStringTerminator(settings.mqtt_server, 40);
  debug_print(F("MQTT server   : "));
  debug_println(settings.mqtt_server);

  setStringTerminator(settings.strMqttPort, 8);
  debug_print(F("MQTT port     : "));
  debug_println(settings.strMqttPort);

  setStringTerminator(settings.mqtt_username, 16);
  debug_print(F("MQTT username : "));
  debug_println(settings.mqtt_username);

  setStringTerminator(settings.mqtt_password, 16);
  debug_print(F("MQTT password : "));
  debug_println(settings.mqtt_password);

  setStringTerminator(settings.mqtt_topic, 20);
  debug_print(F("MQTT topic    : "));
  debug_println(settings.mqtt_topic);

  debug_print(F("MQTT enabled  : "));
  debug_println(settings.mqtt_enabled);

  debug_print(F("Pub. interval : "));
  debug_println(settings.mqtt_publish_interval);

  debug_print(F("NTP enabled   : "));
  debug_println(settings.time_sync_src);

  debug_print(F("Timezone off  : "));
  debug_println(settings.timezone);

  debug_print(F("DST           : "));//daylight saving time
  debug_println(settings.time_dst);

  debug_print(F("CRC           : "));
  debug_hexln(settings.crc);
}

void printUptime(char *time_str)
{
  const long minute = 60;
  const long hour = minute * 60;
  const long day = hour * 24;
  snprintf (time_str, 20, "%ldd %02ld:%02ld", uptime / day, (uptime % day) / hour, (uptime % hour) / minute);

  //debug_print("Uptime: ");
  //debug_print(uptime / day);
  //debug_print("d ");
  //debug_print( (uptime % day) / hour);
  //debug_print("h ");
  //debug_print( (uptime % hour) / minute);
  //debug_println("m");
}

// converts the dBm to a range between 0 and 100%
int32_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  return dbm;
#if 0
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
#endif
}

int getFileSystemVersion()
{
  File f = SPIFFS.open("/version.txt", "r");
  if (!f) {
    debug_println(F("Version file open failed"));
    return -1;
  }

  String version = f.readString();
  debug_print(F("Filesystem version: "));
  debug_println(version);

  return strtol(version.c_str(), NULL, 0);
}

#ifdef MQTT_TLS_ENABLED
// Load Certificates
int loadcerts() {

  if (!SPIFFS.begin()) {
    debug_println(F("Failed to mount file system"));
    return -1;
  }

  // Load client certificate file from SPIFFS
  //openssl x509 -in client.crt -outform PEM -out client.pem
  //openssl x509 -outform der -in client.pem -out client.der
  File cert = SPIFFS.open("/client.der", "r"); //replace esp.der with your uploaded file name
  if (!cert)
    debug_println(F("Failed to open cert file"));

  delay(100);

  // Set client certificate
  if (!client.loadCertificate(cert)) {
    debug_println(F("cert not loaded"));
    return -2;
  }

  // Load client private key file from SPIFFS
  //openssl rsa -inform pem -outform der -in client.key -out client-key.der
  File private_key = SPIFFS.open("/client-key.der", "r"); //replace espkey.der with your uploaded file name
  if (!private_key) {
    debug_println(F("Failed to open private cert file"));
    return -3;
  }

  delay(100);

  // Set client private key
  if (!client.loadPrivateKey(private_key)) {
    debug_println(F("private key not loaded"));
    return -4;
  }

#if 0
  // Load CA file from SPIFFS
  //openssl x509 -in CA.crt -outform PEM -out ca.pem
  //openssl x509 -outform der -in ca.pem -out ca.der
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca.der with your uploaded file name
  if (!ca) {
    debug_println(F("Failed to open ca "));
    return -5;
  }

  delay(100);

  // Set server CA file
  if (!client.loadCACert(ca)) {
    debug_println(F("ca failed"));
    return -6;
  }
#endif

  return 0;
}

/*
   We can verify certificate of MQTT broker
*/
void verifytls() {
  // Fingerprint of the broker CA
  //openssl x509 -in 10.120.10.36.crt -sha1 -noout -fingerprint
  const char* fingerprint = "12:FA:F1:DD:00:E5:95:EE:E4:84:E2:18:1C:2D:16:9F:39:A4:B5:95";

  // Use WiFiClientSecure class to create TLS connection
  debug_print("connecting to ");
  debug_println(settings.mqtt_server);
  //client.setFingerprint((uint8_t *)fingerprint);
  if (!client.connect(settings.mqtt_server, 8883)) {
    debug_println("connection failed");
    return;
  }

  if (client.verify(fingerprint, settings.mqtt_server)) {
    debug_println("certificate matches");
  } else {
    debug_println("certificate doesn't match");
  }
}
#endif //mqtt tls

#ifndef WEB_SERVER_ENABLED
void clear_wifi_settings() {
  debug_println(F("WIFI Reset settings"));
  HTTP.sendHeader("Location", "/update", true); //Redirect to update page
  HTTP.send(302, "text/plane", "");
  delay(1000);
  //HTTP.send(200, "text/plain", "");  // Send it back to the browser with an HTTP status 303 (See Other) to redirect
  serial_write("", 0, INTERNAL_TYPE, RESET_WIFI_SETTINGS);
}

void getSettingsData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, IHUB_SETTINGS);
  wait_for_serial_response(1000);
}

void mqtt_settings_parse()
{
  debug_print(F("settings parse args: "));
  debug_println(HTTP.args());
  int i;
  for (i = 0; i < HTTP.args(); i++)
  {
    debug_print(F("i:"));
    debug_println(i);

    debug_print(F("arg:"));
    debug_println(HTTP.arg(i));

    debug_print(F("argName:"));
    debug_println(HTTP.argName(i));

  }

  if (HTTP.hasArg("mqtt_server"))
  {
    strncpy(stmSettings.mqtt_server, HTTP.arg("mqtt_server").c_str(), 40);
  }
  if (HTTP.hasArg("mqtt_port"))
  {
    stmSettings.mqtt_port = HTTP.arg("mqtt_port").toInt();
  }
  if (HTTP.hasArg("mqtt_username"))
  {
    strncpy(stmSettings.mqtt_username, HTTP.arg("mqtt_username").c_str(), 16);
  }
  if (HTTP.hasArg("mqtt_password"))
  {
    strncpy(stmSettings.mqtt_password, HTTP.arg("mqtt_password").c_str(), 16);
  }
  if (HTTP.hasArg("mqtt_topic"))
  {
    strncpy(stmSettings.mqtt_topic, HTTP.arg("mqtt_topic").c_str(), 20);
  }
  if (HTTP.hasArg("mqtt_publish_interval"))
  {
    stmSettings.mqtt_publish_interval = HTTP.arg("mqtt_publish_interval").toInt();
  }

  sendSettingsToSTM((char*)&stmSettings);

  String message = "Settings received\n";
  HTTP.send(200, "text/plain", message);
  debug_println(message);

  return;
}
#endif //WEB_SERVER_ENABLED

void wifi_restart()
{
  delay(3000);
  ESP.restart();
}

