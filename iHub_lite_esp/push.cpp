#include "ihub.h"

#ifdef PUSH_ENABLED
extern char device_type[];
extern int log_id;
extern StoreStruct_t settings;
extern float battlevel;
extern int push_interval;
extern int errPush;
extern int32_t WifiSignalLevel;

char xml[1000];
WiFiClient push_client;

void createPushXml(unsigned short sampling_time, unsigned short recorder_no, char *UTC_Time)
{
  char xml_tmp[300];
  char msg[50];

  device_type[15] = '\0';

  sprintf(xml_tmp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\r");
  strcpy(xml, xml_tmp);

  sprintf(xml_tmp, "<data logId=\"%d\" controlUnit=\"%s\" deviceType=\"%s\" part=\"%d\">\n\r",
          log_id, settings.serial, DEVICE_TYPE, recorder_no + 1);

  strcat(xml, xml_tmp);

  if (sampling_time > 900)
    sprintf(xml_tmp, "<measurement dateTime=\"%s\" tInterval=\"PT%dM\">\n\r", UTC_Time, sampling_time / 60); //minutes
  else
    sprintf(xml_tmp, "<measurement dateTime=\"%s\" tInterval=\"PT%03dS\">\n\r", UTC_Time, sampling_time);  //seconds

  strcat(xml, xml_tmp);

  dtostrf(temperature , 2, 1, msg);
  createXmlTag("T", "C", 0, msg, xml);

  dtostrf(WifiSignalLevel , 2, 0, msg);
  createXmlTag("RSSI", "dB", 0, msg, xml);

  if (settings.sensor_type[0] == DHT11_TYPE || settings.sensor_type[0] == BME280_TYPE || settings.sensor_type[0] == SHT30_TYPE) //#if defined(DHT11) || defined(BME280_SENSOR)
  {
    dtostrf(humidity , 2, 0, msg);
    createXmlTag("Hum", "%", 0, msg, xml);
  }

  if (settings.sensor_type[0] == BME280_TYPE) //#ifdef BME280_SENSOR
  {
    dtostrf(pressure , 2, 0, msg);
    createXmlTag("atm", "hPa", 0, msg, xml);
  }

  sprintf(xml_tmp, "</measurement>\n\r");
  strcat(xml, xml_tmp);

  sprintf(xml_tmp, "</data>\n\r");
  strcat(xml, xml_tmp);
}

int createXmlTag(char *tag, char *unit, int val, char *str_val, char *xml_buffer)
{
  char outTag[100];
  //char * value_modifier[] = {"_min", "_max", "", "#"};

  char start_tag[] = {"<value ident=\""};
  char unit_tag[] = {"\" unit="};
  char end_tag[] = {"</value>"};
  char *p = outTag;
  outTag[0] = '\0';

  strcat(p, start_tag);
  strcat(p, tag); //name of tag
  //strcat(p, value_modifier[val - 12]); //min, max ...
  strcat(p, "#"); //todo min, max...
  strcat(p, unit_tag);
  strcat(p, "\""); //"
  strcat(p, unit); // V, A, Hz, ...
  strcat(p, "\">"); //"
  strcat(p, str_val); //measured value
  strcat(p, end_tag);
  strcat(p, "\n\r");

  //log_msg(LOG_USER | LOG_INFO, "INFO(advpush) %s: %s",__FUNCTION__, outTag);

  strcat(xml_buffer, outTag);

  return strlen(outTag);
}

void push()
{
  String line;
  char time_str[40];
  int i_pushPort = atoi(settings.pushPort);

  if (i_pushPort == 0)
  {
    debug_println(F("PUSH port = 0. Push disabled"));
    return;
  }

  time_t UTC_time = time(NULL);
  struct tm * timeinfo = localtime(&UTC_time);
  sprintf(time_str, "%d-%02d-%02dT%02d:%02d:%02dZ", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  createPushXml(push_interval, 1, time_str);
  debug_println(F("PUSH:"));
  debug_println(xml);
  debug_println("");

  int retry = 2;
  bool push_connected = false;

  while (retry-- && push_connected == false)
  {
    if (!push_client.connect(settings.pushServer, i_pushPort)) {
      debug_println(F("Push connection retry"));
      delay(500);
    }
    else
      push_connected = true;
  }

  if (push_connected == false)
  {
    debug_println(F("Push connection failed"));
    errPush++;
    return;
  }

  push_client.print(xml); //send push

  //wait for ACK
  unsigned long timeout = millis();
  while (push_client.available() == 0) {
    if (millis() - timeout > 5000) {
      debug_println(F(">>> Push Client Timeout !"));
      push_client.stop();
      errPush++;
      return;
    }
  }

  // Read all the lines of the ACK from server and print them to Serial
  while (push_client.available()) {
    line = push_client.readStringUntil('\r');
    debug_print(line);
  }
  debug_println("");
  push_client.stop();

  //check ACK
  int ackLen;
  char ack[20] = "<ack logId=\"";
  sprintf(ack, "<ack logId=\"%d\"", log_id);
  ackLen = strlen(ack);
  if (!strncmp(line.c_str(), ack, ackLen))
    debug_println(F("ACK OK"));
  else
  {
    debug_println(F("ACK Failed received:"));
    debug_println(line);
    debug_println(F("Sent:"));
    debug_println(ack);
    debug_print(F("Len:"));
    debug_println(ackLen);
    errPush++;
  }
}
#endif //PUSH
