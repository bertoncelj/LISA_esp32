#include "ihub.h"

#ifdef TCP_ENABLED

#include <WiFiClient.h>
//#define TCP_DEBUGING
#define TCP_BUFFER_SIZE 256
extern WiFiServer TcpMiQen;
extern WiFiServer ModbusTcp;

WiFiClient TCPclient;
uint8_t TCPpacketBuffer[TCP_BUFFER_SIZE];
int tcp_timestamp = 0;

WiFiClient TCP502client;
uint8_t TCP502packetBuffer[TCP_BUFFER_SIZE];
int tcp502_timestamp = 0;

void TCPServer ()
{
  if (!TCPclient.connected())
  {
    // try to connect to a new client
    TCPclient = TcpMiQen.available();
  }
  else
  {
    int len = 0;
    //TCPclient.setTimeout(10);
    TCPclient.setNoDelay(1);
    while (TCPclient.available() && len < TCP_BUFFER_SIZE)
      TCPpacketBuffer[len++] = TCPclient.read();

    if (len > 0)
    {
      tcp_timestamp = millis();
      statistics.tcp_rx_packets++;
      
#ifdef TCP_DEBUGING
      DEBUG_UART.printf(F("TCP start size:%d time:%d\n\r"), len, tcp_timestamp);
      if (len < 20)
      {
        debug_print(F("len:")); debug_println(len);
        for (int i = 0; i < len; i++)
        {
          debug_hex(TCPpacketBuffer[i]);
          debug_print(" ");
        }
        debug_println();
      }
#endif

      //send data to STM and wait for response
      delay(1); //make a pause to end previous packet
      serial_write((char *)TCPpacketBuffer, len, TCP_TYPE, 2);
      wait_for_serial_response(1000);  
    }
  }
}

void TCPServer_502 ()
{
  if (!TCP502client.connected())
  {
    // try to connect to a new client
    TCP502client = ModbusTcp.available();
  }
  else
  {
    int len = 0;
    //TCPclient.setTimeout(10);
    TCP502client.setNoDelay(1);
    while (TCP502client.available() && len < TCP_BUFFER_SIZE)
      TCP502packetBuffer[len++] = TCP502client.read();

    if (len > 0)
    {
      tcp502_timestamp = millis();
      statistics.tcp_rx_packets++;
      
#ifdef TCP_DEBUGING
      DEBUG_UART.printf(F("TCP start size:%d time:%d\n\r"), len, tcp502_timestamp);
      if (len < 20)
      {
        debug_print(F("len:")); debug_println(len);
        for (int i = 0; i < len; i++)
        {
          debug_hex(TCP502packetBuffer[i]);
          debug_print(" ");
        }
        debug_println();
      }
#endif

      //send data to STM and wait for response
      delay(1); //make a pause to end previous packet
      serial_write((char *)TCP502packetBuffer, len, TCP_TYPE, 2);
      wait_for_serial_response(1000);  
    }
  }
}
#endif //TCP_ENABLED

