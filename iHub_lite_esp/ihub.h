#ifndef ihub_h
#define ihub_h

#define SW_VERSION 48
#define HW_VERSION 1

#define PRODUCTION_SSID "Tsenzor"
#define PRODUCTION_PASSWORD "mer2senzor_temp33"

/* Secure MQTT TLS */
//#define MQTT_TLS_ENABLED

/* If MQTT TLS is enabled due to lack of RAM, some functionalities must be disabled */
#ifndef MQTT_TLS_ENABLED //if TLS is enabled, we need to use as little RAM as possible
#define WEB_SERVER_ENABLED
#define TCP_ENABLED
#define UDP_ENABLED
#define MDNS_DISCOVERY_ENABLED
//#define PUSH_ENABLED
#endif

//#define TCP_DEBUGING

//Serial is connected to ST32L073, it will be named STM_UART
#define STM_UART Serial
#define STM_UART_BAUDRATE 230400

#define DEBUG_UART Serial1

//debug is serial 1
#define DEBUG_IHL
#ifdef DEBUG_IHL
#define debug_print(msg) Serial1.print(msg)
#define debug_hex(msg) Serial1.print(msg, HEX)
#define debug_println(msg) Serial1.println(msg)
#define debug_hexln(msg) Serial1.println(msg, HEX)
#else
#define debug_print(msg)
#define debug_hex(msg)
#define debug_println(msg)
#define debug_hexln(msg)
#endif

#define D6_RESET_SETTINGS_INPUT
//#ifdef ARDUINO_ESP8266_ESP01
//for 2.4.1
#ifdef ARDUINO_ESP8266_GENERIC
#define D6 12
#define D7 13
#endif

#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#ifdef UDP_ENABLED
#include <WiFiUdp.h>
#endif
#include <Ticker.h>
#include <ArduinoJson.h>
#include <simpleDSTadjust.h>
#include <ESP8266SSDP.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "FS.h"

//#define PUSH
#define RTC_MEMORY

#define MODBUS_TCP_PORT 502
#define MIQEN_TCP_PORT 10001
#define MIQEN_UDP_PORT 33333

#define DEVICE_TYPE "iHUB-L1"

#include <PubSubClient.h>
#define MQTT_MAX_ERR 20

#define NTP_DISABLED 0
#define NTP_ENABLED 1
#define NTP_NOT_SYNC 2
#define NTP_SYNC 3

//#define UTC_OFFSET 1 
//#define NTP_SERVERS "10.96.0.149"
#define NTP_REFRESH_PERIOD 300 
#define UPTIME_REFRESH_PERIOD 60

//#define I2C_Discover

//EEPROM COFIG Data
#define CONFIG_START 0
#define CONFIG_VERSION "v02"
#define DEFAULT_SERIAL "IHL00000"

#define ENGLISH 0
#define RUSSIAN 1

typedef struct StoreStruct {
  // This is for mere detection if they are your settings
  char version[4];
  char wifi_ssid[21];  
  char wifi_password[21];
  char device_type[17];  
  char serial[9];    
  char description[41];
  char location[41];
  char NtpServer1[41];
  char NtpServer2[41];
  char NtpServer3[41];
  char mqtt_server[41];
  char strMqttPort[9];
  char mqtt_username[17];
  char mqtt_password[17];
  char mqtt_topic[21];
  unsigned short time_sync_src;
  signed short timezone;
  unsigned short time_dst; //24.9.2018 daylight saving time
  unsigned short mqtt_enabled;
  unsigned short mqtt_publish_interval;
  unsigned short tcp_port;
  unsigned long crc;
} StoreStruct_t;

//settings for STM, needed for WEB settings
typedef struct SerialNetworkStruct {          
  unsigned short device_type;      
  unsigned short modbus_address;      
  unsigned short description_type;
  char description[20];   
} SerialNetworkStruct_t;

typedef struct SettingsStruct {    
  unsigned short external_relay_command; //40015
  char description[40]; //40101
  char location[40];    //40121    
  short timezone;           //40141 
  unsigned short time_sync_src; //40142
  unsigned short time_dst; //24.9.2018 daylight saving time
  unsigned short tcp_port;      //40201     
  unsigned short ihub_modbus_address; //40202  
  char mqtt_server[40];   //40207 - 40226
  unsigned short mqtt_port; //40227
  char mqtt_username[16];   //40228 - 40235
  char mqtt_password[16];   //40236 - 40243
  char mqtt_topic[20];   //40244 - 40253
  char ntp_server1[40]; //40301 - 40320
  char ntp_server2[40]; //40321 - 40340
  char ntp_server3[40]; //40341 - 40360
  char wifi_ssid[20];   //40361 - 40370
  char wifi_password[20]; //40371 - 40380
  unsigned short ir_external_relay_mode; //40401
  unsigned short ir_energy_counter_enabled; //40402
  unsigned short ir_energy_counter_address; //40403
  unsigned short mqtt_enabled; //40255
  unsigned short mqtt_publish_interval; //40254
  /* rs485 */  
  unsigned short rs485_is_debug_port; //40421 only for debug
/*
0  Baud rate 1200
1 Baud rate 2400
2 Baud rate 4800
3 Baud rate 9600
4 Baud rate 19200
5 Baud rate 38400
6 Baud rate 57600
7 Baud rate 115200
*/  
  unsigned short rs485_baud_rate; //40422
  unsigned short rs485_stop_bits; //40423
  unsigned short rs485_parity;    //40424
  unsigned short rs485_data_bits; //40425
  
  SerialNetworkStruct_t serial_network[2]; //41001, 41011 2 for now
  char ir_external_relay_description[20]; 
  
} SettingsStruct_t;

//RTC RAM Data
#define CRINGBUFCOUNT   200
typedef struct rtcMem
{
  uint32_t crc32;
  //int      sRingBufCnt;                // Number of populated fields
  //int      sRingBufPut;                // Input index address
  //int      sRingBufGet;                // Output index address
  //short    sRingBuf[CRINGBUFCOUNT];    // Ring Buffer    todo:sparamo ram
  long     last_timestamp;
  int      upgrade_counter;
} rtcMem_t;

typedef struct statisticsStruct {        
  int errors;  
  int tcp_rx_packets;
  int tcp_tx_packets;
  int serial_tx_packets;
  int serial_rx_packets;
  int web_requests;  
} statisticsStruct_t;

typedef struct wifiTempSensor {  
  char IP[20];
  char device_type[17];
  char serial[9];
  char description[41];
  char location[41];
  char temperature[5];
  char humidity[5];
  char pressure[5];
  char timestamp[11];
} wifiTempSensor_t;

#define BOOTLOADER_MODE 1
#define WIFI_CONNECT_TIMEOUT 60
#define WIFI_ACCESS_POINT_DURATION 180
#define UPGRADE_ERASE_TIMEOUT 4000

#ifdef MQTT_TLS_ENABLED
#define UART_BUF_SIZE 1024
#else
#define UART_BUF_SIZE (1024+200) //4096 //iHUB lite will be limited to 1K + some bytes for remote terminal
#endif

#define WRAPPER_HEADER_SIZE 8
#define WRAPPER_SIZE (WRAPPER_HEADER_SIZE + 2)
#define TCP_TYPE  0
#define UDP_TYPE  1
#define NTP_TYPE  2
#define MQTT_TYPE 3
#define WEB_TYPE  4
#define WIFI_STATE_TYPE 5
#define INTERNAL_TYPE 6  
#define UPGRADE_TYPE 7
#define MEASUREMENTS_TYPE 8

#define STATE_STARTED        0
#define STATE_FILESYSTEM     1
#define STATE_CONNECTING     2
#define STATE_CONNECTED_SSID 3
#define STATE_CONNECTED_PASS 4
#define STATE_READY          5
#define STATE_RESET          6
#define STATE_SERIAL_NUMBER  7
#define STATE_CONNECT_TOUT   8
#define STATE_RESTARTING     9
#define STATE_GET_WIFI_SETTINGS   10
#define STATE_UPGRADING      11
#define STATE_AP_MODE        12
#define STATE_GET_STM_SETTINGS 13
#define STATE_READY_PRODUCTION 14

#define UDP_INFO_IHUB    0
#define UDP_INFO_LEFT    1
#define UDP_INFO_RIGHT   2
#define UDP_INFO_RS485_1 3
#define UDP_INFO_RS485_2 4
#define WIFI_SETTINGS    10
#define STM_SETTINGS     11

#define UPGRADE_START   0
#define UPGRADE_VERIFY  1
#define UPGRADE_PROGRAM 2
#define UPGRADE_ERASE   3
#define UPGRADE_END     4
#define UPGRADE_DATA    5

#define IHUB_LITE_APP_FILE_TYPE 1013
#define IHUB_LITE_BOOTLOADER_FILE_TYPE 1014

//requests to stm    
#define COUNTERS                 1
#define MEASUREMENTS             2
#define IHUB_SETTINGS            3
#define MEASUREMENTS_MQTT        4
#define IHUB_SETTINGS_MQTT       5
#define COUNTERS_MQTT            6
#define STATUS                   7
#define STATUS_MQTT              8
#define BICOM_STATE              9
#define IR_BICOM_ON             10
#define IR_BICOM_OFF            11
#define IR_BICOM_TOGGLE         12    
#define BICOM_485_1_ON          13
#define BICOM_485_1_OFF         14
#define BICOM_485_1_TOGGLE      15
#define BICOM_485_2_ON          16
#define BICOM_485_2_OFF         17
#define BICOM_485_2_TOGGLE      18
#define MAXIMUM_DEMANDS         19     
#define MAXIMUM_DEMANDS_MQTT    20
#define SET_IHUB_SETTINGS       21
#define SET_IHUB_SETTINGS_MQTT  22
#define RAM_LOGGER              23    
#define RAM_LOGGER_MQTT         24
#define GET_STM_STATISTICS      25    
#define GET_STM_STATISTICS_MQTT 26
#define DEVICE_SETTINGS         27
#define DEVICE_SETTINGS_MQTT    28
#define DEXMA_LOAD_SCHEDULE_MQTT 29
#define SCAN_RS485_BUS          30
#define GET_RS485_DEVICES       31
#define BICOM_STATE_MQTT        32

#define INTERNAL_TIME        1
#define INTERNAL_UPTIME      2
#define RESET_WIFI_SETTINGS  3
#define INTERNAL_WIFI_SIGNAL 4
#define PRODUCTION_MODE      5
#define MQTT_DEBUG_ON        6
#define MQTT_DEBUG_OFF       7

//MQTT Types
#define MQTT_COMMAND           1
#define MQTT_SETTINGS_PHYSICAL 2
#define MQTT_SETTINGS_LOGICAL  3
#define MQTT_SETTINGS_GENERAL  4
#define MQTT_LOAD_SCHEDULE     5
#define MQTT_LOAD_PROFILE      6
#define MQTT_ACK               7
#define MQTT_DBG_TX            8 //publish from ST to MQTT
#define MQTT_DBG_RX            9 //publish form MQTT to ST

enum {
  UPGRADE_JSON_ERR_UPDATE_FAILED ,
  UPGRADE_JSON_ERR_NO_FINGERPRINT,
  UPGRADE_JSON_ERR_WRONG_HOST_URL,
  UPGRADE_JSON_ERR_WRONG_COMMAND
};

typedef struct
{
  unsigned short start_key;
  unsigned short unique_id;
  unsigned char source_type; //modbus, push, pqdiff
  unsigned char packet_type; //app specific
  unsigned short size; //complete size
}wrapper_header_t;

const char SITE_index[] PROGMEM = R"=====(
  <html>            
  <body>                         
    <form action="/mqtt_settings_parse" target="hiddenFrame">                                                    
      <fieldset id="mqtt_settings">                                             
        <legend>MQTT Settings                           
        </legend>                                                                                                                                                                                                
        <br>  MQTT Hostname:                                                
        <br>                                               
        <input type="text" name="mqtt_server" id="mqtt_server" size="40" maxlength="39">                                               
        <br>                                               
        <br>  MQTT Port:                                                
        <br>                                               
        <input type="text" name="mqtt_port" id="mqtt_port" size="5" maxlength="5">                                               
        <br>                                                                  
        <br>  MQTT Topic:                                                
        <br>                                               
        <input type="text" name="mqtt_topic" id="mqtt_topic" maxlength="19">                                               
        <br>                                               
        <br>  MQTT Publish interval (in seconds):                                                
        <br>                                               
        <input type="text" name="mqtt_publish_interval" id="mqtt_publish_interval" size="5" maxlength="5">                                               
        <br>                                                          
      </fieldset>                                       
      <br>                                       
      <input type="submit" value="Save settings" title="Press to send, then wait a few seconds">                                              
    </form>
    <br>
    <a href="/update">SW Upgrade</a>
    <script type="text/javascript">
      function updateData() {
      console.log("updateData()");
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          console.log("updateData(): got Data:", this.responseText);                                       
          var measData = JSON.parse(this.responseText);
          console.log(measData);          
          document.getElementById("mqtt_server").value = measData.mqtt_server;
          document.getElementById("mqtt_port").value = measData.mqtt_port;          
          document.getElementById("mqtt_topic").value = measData.mqtt_topic;
          document.getElementById("mqtt_publish_interval").value = measData.mqtt_publish_interval;                   
        }
      };
      xhttp.open("GET", "/settings", true); //add timestamp to avoid browser caching
      xhttp.send();
    }      
      updateData();    
    </script>                                        
  </body>
</html>
)=====";

const char clear_wifi_settings_page[] PROGMEM = R"=====(
  <html>
   <head>
     <title>Clear settings</title>
   </head>
   <body>
   <p>Reset WiFi settings<br>
   Are you sure?<br>   
   </p>    
    <form action="/clear_wifi_settings" method=POST><input type="submit" value="Clear WiFi settings"></form>
    <form action="/" method="POST"><input type="submit" value="NO, Back to main page"></form>
   </body>
  </html>
)=====";

/*externals*/
#ifdef TCP_ENABLED
extern int tcp_timestamp;
extern int tcp502_timestamp;
#endif
extern statisticsStruct_t statistics;
/*****Function prototypes ********/
void UDP_Reply();
void UdpSendInfo(char *udp_info);
void UdpGetSettings();
unsigned short swap16(unsigned short num);
unsigned long swap32(unsigned long num);
void saveConfig();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void mqttReconnect();
int GetRingBufferToJson(char *outBuffer, int size);
void printLcdState();
void prepareUnixTimeLcd();
void printDisplay();
void push();
int createXmlTag(char *tag, char *unit, int val, char *str_val, char *xml_buffer);
//void InitRingBuffer(void);
void PutRingBuffer(short d);
int IncRingBufferPointer(int a);
uint32_t calculateCRC32(const uint8_t *data, size_t length);
void saveRtcMem();
void publishSettingsRequest();
void publishSettings(char *mqtt_buffer);
void publishMeasurementsRequest();
void publishMeasurements(char *mqtt_buffer);
void publishCountersRequest();
void publishCounters(char *mqtt_buffer);
void publishStatus(char *mqtt_buffer);
int errMqttIncrement();
int errMqttGet();
void TCPServer();
void TCPServer_502();
unsigned short CRC16(unsigned char *puchMsg, int usDataLen);
void parse_serial();
void serial_write(const char *buffer, int len, unsigned char source_type, unsigned char app_id);
int check_wrapper(uint8_t *buf, int len, bool verbose);
int get_data_from_wrapper(uint8_t *wrapped_buf, uint8_t *raw_data, int size);
void listFilesystem();
int upgradeMaster(const char *filename);
void sendHTTPJsonData(char *data);
void setup_web_server();
void printUptime(char *time_str);
bool loadConfig();
void printSettings();
int wait_for_serial_response(int timeout);
void publishDemands(char *mqtt_buffer);
void parseJsonPowerData(char *json);
int get_wrapped_data_size(char *wrapped_buf, int size);
void publishStatusRequest();
void publishDebugWiFiTx(char *mqtt_buffer);
void publishBicomsState(char *mqtt_buffer);
bool publishToMqtt(char *topic, char *mqtt_buffer);
int getDataFromWifiTempSensors();
void mdns_discover();
void mqttUpgrade(char *json);
bool createJsonResponse(char *result, int buflen, const char *deviceId, const char *ts, const char *requestId, const char *version, const char *status, const char *code, const char *description);
#ifdef MQTT_TLS_ENABLED
int loadcerts();
void verifytls();
void mqtt_settings_parse();
#endif
void clear_wifi_settings();
void wifi_restart();
void sendSettingsToSTM(char* settingsData);

#endif //mc_temp_h
