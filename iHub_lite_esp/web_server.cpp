#include "ihub.h"

#ifdef WEB_SERVER_ENABLED
extern ESP8266WebServer HTTP;
extern ESP8266HTTPUpdateServer httpUpdater;
extern StoreStruct_t settings;
extern SettingsStruct_t stmSettings;
extern time_t UTC_time;
extern time_t local_time;
extern int32_t WifiSignalLevel;
extern int version_filesystem;
extern bool stm_settings_valid;
extern int STM_bootloader_mode;
extern wifiTempSensor_t tempSensor[5];
extern int numberOfWiFiDevices;

void getCounterData();
void getMeasurementsData();
bool handleFileRead(String path);
void handleFileCreate();
void handleFileDelete();
void handleFileUpload();
void handleFileList();
void getSettingsData();
void getWifiData();
void getWifiDevices();
void getStatusData();
void getRamLoggerData();
void setBicomCommand();
void getBicomState();
void getDemandsData();
//void handleGraph();
void drawGraphRussian();
void drawGraphEnglish();
void handleSendConfig();
void settingsParse();
void getStmStatistics();
void getDeviceSettingsData();
void getRS485Devices();
void scanRS485Bus();
void detectWifiSensors();

File fsUploadFile;

#ifndef MQTT_TLS_ENABLED
int power_buf[96];
int power_samples = 0;
int power_max_value;
int power_min_value;
int total_power;
#endif

const char* www_username = "admin";
const char* www_password = "iskra";

void setup_web_server()
{
  debug_println(F("Starting HTTP..."));

  //HTTP.serveStatic("/", SPIFFS, "/index.html");  // root is linked to index.html
  HTTP.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });

//todo web password
#if 0
  HTTP.on("/", []() {
    if (!HTTP.authenticate(www_username, www_password)) {
      return HTTP.requestAuthentication();
    }
    HTTP.send(200, "text/plain", "Login OK");
  });
#endif  

  HTTP.on ( "/settings", getSettingsData );
  HTTP.on ( "/wifi_data", getWifiData );
  HTTP.on ( "/status", getStatusData );
  HTTP.on ( "/detect_rs485", scanRS485Bus );  
  HTTP.on ( "/get_rs485_devices", getRS485Devices );
  HTTP.on ( "/stm_statistics", getStmStatistics );
  HTTP.on ( "/reset_settings", []() {
    HTTP.send ( 200, "text/html", clear_wifi_settings_page );
  } );
  HTTP.on ( "/clear_wifi_settings", clear_wifi_settings );
  HTTP.on ( "/reboot", wifi_restart );
  HTTP.on ( "/bicom_state", getBicomState );
  HTTP.on ( "/bicom_command", HTTP_POST, setBicomCommand);
  HTTP.on ( "/settings_parse", HTTP_GET, settingsParse);

#ifndef MQTT_TLS_ENABLED
  HTTP.on ( "/counters", getCounterData );
  HTTP.on ( "/measurements", getMeasurementsData );
  HTTP.on ( "/demands", getDemandsData );  
  HTTP.on ( "/device_settings", getDeviceSettingsData );    
  HTTP.on ( "/power_rus.svg", drawGraphRussian );
  HTTP.on ( "/power.svg", drawGraphEnglish );
#endif
  
#ifdef MDNS_DISCOVERY_ENABLED
  HTTP.on ( "/wifi_devices", getWifiDevices );
  HTTP.on ( "/detect_wifi_sensors", detectWifiSensors );
#endif  

  //list directory
  HTTP.on("/list", HTTP_GET, handleFileList);
  //load editor
  HTTP.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) HTTP.send(404, "text/plain", "FileNotFound");
  });
  //create file
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  HTTP.on("/edit", HTTP_POST, []() {
    HTTP.send(200, "text/plain", "");
  }, handleFileUpload);

  HTTP.on ( "/sendConfig", handleSendConfig );

  //called when the url is not defined here
  //use it to load content from SPIFFS
  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
    {
      // HTTP.send(404, "text/plain", "FileNotFound");
      HTTP.sendHeader("Location", "/update", true); //Redirect to our html web page
      HTTP.send(302, "text/plane", "");
    }
  });

  httpUpdater.setup(&HTTP); //web upgrade
  HTTP.begin();
}

String getContentType(String filename) {
  if (HTTP.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  debug_println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    HTTP.sendHeader("cache-control", "max-age=3600");  //send header to use cached files
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (HTTP.uri() != "/edit") return;
  HTTPUpload& upload = HTTP.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    debug_print(F("handleFileUpload Name: "));
    debug_println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //debug_print(F("handleFileUpload Data: ")); debug_println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    debug_print(F("handleFileUpload Size: "));
    debug_println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (HTTP.args() == 0) return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  debug_println("handleFileDelete: " + path);
  if (path == "/")
    return HTTP.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return HTTP.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  HTTP.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (HTTP.args() == 0)
    return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  debug_println("handleFileCreate: " + path);
  if (path == "/")
    return HTTP.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return HTTP.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return HTTP.send(500, "text/plain", "CREATE FAILED");
  HTTP.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    HTTP.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = HTTP.arg("dir");
  debug_println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  HTTP.send(200, "text/json", output);
}

#ifndef MQTT_TLS_ENABLED
void getCounterData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, COUNTERS);
  wait_for_serial_response(1000);
}

void getMeasurementsData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, MEASUREMENTS);
  wait_for_serial_response(1000);
}

void getDemandsData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, MAXIMUM_DEMANDS);
  wait_for_serial_response(1000);
}

void getDeviceSettingsData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, DEVICE_SETTINGS);
  wait_for_serial_response(1000);
}

void getRamLoggerData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, RAM_LOGGER);
  wait_for_serial_response(1000);
}
#endif //MQTT_TLS_ENABLED

void getSettingsData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, IHUB_SETTINGS);
  wait_for_serial_response(1000);
}

void getStatusData() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, STATUS);
  wait_for_serial_response(1000);
}

void getBicomState() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, BICOM_STATE);
  wait_for_serial_response(1000);
}

void setBicomCommand() {                          // If a POST request is made to URI /bicom_on
  statistics.web_requests++;
  debug_print(F("GET:"));
  debug_println(HTTP.arg("command"));
  if (HTTP.arg("command") == "ON")
    serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_ON);
  else if (HTTP.arg("command") == "OFF")
    serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_OFF);
  else if (HTTP.arg("command") == "TOGGLE")
    serial_write("", 0, MEASUREMENTS_TYPE, IR_BICOM_TOGGLE);
  else if (HTTP.arg("command") == "485_1_ON")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_ON);
  else if (HTTP.arg("command") == "485_1_OFF")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_OFF);
  else if (HTTP.arg("command") == "485_1_TOGGLE")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_1_TOGGLE);
  else if (HTTP.arg("command") == "485_2_ON")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_ON);
  else if (HTTP.arg("command") == "485_2_OFF")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_OFF);
  else if (HTTP.arg("command") == "485_2_TOGGLE")
    serial_write("", 0, MEASUREMENTS_TYPE, BICOM_485_2_TOGGLE);

  HTTP.send(204, "text/plain", "");    //16.3.2018: send response to POST, so we dont need redirection

#if 0 //16.3.2018: removed page redirection, because 204 response is OK      
  HTTP.sendHeader("Location", "/bicom.html"); // Add a header to respond with a new location for the browser to stay on bicom page
  HTTP.send(303);                            // Send it back to the browser with an HTTP status 303 (See Other) to redirect
#endif
}

void getStmStatistics() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, GET_STM_STATISTICS);
  wait_for_serial_response(1000);
}

void handleLogin() {                         // If a POST request is made to URI /login
  if ( ! HTTP.hasArg("username") || ! HTTP.hasArg("password")
       || HTTP.arg("username") == NULL || HTTP.arg("password") == NULL) { // If the POST request doesn't have username and password data
    HTTP.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  if (HTTP.arg("username") == "John Doe" && HTTP.arg("password") == "password123") { // If both the username and the password are correct
    HTTP.send(200, "text/html", "<h1>Welcome, " + HTTP.arg("username") + "!</h1><p>Login successful</p>");
  } else {                                                                              // Username and password don't match
    HTTP.send(401, "text/plain", "401: Unauthorized");
  }
}

void scanRS485Bus() {
  statistics.web_requests++;
  HTTP.send(204, "text/plain", "");    //16.3.2018: send response to POST, so we dont need redirection
  serial_write("", 0, MEASUREMENTS_TYPE, SCAN_RS485_BUS);
  wait_for_serial_response(1000);
}

void getRS485Devices() {
  statistics.web_requests++;
  serial_write("", 0, MEASUREMENTS_TYPE, GET_RS485_DEVICES);
  wait_for_serial_response(1000);
}

void detectWifiSensors()
{
  HTTP.send(204, "text/plain", "");    //send response to POST, so we dont need redirection
  mdns_discover();
}

void getWifiData() {
  char time_str[50];
  char uptime_str[20];
  char ver_str[10];
  char mac_addr[20];
  //char time_sync_str[20];
  char cpu_mode[15];

  struct tm * timeinfo = localtime(&local_time);

  sprintf(time_str, "%02d.%02d.%d %02d:%02d:%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  memset(uptime_str, 0, 20);
  printUptime(uptime_str);

  sprintf(ver_str, "%d.%02d", SW_VERSION / 100, SW_VERSION % 100);

  unsigned char mac[6];
  WiFi.macAddress(mac);
  sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  if (STM_bootloader_mode)
    sprintf(cpu_mode, "0"); //sprintf(cpu_mode, "Bootloader");
  else
    sprintf(cpu_mode, "1"); //sprintf(cpu_mode, "Application");

#if 0
  if (settings.time_sync_src == 0)
    sprintf(time_sync_str, "No synchronisation");
  else
    sprintf(time_sync_str, "Ethernet NTP");

  HTTP.send ( 200, "application/json", "{\"serial\":\"" + String(settings.serial) + "\", \"ssid\":\"" + String(WiFi.SSID()) + "\", \"mac_address\":\"" + String(mac_addr) + "\", \"wifi_signal_level\":\"" + String(WifiSignalLevel)
              + " dBm\", \"description\":\"" + settings.description + "\", \"location\":\"" + settings.location + "\", \"time_sync\":\"" + time_sync_str + "\", \"timezone\":\"" + String(settings.timezone) + "\", \"fs_ver\":\"" + String(version_filesystem)
              + "\", \"device_type\":\"" + settings.device_type + "\", \"sw_version\":\"" + ver_str + "\", \"ntp_ip_address1\":\"" + settings.NtpServer1  + "\", \"ntp_ip_address2\":\"" + settings.NtpServer2  + "\", \"ntp_ip_address3\":\"" + settings.NtpServer3
              + "\", \"mqtt_ip_address\":\"" + settings.mqtt_server  + "\", \"mqtt_port\":\"" + settings.strMqttPort + "\", \"mqtt_topic\":\"" + settings.mqtt_topic + "\", \"mqtt_publish_time\":\"" + String(settings.mqtt_publish_interval) + " s\", \"mqtt_enabled\":\"" + String(settings.mqtt_enabled)
              + "\", \"tcp_port\":\"" + String(settings.tcp_port) + "\", \"ip_address\":\"" + WiFi.localIP().toString() + "\", \"uptime\":\"" + uptime_str + "\", \"timestamp\":\"" + time_str
              + "\"}" );
#else //without settings, just statuses and statistics
  HTTP.send ( 200, "application/json", "{\"serial\":\"" + String(settings.serial) + "\", \"ssid\":\"" + String(WiFi.SSID()) + "\", \"mac_address\":\"" + String(mac_addr)
              + "\", \"wifi_signal_level\":\"" + String(WifiSignalLevel) + " dBm\", \"description\":\"" + settings.description + "\", \"location\":\"" + settings.location +
              "\", \"fs_ver\":\"" + String(version_filesystem) + "\", \"device_type\":\"" + settings.device_type + "\", \"sw_version\":\"" + ver_str
              + "\", \"ip_address\":\"" + WiFi.localIP().toString()
              + "\", \"errors\":\"" + statistics.errors + "\", \"tcp_rx_packets\":\"" + statistics.tcp_rx_packets + "\", \"tcp_tx_packets\":\"" + statistics.tcp_tx_packets + "\", \"web_requests\":\"" + statistics.web_requests
              + "\", \"serial_tx_packets\":\"" + statistics.serial_tx_packets + "\", \"serial_rx_packets\":\"" + statistics.serial_rx_packets + "\", \"wifi_heap\":\"" + system_get_free_heap_size()
              + "\", \"uptime\":\"" + uptime_str + "\", \"cpu_mode\":\"" + cpu_mode + "\", \"timestamp\":\"" + time_str
              + "\"}" );
#endif
}

#ifdef MDNS_DISCOVERY_ENABLED
void getWifiDevices()
{
  int j;
  char data[1000];
  char tmp[200];

  strcpy(data, "{\"data\":[");
  for (j = 0; j < numberOfWiFiDevices; j++)
  {
    if (j > 0)
      strcat(data, ",");

    sprintf(tmp, "{\"Serial\":\"%s\",\"Description\":\"%s\",\"Location\":\"%s\",\"model\":\"%s\",\"Temperature\":\"%s\",\"Humidity\":\"%s\",\"Pressure\":\"%s\",\"IP_Address\":\"%s\",\"Timestamp\":\"%s\"}",
            tempSensor[j].serial, tempSensor[j].description, tempSensor[j].location, tempSensor[j].device_type, tempSensor[j].temperature, tempSensor[j].humidity, tempSensor[j].pressure, tempSensor[j].IP, tempSensor[j].timestamp);
    strcat(data, tmp);
    // debug_println(data);
  }
  strcat(data, "]}");
  HTTP.send ( 200, "application/json", data);
}
#endif

#if 0
void clear_wifi_settings() {
  HTTP.send(200, "text/html", "Resetting WiFi Settings<br>Are you sure?<br><form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Clear WiFi settings\"></form><form action=\"/\" method=\"POST\"><input type=\"submit\" value=\"NO, Back to main page\"></form>");
}
#else
void clear_wifi_settings() {
  debug_println(F("WIFI Reset settings"));
  HTTP.sendHeader("Location", "/"); // Add a header to respond with a new location for the browser to go to the home page
  HTTP.send(303);
  //HTTP.send(200, "text/plain", "");  // Send it back to the browser with an HTTP status 303 (See Other) to redirect
  serial_write("", 0, INTERNAL_TYPE, RESET_WIFI_SETTINGS);
}
#endif

#ifndef MQTT_TLS_ENABLED
void drawGraph(int language) {
#define NUM_BARS (24*4)
#define BAR_WIDTH 8
#define GRAPH_WIDTH (NUM_BARS*BAR_WIDTH)
#define GRAPH_HIGHT (GRAPH_WIDTH/3)
#define GRAPH_X_START 65
#define GRAPH_Y_START 10
#define MAX_INPUT_VAL (250*80*3)
#define LEGEND_X_START (GRAPH_X_START + 250)
#define LEGEND_Y_START (GRAPH_Y_START + GRAPH_HIGHT + 50)

  getRamLoggerData(); //get ramlogger data for graph

  String out = "";
  char temp[150];
  int i;
  sprintf(temp, "<svg xmlns='http://www.w3.org/2000/svg' version='1.1' width='%d' height='%d'>\n", GRAPH_WIDTH + 100, GRAPH_HIGHT + 200);
  out += temp;
  sprintf(temp, "<rect x='%d' y='%d' width='%d' height='%d' fill='rgb(242, 242, 242)' stroke-width='1' stroke='rgb(0, 0, 0)' />\n", GRAPH_X_START, GRAPH_Y_START, GRAPH_WIDTH, GRAPH_HIGHT);
  out += temp;
  out += "<g stroke=\"black\">\n";

  //calculate height of graph
  int graph_max_value = 0;
  int graph_min_value = 0;

  int graph_height = power_max_value - power_min_value; //complete height
  //round up min, max
  if (graph_height < 1000)
  {
    if (power_max_value)
      graph_max_value = ((power_max_value / 100) + 1) * 100;
    if (power_min_value)
      graph_min_value = ((power_min_value / 100) - 1) * 100;
  }
  else if (graph_height < 10000)
  {
    if (power_max_value)
      graph_max_value = ((power_max_value / 1000) + 1) * 1000;
    if (power_min_value)
      graph_min_value = ((power_min_value / 1000) - 1) * 1000;
  }
  else
  {
    if (power_max_value)
      graph_max_value = ((power_max_value / 10000) + 1) * 10000;
    if (power_min_value)
      graph_min_value = ((power_min_value / 10000) - 1) * 10000;
  }

  if (graph_min_value < -60000)
    graph_min_value = -60000;

  if (graph_max_value > 60000)
    graph_max_value = 60000;

  //update height to rounded min max
  graph_height = graph_max_value - graph_min_value; //rounded height

  debug_print(F("Power max  : "));
  debug_println(power_max_value);
  debug_print(F("Power min  : "));
  debug_println(power_min_value);
  debug_print(F("Graph max scale: "));
  debug_println(graph_max_value);
  debug_print(F("Graph min scale: "));
  debug_println(graph_min_value);
  debug_print(F("Graph height: "));
  debug_println(graph_height);
  debug_print(F("No of samples: "));
  debug_println(power_samples);

  if (power_samples < 2)
  {
    if (language == RUSSIAN) //russian
      sprintf(temp, "<text x='%d' y='%d'>Недостаточно данных. Подождите 30 минут после включения.</text>\n", GRAPH_X_START + 200, GRAPH_HIGHT / 2); //y text
    else
      sprintf(temp, "<text x='%d' y='%d'>Not enough data. Wait for 30 minutes after powerup</text>\n", GRAPH_X_START + 200, GRAPH_HIGHT / 2); //y text
    out += temp;
  }

  //trenutni bar je: ure*4 + minute/4
  time_t time_bar = local_time;
  struct tm * timeinfo = localtime(&time_bar);
  int start_bar = timeinfo->tm_hour * 4 + (timeinfo->tm_min) / 15 - 1;
  debug_print(F("Start bar:"));
  debug_println(start_bar);

  //#define SVG_BAR_CHART
#ifdef SVG_BAR_CHART

  char *color;
  char *color1 = {"rgb(77,  166, 255)"};
  char *color2 = {"rgb(204, 204, 204)"};
  color = color1;

  for (i = 0; i < power_samples; i++)
  {
    int curr_bar = start_bar - i; //from right to left
    if (curr_bar < 0) //yesterday
    {
      curr_bar += NUM_BARS;
      color = color2; //different color for yesterday events
    }
    //DEBUG_UART.printf("BAR:%d val:%d\n\r",curr_bar, power_buf[i]);
    int y = (power_buf[i] * GRAPH_HIGHT) / graph_height; //int y = rand() % GRAPH_HIGHT - 20;
    int x = GRAPH_X_START + curr_bar * BAR_WIDTH;
    sprintf(temp, "<rect x='%d' y='%d' fill='%s' width='%d' height='%d'></rect>\n", x, GRAPH_HIGHT - y + GRAPH_Y_START, color, BAR_WIDTH - 1, y);
    out += temp;
  }
#else //LINE CHART    
  if (power_samples > 1 && graph_height)
  {
    int rollover = 0;
    int graph_zero = graph_min_value * GRAPH_HIGHT / graph_height;
    debug_print(F("Graph_zero px: "));
    debug_println(graph_zero);
    debug_print(F("Graph_max px: "));
    debug_println(GRAPH_HIGHT);
    debug_print(F("Graph_min px: "));
    debug_println(GRAPH_Y_START);

    //draw todays time line
    sprintf(temp, "<rect x='%d' y='%d' width='%d' height='%d' fill='rgb(128, 204, 255)' stroke-width='1' stroke='rgb(0, 0, 0)' />\n", GRAPH_X_START, GRAPH_Y_START, (start_bar + 1) * BAR_WIDTH, GRAPH_HIGHT);
    out += temp;
    //sprintf(temp, "<polyline points=\"20,20 40,25 60,40 80,120 120,140 200,180\" ");
    sprintf(temp, "<polyline points=\"");
    out += temp;
    for (i = 0; i < power_samples; i++)
    {
      int curr_bar = start_bar - i; //from right to left
      if (curr_bar < 0) //yesterday
      {
        curr_bar += NUM_BARS;  //move to right
        if (rollover == 0)
        {
          rollover = 1;
          out += "\" "; //end of points
          sprintf(temp, "style=\"fill:none;stroke:black;stroke-width:3\"/>");
          out += temp;
          sprintf(temp, "<polyline points=\""); //start new line
          out += temp;
        }
      }
      if (power_buf[i] >= 0)
        sprintf(temp, "%d,%d", GRAPH_X_START + (curr_bar + 1) * BAR_WIDTH, GRAPH_HIGHT + GRAPH_Y_START - ((power_buf[i] * GRAPH_HIGHT) / graph_height) + graph_zero);
      else
        sprintf(temp, "%d,%d", GRAPH_X_START + (curr_bar + 1) * BAR_WIDTH, GRAPH_HIGHT + GRAPH_Y_START - ((((graph_min_value - power_buf[i]) * graph_zero) / abs(graph_min_value))));
      out += temp;
      if (i != power_samples - 1)
        out += ",";
      else
        out += "\" "; //end of points
    }
    sprintf(temp, "style=\"fill:none;stroke:black;stroke-width:3\"/>");
    out += temp;
  }
#endif

  //vodoravne crte
  char strval[10];
  int divider = 1000; //scale in kWatts
  char unit[3];

  if (graph_height < 1000) //scale in Watts
  {
    strcpy(unit, "W");
    divider = 1;
  }
  else
    strcpy(unit, "kW");

  for (i = 0; i < 4; i++)
  {
    int precision = 0;

    sprintf(temp, "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke-width='1' stroke='rgb(217, 217, 217)' />\n", GRAPH_X_START - 5 , (4 - i) * (GRAPH_HIGHT / 4) + GRAPH_Y_START, GRAPH_WIDTH + GRAPH_X_START, (4 - i) * (GRAPH_HIGHT / 4) + GRAPH_Y_START);
    out += temp;

    if (graph_height * i % (4 * divider))
      precision = 2;
    dtostrf(((graph_max_value * i + (graph_min_value * (4 - i))) / (float)(4 * divider)), 4, precision, strval);
    sprintf(temp, "<text x='2' y='%d'>%s %s</text>\n", (4 - i) * (GRAPH_HIGHT / 4) + 5 + GRAPH_Y_START, strval, unit); //x text
    out += temp;
  }
  dtostrf(graph_max_value / (float)divider, 4, 0, strval);
  sprintf(temp, "<text x='2' y='%d'>%s %s</text>\n", 0 + 5 + GRAPH_Y_START, strval, unit); //x max value
  out += temp;

  //x os text
  for (i = 0; i < GRAPH_WIDTH + 10; i += GRAPH_WIDTH / 12)
  {
    sprintf(temp, "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke-width='1' stroke='rgb(0, 0, 0)' />\n", i + GRAPH_X_START , GRAPH_HIGHT + GRAPH_Y_START - 5, i + GRAPH_X_START, GRAPH_HIGHT + GRAPH_Y_START + 5);
    out += temp;
    sprintf(temp, "<text x='%d' y='%d'>%d:00</text>\n", i < (5 * GRAPH_WIDTH / 12) ? i + GRAPH_X_START - 11 : i + GRAPH_X_START - 18, GRAPH_HIGHT + 30, i / 32); //y text
    out += temp;
  }

  //legend
  sprintf(temp, "<rect x='%d' y='%d' width='%d' height='%d' fill='rgb(128, 204, 255)' stroke-width='1' stroke='rgb(0, 0, 0)' />\n", LEGEND_X_START, LEGEND_Y_START, 50, 20);
  out += temp;
  if (language == RUSSIAN) //russian
    sprintf(temp, "<text x='%d' y='%d'>Сегодня</text>\n", LEGEND_X_START + 60, LEGEND_Y_START + 15);
  else //english
    sprintf(temp, "<text x='%d' y='%d'>Today</text>\n", LEGEND_X_START + 60, LEGEND_Y_START + 15);
  out += temp;
  sprintf(temp, "<rect x='%d' y='%d' width='%d' height='%d' fill='rgb(204, 204, 204)' stroke-width='1' stroke='rgb(0, 0, 0)' />\n", LEGEND_X_START + 140, LEGEND_Y_START, 50, 20);
  out += temp;
  if (language == RUSSIAN) //russian
    sprintf(temp, "<text x='%d' y='%d'>Вчера</text>\n", LEGEND_X_START + 200, LEGEND_Y_START + 15);
  else
    sprintf(temp, "<text x='%d' y='%d'>Yesterday</text>\n", LEGEND_X_START + 200, LEGEND_Y_START + 15);
  out += temp;
  dtostrf(total_power / (float)1000, 4, 2, strval);
  if (language == RUSSIAN) //russian
    sprintf(temp, "<text x='%d' y='%d'>Потребление энергии за последние 24 часа: %s kWh</text>\n", LEGEND_X_START, LEGEND_Y_START + 55, strval);
  else
    sprintf(temp, "<text x='%d' y='%d'>Total energy in last 24 hours: %s kWh</text>\n", LEGEND_X_START, LEGEND_Y_START + 55, strval);
  out += temp;

  //#define DEBUG_GRAPH
#ifdef DEBUG_GRAPH
  sprintf(temp, "<text x='%d' y='%d'>Pmax:%d Graph Max:%d Graph Height:%d Unit:%s</text>\n", LEGEND_X_START - 100, LEGEND_Y_START + 75, power_max_value, graph_max_value, graph_height, unit);
  out += temp;
#endif

  out += "</g>\n</svg>\n";

  DEBUG_UART.printf("Graph len:%d heap:%d\n\r", out.length(), system_get_free_heap_size());
  HTTP.send ( 200, "image/svg+xml", out);
}

void drawGraphEnglish()
{
  drawGraph(ENGLISH);
}

void drawGraphRussian()
{
  drawGraph(RUSSIAN);
}
#endif //#ifndef MQTT_TLS_ENABLED

void handleSendConfig()
{
  if (HTTP.hasArg("plain") == false) { //Check if config received

    HTTP.send(200, "text/plain", "Config not received");
    return;
  }

  String message = "Config received:\n";
  message += HTTP.arg("plain");
  message += "\n";

  HTTP.send(200, "text/plain", message);
  debug_println(message);
}


void settingsParse()
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
  //General settings
  if (HTTP.hasArg("description"))
  {
    strncpy(stmSettings.description, HTTP.arg("description").c_str(), 40);
  }
  if (HTTP.hasArg("location"))
  {
    strncpy(stmSettings.location, HTTP.arg("location").c_str(), 40);
  }
  if (HTTP.hasArg("timezone"))
  {
    stmSettings.timezone = HTTP.arg("timezone").toInt();//todo preveri vrednosti settingov
  }
  if (HTTP.hasArg("time_sync_src"))
  {
    stmSettings.time_sync_src = HTTP.arg("time_sync_src").toInt();
  }
  if (HTTP.hasArg("time_dst"))//daylight saving time
  {
    stmSettings.time_dst = HTTP.arg("time_dst").toInt();//todo preveri vrednosti settingov
  }
  if (HTTP.hasArg("ntp_server1"))
  {
    strncpy(stmSettings.ntp_server1, HTTP.arg("ntp_server1").c_str(), 40);
  }
  if (HTTP.hasArg("ntp_server2"))
  {
    strncpy(stmSettings.ntp_server2, HTTP.arg("ntp_server2").c_str(), 40);
  }
  if (HTTP.hasArg("ntp_server3"))
  {
    strncpy(stmSettings.ntp_server3, HTTP.arg("ntp_server3").c_str(), 40);
  }

  //Communication settings
  if (HTTP.hasArg("ihub_modbus_address"))
  {
    stmSettings.ihub_modbus_address = HTTP.arg("ihub_modbus_address").toInt();
  }
  if (HTTP.hasArg("tcp_port"))
  {
    stmSettings.tcp_port = HTTP.arg("tcp_port").toInt();
  }
  if (HTTP.hasArg("wifi_ssid"))
  {
    strncpy(stmSettings.wifi_ssid, HTTP.arg("wifi_ssid").c_str(), 20);
  }
  if (HTTP.hasArg("wifi_password"))
  {
    strncpy(stmSettings.wifi_password, HTTP.arg("wifi_password").c_str(), 20);
  }
  if (HTTP.hasArg("mqtt_enabled"))
  {
    stmSettings.mqtt_enabled = HTTP.arg("mqtt_enabled").toInt();
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

  //IR Settings
  if (HTTP.hasArg("ir_external_relay_mode"))
  {
    stmSettings.ir_external_relay_mode = HTTP.arg("ir_external_relay_mode").toInt();
  }
  if (HTTP.hasArg("ir_energy_counter_enabled"))
  {
    stmSettings.ir_energy_counter_enabled = HTTP.arg("ir_energy_counter_enabled").toInt();
  }
  if (HTTP.hasArg("ir_energy_counter_address"))
  {
    stmSettings.ir_energy_counter_address = HTTP.arg("ir_energy_counter_address").toInt();
  }
  if (HTTP.hasArg("ir_relay_description"))
  {
    strncpy(stmSettings.ir_external_relay_description, HTTP.arg("ir_relay_description").c_str(), 20);
  }

  //RS485 Settings
  if (HTTP.hasArg("rs485_baud_rate"))
  {
    stmSettings.rs485_baud_rate = HTTP.arg("rs485_baud_rate").toInt();
  }
  if (HTTP.hasArg("rs485_parity"))
  {
    stmSettings.rs485_parity = HTTP.arg("rs485_parity").toInt();
  }
  if (HTTP.hasArg("rs485_stop_bits"))
  {
    stmSettings.rs485_stop_bits = HTTP.arg("rs485_stop_bits").toInt();
  }
  if (HTTP.hasArg("rs485_device_type_1"))
  {
    stmSettings.serial_network[0].device_type = HTTP.arg("rs485_device_type_1").toInt();
  }
  if (HTTP.hasArg("rs485_modbus_address_1"))
  {
    stmSettings.serial_network[0].modbus_address = HTTP.arg("rs485_modbus_address_1").toInt();
  }
  if (HTTP.hasArg("rs485_description_1"))
  {
    strncpy(stmSettings.serial_network[0].description, HTTP.arg("rs485_description_1").c_str(), 20);
  }
  if (HTTP.hasArg("rs485_device_type_2"))
  {
    stmSettings.serial_network[1].device_type = HTTP.arg("rs485_device_type_2").toInt();
  }
  if (HTTP.hasArg("rs485_modbus_address_2"))
  {
    stmSettings.serial_network[1].modbus_address = HTTP.arg("rs485_modbus_address_2").toInt();
  }
  if (HTTP.hasArg("rs485_description_2"))
  {
    strncpy(stmSettings.serial_network[1].description, HTTP.arg("rs485_description_2").c_str(), 20);
  }

  sendSettingsToSTM((char*)&stmSettings);

  String message = "Settings received\n";
  HTTP.send(200, "text/plain", message);
  debug_println(message);

  return;
}

//
//https://arduinojson.org/assistant/
//

#ifndef MQTT_TLS_ENABLED
void parseJsonPowerData(char *json)
{
  int i;

  debug_println(F("parseJsonPowerData"));  

  const size_t bufferSize = JSON_ARRAY_SIZE(96) + JSON_OBJECT_SIZE(5) + 80;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  //debug_print(F("Free Heap: "));
  //debug_println(system_get_free_heap_size());

  //const char* json = "{\"parameter\":\"19\",\"interval\":\"15\",\"samples\":\"4\",\"timestamp\":\"65531\",\"data\":[162,162,162,114]}";

  JsonObject& root = jsonBuffer.parseObject(json);

  //debug_print(F("Free Heap: "));
  //debug_println(system_get_free_heap_size());

  const char* parameter = root["parameter"]; // "19"
  const char* interval = root["interval"]; // "15"
  const char* samples = root["samples"]; // "4"
  const char* timestamp = root["timestamp"]; // "65531"

  if (samples)
    power_samples = atoi(samples);
  else
    power_samples = 0;

  debug_print(F("Number of samples:"));
  debug_println(power_samples);

  if (power_samples > 96 || power_samples < 1)
  {
    debug_print(F("ERROR: Samples count:"));
    debug_println(power_samples);
    return;
  }

  JsonArray& data = root["data"];
  //debug_print(F("Free Heap: "));
  //debug_println(system_get_free_heap_size());
  //int data0 = data[0]; // 162
  //int data1 = data[1]; // 162
  //int data2 = data[2]; // 162
  //int data3 = data[3]; // 114

  total_power = 0;
  power_max_value = 0;
  power_min_value = 0;
  for (i = 0; i < power_samples; i++)
  {
    power_buf[i] = data[i];
    int tmp_power = data[i];
    //tmp_power /= 4;   //to kWh because we use 15 minute intervals
    total_power += tmp_power;
    if (data[i] > power_max_value) //save max value
      power_max_value = data[i];
    if (data[i] < power_min_value) //save min value
      power_min_value = data[i];
  }

  total_power /= 4; //to kWh because we use 15 minute intervals
  debug_print(F("Total power: "));
  debug_print(total_power);
  debug_println(F(" Wh"));
  debug_print(F("Power max: "));
  debug_println(power_max_value);
}
#endif //#ifndef MQTT_TLS_ENABLED
#endif //web_server_enabled

