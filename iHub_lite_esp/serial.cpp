#include "ihub.h"

//#define DEBUG_SERIAL

extern StoreStruct_t settings;
extern SettingsStruct_t stmSettings;
extern WiFiClient TCPclient;
extern WiFiClient TCP502client;
#ifdef UDP_ENABLED
extern char udp_info_ihub[200];
extern char udp_info_left_ir[200];
extern char udp_info_rs485[2][200];
#endif
extern int upgrade_timestamp;
extern char upgrade_ack[10];
extern int STM_bootloader_mode;
//extern char json_buf[JSON_SIZE];
extern bool reset_after_receive_settings;
extern bool stm_settings_valid;
extern bool publish_settings;
extern unsigned char auchCRCHi[];
extern unsigned char auchCRCLo[];
extern bool shouldSaveConfig;
extern ESP8266WebServer HTTP;

int wrapper_uid;
int serial_semaphore = 0;

#ifdef UDP_ENABLED
bool udp_ihub_received = false;
bool udp_left_ir_received = false;
bool udp_rs485_1_received = false;
bool udp_rs485_2_received = false;
#endif

uint8_t uartMsg[UART_BUF_SIZE];
char *raw_msg;

void parse_serial()
{
  //size_t len = UART_BUF_SIZE;//for succesfull compilation must be this way
  //uint8_t uartMsg[len];
  //uint8_t raw_msg[len];
  bool wrapper_checked = false;

  size_t size = 0;
  while (STM_UART.available() && size < UART_BUF_SIZE)
  {
    size_t bytes = STM_UART.available();
    STM_UART.readBytes((char*)&uartMsg[size], bytes);
    size += bytes;
    //check if msg is complete
#if 0
    if (size > 10 && check_wrapper(uartMsg, size, false))
    {
      wrapper_checked = true;
      break;
    }
#endif
    delay(2); //wait for complete serial msg, 1ms is not enough
  }

  serial_semaphore = 0; //release

  if (size == 0)
    return;

  //debug_print("F(STM Uart size: "));
  //debug_println(size);

  statistics.serial_rx_packets++;

  if (wrapper_checked == false)
  {
    if (check_wrapper(uartMsg, size, true) == 0)
    {
      debug_println(F("Wrapper ERROR"));
      statistics.errors++;
      return;
    }
  }

  //memset(raw_msg, 0, UART_BUF_SIZE);
  //int raw_size = get_data_from_wrapper(uartMsg, raw_msg, size);
  int raw_size = get_wrapped_data_size((char*)uartMsg, size);
  raw_msg = (char*)uartMsg + WRAPPER_HEADER_SIZE;
  if (raw_size < 0)
  {
    debug_println(F("Wrapper ERROR: size wrong"));
    statistics.errors++;
    return;
  }

  raw_msg[raw_size] = '\0'; //terminate rawmsg

#ifdef DEBUG_SERIAL
  DEBUG_UART.printf(F("Serial wrapper type:%d size:%d\n\r"), uartMsg[4], raw_size);
  if (raw_size < 250)
  {
    for (int i = 0; i < raw_size; i++)
    {
      debug_hex(raw_msg[i]);
      debug_print(" ");
    }
    debug_println();
  }
#endif

  if (uartMsg[4] == WIFI_STATE_TYPE)
  {
    debug_print(F("WIFI_STATE_TYPE ack size:"));
    debug_println(raw_size);
  }
#ifdef TCP_ENABLED  
  else if (uartMsg[4] == TCP_TYPE)
  {
    statistics.tcp_tx_packets++;
    //send response to TCP client
    if (tcp_timestamp)
    {
      //DEBUG_UART.printf("TCP answer size:%d time:%d\n\r", raw_size, millis());
      if (raw_size > 0)
      {
        int ret = TCPclient.write((uint8_t*)raw_msg, raw_size);
        //DEBUG_UART.printf("TCP written:%d\n\r", ret);
      }
      tcp_timestamp = 0;
    }
    if (tcp502_timestamp)
    {
      //DEBUG_UART.printf("TCP answer size:%d time:%d\n\r", raw_size, millis());
      if (raw_size > 0)
      {
        int ret = TCP502client.write((uint8_t*)raw_msg, raw_size);
        //DEBUG_UART.printf("TCP written:%d\n\r", ret);
      }
      tcp502_timestamp = 0;
    }
  }
#endif //TCP_ENABLED  
  
  else if (uartMsg[4] == UDP_TYPE)
  {
    DEBUG_UART.printf("UDP size:%d type:%d\n\r", raw_size, uartMsg[5]);
    if (uartMsg[5] == WIFI_SETTINGS)
    {
      StoreStruct_t *tmp_settings;
      tmp_settings = (StoreStruct_t*)raw_msg;
      unsigned long crc = calculateCRC32((uint8_t*)raw_msg, raw_size - 4);

      DEBUG_UART.printf("Settings received, size: %d crc:%08lx received crc:%08lx\n\r", raw_size, crc, tmp_settings->crc);

      if ((sizeof(settings) != raw_size) || (crc != tmp_settings->crc))
      {
        DEBUG_UART.printf("ERROR Wifi settings size:%d msg size:%d calc crc:%08lx received crc:%08lx\n\r", sizeof(settings), raw_size, crc, tmp_settings->crc);
        statistics.errors++;
        publish_settings = false;
      }
      else //settings packet ok
      {
        if (tmp_settings->crc != settings.crc)
        {
          DEBUG_UART.printf("Saving Settings, size: %d new crc:%08lx crc:%08lx\n\r", raw_size, tmp_settings->crc, settings.crc);
          memcpy((void*)&settings, raw_msg, raw_size);
          saveConfig();
          delay(100);
          loadConfig();
          debug_println(F("------------------------------------"));
          printSettings();//it also terminates strings
          debug_println(F("------------------------------------"));
          debug_println(F("Applying settings...RESTARTING"));
          serial_write("0", 0, UDP_TYPE, WIFI_SETTINGS); //send ack
          if (reset_after_receive_settings)
          {
            wifi_restart();
          }
        }
        else
        {
          debug_println(F("Settings are the same for ESP"));
          publish_settings = true; //publish them anyway, because STM settings are probably changed
        }
      }
      serial_write("0", 0, UDP_TYPE, WIFI_SETTINGS); //send ack
    }
    else if (uartMsg[5] == STM_SETTINGS)
    {
      debug_print(F("STM Settings received, size:"));
      debug_println(raw_size);

      if (raw_size != sizeof(stmSettings))
      {
        DEBUG_UART.printf("ERROR STM Settings size:%d not:%d\n\r", raw_size, sizeof(stmSettings));
        stm_settings_valid = false;
        publish_settings = false; //dont publish settings
        statistics.errors++;
      }
      else
      {
        memcpy(&stmSettings, raw_msg, raw_size);   //set local copy of STM settings which we need for modifying and sending them back
        stm_settings_valid = true;
        publish_settings = true;
      }
    }    
    else if (uartMsg[5] == UDP_INFO_IHUB)
    {
#ifdef UDP_ENABLED      
      if (raw_size != 200)
      {
        debug_print(F("ERROR UDP msg size:"));
        debug_println(raw_size);
        statistics.errors++;
      }
      else
      {
        udp_ihub_received = true;        
        memcpy(udp_info_ihub, raw_msg, raw_size);        
      }
#endif      
      serial_write("", 0, UDP_TYPE, UDP_INFO_IHUB); //send ack even if udp is disabled
    }
    else if (uartMsg[5] == UDP_INFO_LEFT)
    {
#ifdef UDP_ENABLED      
      if (raw_size != 200)
      {
        debug_print(F("ERROR UDP msg size:"));
        debug_println(raw_size);
        statistics.errors++;
      }
      else
      {
        udp_left_ir_received = true;        
        memcpy(udp_info_left_ir, raw_msg, raw_size);
      }
#endif      
      serial_write("", 0, UDP_TYPE, UDP_INFO_LEFT); //send ack even if udp is disabled
    }
    else if (uartMsg[5] == UDP_INFO_RS485_1)
    {
#ifdef UDP_ENABLED      
      if (raw_size != 200)
      {
        debug_print(F("ERROR UDP msg size:"));
        debug_println(raw_size);
        statistics.errors++;
      }
      else
      {
        udp_rs485_1_received = true;
        
        memcpy(udp_info_rs485[0], raw_msg, raw_size);
      }
#endif      
      serial_write("", 0, UDP_TYPE, UDP_INFO_RS485_1); //send ack even if udp is disabled
    }
    else if (uartMsg[5] == UDP_INFO_RS485_2)
    {
#ifdef UDP_ENABLED      
      if (raw_size != 200)
      {
        debug_print(F("ERROR UDP msg size:"));
        debug_println(raw_size);
        statistics.errors++;
      }
      else
      {
        udp_rs485_2_received = true;
        memcpy(udp_info_rs485[1], raw_msg, raw_size);
      }
#endif      
      serial_write("", 0, UDP_TYPE, UDP_INFO_RS485_2); //send ack even if udp is disabled
    }
  }//UDP_TYPE  
  else if (uartMsg[4] == INTERNAL_TYPE)
  {
    raw_msg[raw_size] = '\0';
    if (uartMsg[5] == BOOTLOADER_MODE)
      STM_bootloader_mode = 1;
    //else
    //STM_bootloader_mode = 0;
    else if (uartMsg[5] == MQTT_DBG_TX) //publish debug info
    {
      debug_print(F("Debug TX received:"));
      debug_println(raw_msg);
      publishDebugWiFiTx((char*)raw_msg);      
    }

    //DEBUG_UART.printf("Internal size:%d :%s STM bootloader:%d\n\r", raw_size, raw_msg, STM_bootloader_mode);
  }
  else if (uartMsg[4] == UPGRADE_TYPE)
  {
    raw_msg[raw_size] = '\0';
    memcpy(upgrade_ack, raw_msg, raw_size + 1);
    //DEBUG_UART.printf("%s\n\r", upgrade_ack);
    upgrade_timestamp = 0;
  }
  else if (uartMsg[4] == MEASUREMENTS_TYPE)
  {
    raw_msg[raw_size] = '\0';
    //DEBUG_UART.printf("MEASUREMENTS:%s\n\r", raw_msg);

    if (uartMsg[5] == IHUB_SETTINGS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == IHUB_SETTINGS_MQTT)
    {      
      publishSettings((char*)raw_msg);
    }
#ifdef MQTT_TLS_ENABLED 
    //in TLS version, mqtt parameters can be modified on captive portal, so this settings must be sent to stm
    //   shouldSaveConfig is flag which is set when settings are modified on captive portal
    //when we receive ack from stm, flag is cleared
    else if (uartMsg[5] == SET_IHUB_SETTINGS) //ack on send stm settings
    {      
      shouldSaveConfig = false;
    }    
#endif    
    if (uartMsg[5] == DEVICE_SETTINGS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == MEASUREMENTS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == MEASUREMENTS_MQTT)
    {      
      publishMeasurements((char*)raw_msg);
    }
    else if (uartMsg[5] == MAXIMUM_DEMANDS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    if (uartMsg[5] == MAXIMUM_DEMANDS_MQTT)
    {
      publishDemands((char*)raw_msg);      
    }
    else if (uartMsg[5] == COUNTERS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == COUNTERS_MQTT)
    {
      publishCounters((char*)raw_msg);      
    }
    else if (uartMsg[5] == STATUS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == STATUS_MQTT)
    {
      publishStatus((char*)raw_msg);      
    }
#ifdef WEB_SERVER_ENABLED    
    else if (uartMsg[5] == RAM_LOGGER || uartMsg[5] == RAM_LOGGER_MQTT)
    {
      //sendHTTPJsonData((char*)raw_msg);   //send to http
#ifndef MQTT_TLS_ENABLED      
      parseJsonPowerData((char*)raw_msg);
#endif //#ifndef MQTT_TLS_ENABLED      
    }
#endif //WEB_SERVER_ENABLED        
    else if (uartMsg[5] == BICOM_STATE)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == BICOM_STATE_MQTT)
    {
      publishBicomsState((char*)raw_msg);      
    }
    else if (uartMsg[5] == GET_STM_STATISTICS)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
    else if (uartMsg[5] == GET_RS485_DEVICES)
    {
      sendHTTPJsonData((char*)raw_msg);   //send to http
    }
  }//MEASUREMENTS_TYPE
  else
    DEBUG_UART.printf("Unknown wrapper type:%d size:%d\n\r", uartMsg[4], raw_size);
}

int wait_for_serial_response(int timeout)
{
  while (STM_UART.available() == 0 && timeout--)
    delay(1);

  if (STM_UART.available() == 0)
  {
    debug_println(F("Serial response timeout"));
    return -1;
  }
  else
    parse_serial();

  return 0;
}

//for sending big JSON payload without copy payload
void serial_write(const char *buffer, int len, unsigned char source_type, unsigned char app_id)
{
  char wrapper_buf[WRAPPER_HEADER_SIZE];
  int i;
  unsigned char uchCRCHi = 0xFF ; /* high CRC byte initialized */
  unsigned char uchCRCLo = 0xFF ; /* low CRC byte initialized  */
  unsigned int uIndex ;               /* will index into CRC lookup*/
  wrapper_header_t *wrapper;

  if (len > UART_BUF_SIZE)
  {
    debug_println(F("ERROR:serial_write_without_copy size > UART_BUF_SIZE"));
    statistics.errors++;
    return;
  }

  if (serial_semaphore)
  {
    debug_print(F("Serial port semaphore: ")); //just print warning
    debug_println(source_type);
  }

  //prepare wrapper header
  wrapper = (wrapper_header_t *) wrapper_buf;
  wrapper->start_key = 0xA9A9;
  wrapper->unique_id = wrapper_uid++;
  wrapper->source_type = source_type;
  wrapper->packet_type = app_id;
  wrapper->size = len;

  //calculate CRC of wrapper
  for (i = 2; i < WRAPPER_HEADER_SIZE; i++) //calculate crc from 2nd byte
  {
    uIndex = uchCRCHi ^ wrapper_buf[i];       /* calculate the CRC  */
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
    uchCRCLo = auchCRCLo[uIndex] ;
  }

  //send wrapper header
  STM_UART.write((uint8_t *)wrapper_buf, WRAPPER_HEADER_SIZE);

  for (i = 0; i < len; i++) //calculate crc of payload
  {
    uIndex = uchCRCHi ^ buffer[i];       /* calculate the CRC  */
    uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;
    uchCRCLo = auchCRCLo[uIndex] ;
  }

  //send data payload
  STM_UART.write((uint8_t *)buffer, len);

  //send crc of wrapper
  char crcBuf[2];
  crcBuf[0] = uchCRCHi;
  crcBuf[1] = uchCRCLo;
  STM_UART.write((uint8_t *)crcBuf, 2);
  STM_UART.flush();

  serial_semaphore = millis(); //set semaphore
  statistics.serial_tx_packets++;
}

int check_wrapper(uint8_t *buf, int len, bool verbose)
{
  int datalen;
  unsigned short crc, received_crc;

  if (len > 1 && buf[0] == 0xA9 && buf[1] == 0xA9)
  {
    datalen = (buf[7] << 8) | (buf[6] & 0xff);

    if (datalen > len - 10)
    {
      if (verbose)
      {
        DEBUG_UART.printf("ERROR check wrapper.Datalen:%d len:%d\n\r", datalen, len);
        statistics.errors++;
      }
      return 0;
    }
    crc = CRC16((unsigned char*)(buf + 2), datalen + 6); //crc is calculated on data + 6 bytes of header
    received_crc = (buf[datalen + 8] << 8) | (buf[datalen + 9] & 0xff);
    if (crc != received_crc)
    {
      if (verbose)
      {
        DEBUG_UART.printf("ERROR wrapper.CRC:%x rCRC:%x\n\r", crc, received_crc);
        statistics.errors++;
      }
      return 0;
    }
    return 1;
  }
  if (verbose)
  {
    statistics.errors++;
    debug_print(F("ERROR check wrapper. Len: "));
    debug_print(len);
    if (len > 1)
    {
      debug_print(F(" Start:"));
      debug_hex(buf[0]); debug_print(" "); debug_hex(buf[1]); debug_println();
    }
  }
  return 0;
}

int get_wrapped_data_size(char *wrapped_buf, int size)
{
  int data_size = wrapped_buf[7] * 256 + wrapped_buf[6];

  if ( size > UART_BUF_SIZE || size < WRAPPER_SIZE || data_size > (size - WRAPPER_SIZE))
  {
    DEBUG_UART.printf("ERROR Wrong size:%d Wrapper:%d\n\r", size, data_size);
    //statistics.errors++;
    return -1;
  }
  if (wrapped_buf[0] != 0xA9 && wrapped_buf[1] != 0xA9)
  {
    DEBUG_UART.printf("ERROR Wrapper:%x %x\n\r", wrapped_buf[0], wrapped_buf[1]);
    //statistics.errors++;
    return -2;
  }
  if (data_size != (size - WRAPPER_SIZE))
    debug_print(F("WARNING: WRAPPER contains more than 1 msg\n\r"));

  return (data_size);
}

void sendHTTPJsonData(char *data)
{
  HTTP.send ( 200, "application/json", data );
}


