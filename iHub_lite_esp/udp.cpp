#include "ihub.h"

#ifdef UDP_ENABLED

extern StoreStruct_t settings;
extern char UDPpacketBuffer[255];
extern char device_type[16];
extern int uptime;
extern int errPush;
extern unsigned short i_mqttPort;
extern bool udp_ihub_received;
extern bool udp_left_ir_received;
extern bool udp_rs485_1_received;
extern bool udp_rs485_2_received;

extern WiFiUDP UdpMiQen;
char UDPpacketBuffer[255];
char udp_info_ihub[200];
char udp_info_left_ir[200];
char udp_info_rs485[2][200];

void UDP_Reply()
{
  char time_str[40];

  time_t UTC_time = time(NULL);
  struct tm * timeinfo = localtime(&UTC_time);
  sprintf(time_str, "%d-%02d-%02dT%02d:%02d:%02dZ", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  int len = UdpMiQen.read(UDPpacketBuffer, 255);
  //if (len > 0) UDPpacketBuffer[len-1] = 0;
  debug_print(time_str);
  debug_print(F(": Received len: "));
  debug_println(len);

  //STM_UART.print("UDP Received len: ");
  //STM_UART.println(len);
  //STM_UART.write(UDPpacketBuffer, len);
  //serial_write(UDPpacketBuffer, len, UDP_TYPE, 3);
  
  //debug_println(UDPpacketBuffer);
  if (len < 20)
  {
    for (int i = 0; i < len; i++)
      debug_hex(UDPpacketBuffer[i]);

    debug_println();
  }

  if (UDPpacketBuffer[3] == 0x1D || UDPpacketBuffer[3] == '4') //reset command
  {
    char serial_msg[20];
    debug_println(F("UDP Reset Command"));
    debug_println(F("RESTARTING !!!!!!!"));

    sprintf(serial_msg, "RESTARTING");
    serial_write(serial_msg, strlen(serial_msg), WIFI_STATE_TYPE, STATE_RESTARTING); //STM_UART.println("RESTARTING");
    wifi_restart();
  }

  if (UDPpacketBuffer[3] == 0x1A  && len == 4) //query info command
  {
    DEBUG_UART.printf("UDP Query response. Ihub:%d Ir:%d RS485_1:%d RS485_2:%d\n\r", udp_ihub_received, udp_left_ir_received, udp_rs485_1_received, udp_rs485_2_received);
    
    if(udp_ihub_received)
      UdpSendInfo(udp_info_ihub);

    delay(100);

    if(udp_left_ir_received)
      UdpSendInfo(udp_info_left_ir);

    delay(100);

    if(udp_rs485_1_received)
      UdpSendInfo(udp_info_rs485[0]);

    delay(100);

    if(udp_rs485_2_received)
      UdpSendInfo(udp_info_rs485[1]);
  }

  if (UDPpacketBuffer[3] == 0x1c && len == 154) //send settings command
  {
    UdpGetSettings();
  }
}

void UdpSendInfo(char *udp_info)
{    
  UdpMiQen.beginPacket(UdpMiQen.remoteIP(), UdpMiQen.remotePort());
  UDPpacketBuffer[0] = 0;
  UDPpacketBuffer[1] = 0;
  UDPpacketBuffer[2] = 0;
  UDPpacketBuffer[3] = 0x1b; //Device info  
  
#if 0 //wifi browser way

  char info[250];
  memset(info, 0, 250);
  
  IPAddress wifiip = WiFi.localIP();
  memcpy(&info[0], &wifiip[0], 4);
  //SSID
  memcpy(&info[0] + 4, WiFi.SSID().c_str(), 12);
  unsigned char mac[6];
  WiFi.macAddress(mac);
  memcpy(&info[0] + 16, mac, 6);
  unsigned short local_port = swap16(10001);
  memcpy(&info[0] + 22, &local_port, 2);
  memcpy(&info[0] + 24, &device_type, 15); //bug in info table, only 15 chars reserved
  memcpy(&info[0] + 39, &settings.serial, 8);
  unsigned short software_version = swap16(SW_VERSION);
  memcpy(&info[0] + 47, &software_version, 2);
  unsigned short hardware_version = swap16(HW_VERSION);
  memcpy(&info[0] + 49, &hardware_version, 2);
  //char *description = "Temperaturni senzor                     ";
  memcpy(&info[0] + 51, settings.description, 40);
  //char *location = "Pri Marjanu                                ";
  memcpy(&info[0] + 91, settings.location, 40);
  unsigned short modbus_address = swap16(33);
  memcpy(&info[0] + 131, &modbus_address, 2);
  //push server at 133
  IPAddress IPtmp;
  if (IPtmp.fromString(settings.pushServer))
    memcpy(&info[0] + 133, &IPtmp[0], 4);
  /*
    debug_print("IP push: ");
    debug_println(pushServer);
    debug_print(info[133], HEX);
    debug_print(info[134], HEX);
    debug_print(info[135], HEX);
    debug_print(info[136], HEX);
    debug_println();
  */
  //push port at 137
  unsigned short udp_pushPort = swap16(atoi(settings.pushPort));
  memcpy(&info[0] + 137, &udp_pushPort, 2);
  //push interval at 139
  unsigned long udp_push_interval = swap32(mqtt_publish_interval);//unsigned long udp_push_interval = swap32(atoi(settings.S_push_interval));
  memcpy(&info[0] + 139, &udp_push_interval, 4);
  //ntp server at 143
  if (IPtmp.fromString(settings.NtpServer))
    memcpy(&info[0] + 143, &IPtmp[0], 4);

#if 0 //no measurements on iHub lite
  //measurement 1 at 147
  signed short udp_temperature = swap16((signed short)(temperature * 100));
  memcpy(&info[0] + 147, &udp_temperature, 2);
  //measurement 2 at 149
  signed short udp_measurement2;
  if (settings.sensor_type[0] == BME280_TYPE || settings.sensor_type[0] == DHT11_TYPE || settings.sensor_type[0] == SHT30_TYPE) 
    udp_measurement2 = swap16((signed short)humidity);
  else
    udp_measurement2 = 0;
  memcpy(&info[0] + 149, &udp_measurement2, 2);
  //measurement 3 at 151
  signed short udp_measurement3;
  if (settings.sensor_type[0] == BME280_TYPE)
    udp_measurement3 = swap16((signed short)pressure);
  else
    udp_measurement3 = 0;
  memcpy(&info[0] + 151, &udp_measurement3, 2);
  //battery level at 163
  unsigned short battlevel_udp = swap16(battlevel * 100); //battery level
  memcpy(&info[0] + 163, &battlevel_udp, 2);
#endif  
  //Uptime at 165
  unsigned long udp_uptime = swap32(uptime);
  memcpy(&info[0] + 165, &udp_uptime, 4);
  //errors at 169
  unsigned short udp_errPush = swap16(errPush);
  memcpy(&info[0] + 169, &udp_errPush, 2);
  //zurb 19.5: added mqtt
  if (IPtmp.fromString(settings.mqtt_server))
    memcpy(&info[0] + 173, &IPtmp[0], 4);
  //mqtt port at 177
  unsigned short udp_mqttPort = swap16((i_mqttPort));
  memcpy(&info[0] + 177, &udp_mqttPort, 2);        

  memcpy(UDPpacketBuffer + 4, info, 250);
  
  UdpMiQen.write(UDPpacketBuffer, 254);
  //UdpMiQen.write("\r\n");
  UdpMiQen.endPacket();

#else //MiQen type

  //fill our mac address
  //unsigned char mac[6];
  //WiFi.macAddress(mac);
  //memcpy(&udp_info[0] + 16, mac, 6);
  
  memcpy(UDPpacketBuffer + 4, udp_info, 200);  

  UdpMiQen.write(UDPpacketBuffer, 204);
  //UdpMiQen.write("\r\n");
  UdpMiQen.endPacket();
#endif
  delay(200);
}

void UdpGetSettings()
{
  debug_print(F("Description: "));
  strcpy(settings.description, &UDPpacketBuffer[4 + 0]);
  debug_println(settings.description);

  debug_print(F("Location: "));
  strcpy(settings.location, &UDPpacketBuffer[4 + 40]);
  debug_println(settings.location);

  debug_print(F("Push IP: "));
  //unsigned char udp_push_ip[4];
  //memcpy(&udp_push_ip[0], &UDPpacketBuffer[4 + 80], 4);
  //IPAddress IP_push(udp_push_ip);
  //debug_print(udp_push_ip[0], HEX);
  //debug_print(udp_push_ip[1], HEX);
  //debug_print(udp_push_ip[2], HEX);
  //debug_print(udp_push_ip[3], HEX);
  //debug_println(IP_push);
  //sprintf(settings.pushServer, "%d.%d.%d.%d" , UDPpacketBuffer[4 + 80] , UDPpacketBuffer[4 + 81], UDPpacketBuffer[4 + 82], UDPpacketBuffer[4 + 83]);
  //debug_println(settings.pushServer);

//  debug_print("Push Port: ");
//  unsigned short udp_push_port;
//  memcpy(&udp_push_port, &UDPpacketBuffer[4 + 84], 2);  
//  sprintf(settings.pushPort, "%d" , swap16(udp_push_port));
//  debug_println(settings.pushPort);

  unsigned long udp_push_interval;
  memcpy(&udp_push_interval, &UDPpacketBuffer[4 + 86], 4);
  //udp_push_interval = swap32(udp_push_interval);
  settings.mqtt_publish_interval = swap32(udp_push_interval); //sprintf(settings.mqtt_publish_interval, "%ld" , swap32(udp_push_interval));
  debug_print(F("Push Interval: "));
  debug_println(settings.mqtt_publish_interval);

  debug_print(F("NTP server: "));
  //unsigned char udp_ntp_ip[4];
  //memcpy(&udp_ntp_ip[0], &UDPpacketBuffer[4 + 90], 4);
  //IPAddress IP_ntp(udp_ntp_ip);
  sprintf(settings.NtpServer1, "%d.%d.%d.%d" , UDPpacketBuffer[4 + 90] , UDPpacketBuffer[4 + 91], UDPpacketBuffer[4 + 92], UDPpacketBuffer[4 + 93]);
  debug_println(settings.NtpServer1);

  debug_print(F("MQTT server: "));
  sprintf(settings.mqtt_server, "%d.%d.%d.%d" , UDPpacketBuffer[4 + 96] , UDPpacketBuffer[4 + 97], UDPpacketBuffer[4 + 98], UDPpacketBuffer[4 + 99]);
  debug_println(settings.mqtt_server);

  debug_print(F("MQTT Port: "));
  unsigned short udp_mqtt_port;
  memcpy(&udp_mqtt_port, &UDPpacketBuffer[4 + 100], 2);
  sprintf(settings.strMqttPort, "%d" , swap16(udp_mqtt_port));
  debug_println(settings.strMqttPort);  

  char udp_response[14];
  UdpMiQen.beginPacket(UdpMiQen.remoteIP(), UdpMiQen.remotePort());
  udp_response[0] = 0;
  udp_response[1] = 0;
  udp_response[2] = 0;
  udp_response[3] = 0x1e;

  memcpy(&udp_response[4], settings.serial, 8);

  short result = swap16(0);
  memcpy(&udp_response[12], &result, 2);

  UdpMiQen.write(udp_response, 14);
  //UdpMiQen.write("\r\n");
  UdpMiQen.endPacket();

  debug_println(F("Saving UDP config & RESTART"));

  saveConfig();

  wifi_restart();
}
#endif //UDP_ENABLED

