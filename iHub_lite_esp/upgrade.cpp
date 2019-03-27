#include "ihub.h"

int upgradeStartCommand(const char *filename);
int upgradeVerify(const char *filename);
int upgradeEraseFlash();
int upgradeProgram(const char *filename);
int upgradeSendFile(const char *filename);

extern String upgradeFile;
extern rtcMem_t rtcMem;

char upgrade_ack[10];
int upgrade_timestamp = 0;
int upgrade_file_lines = 0;

void listFilesystem()
{
  String str = "";
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    str += dir.fileName();
    if (dir.fileName().startsWith("/upgrade"))
    {
      upgradeFile = dir.fileName();
      rtcMem.upgrade_counter++; //counter to prevent endless upgrade loop
      saveRtcMem(); //update rtc memory
    }
    str += " / ";
    str += dir.fileSize();
    str += "\r\n";
  }
  debug_println(str);
}

int upgradeMaster(const char *filename)
{
  int ret;
  int retry = 3;

  if (!SPIFFS.exists(filename) )
  {
    debug_print(F("Upgrade file doesn't exist: "));
    debug_println(filename);
    rtcMem.upgrade_counter = 0;//counter to prevent endless upgrade loop
    saveRtcMem(); //update rtc memory
    return -1;
  }

  if (rtcMem.upgrade_counter > 3) //try 3 times then erase file
  {
    debug_print(F("Upgrade counter > 3!!!!!!!!!!"));
    SPIFFS.remove(filename);
    rtcMem.upgrade_counter = 0;//counter to prevent endless upgrade loop
    saveRtcMem(); //update rtc memory    
    return -1;
  }

  while (retry--)
  {
    delay(1000);
    //upgrade start puts stm to bootloader mode from stm version 29!!!!
    debug_println(F("----------------------UPGRADE START----------------------"));
    ret = upgradeStartCommand(filename); //send size, type, version
    if (ret)
    {
      debug_println(F("ERROR UPGRADE START FAILED"));
      continue;
    }
    
    delay(1000);
    //2nd upgrade start is real start. this is for the compatibility reasons
    debug_println(F("----------------------UPGRADE START----------------------"));
    ret = upgradeStartCommand(filename); //send size, type, version
    if (ret)
    {
      debug_println(F("ERROR UPGRADE START FAILED"));
      continue;
    }

    delay(100);
    debug_println(F("----------------------UPGRADE VERIFY----------------------"));
    ret = upgradeVerify(filename);
    if (ret)
    {
      debug_println(F("ERROR VERIFY FAILED"));

      delay(100);
      debug_println(F("----------------------UPGRADE ERASE----------------------"));
      ret = upgradeEraseFlash();
      if (ret)
      {
        debug_println(F("ERROR UPGRADE ERASE FAILED"));
        continue;
      }

      delay(100);
      debug_println(F("----------------------UPGRADE PROGRAM----------------------"));
      ret = upgradeProgram(filename);
      if (ret)
      {
        debug_println(F("ERROR UPGRADE PROGRAM FAILED"));
        continue;
      }
#if 0 //dont verify after write, because write has verify   
      debug_println(F("UPGRADE VERIFY"));
      ret = upgradeVerify(filename);
      if (ret)
      {
        debug_println(F("ERROR VERIFY FAILED"));
      }
#endif
    }
    break;
  }//while

  delay(100);
  SPIFFS.remove(filename);
  rtcMem.upgrade_counter = 0;//counter to prevent endless upgrade loop
  saveRtcMem(); //update rtc memory 
  
  debug_println(F("----------------------Upgrade finished ----------------------"));
  serial_write("END", 3, UPGRADE_TYPE, UPGRADE_END); //send upgrade end command  
  delay(100);

  return 0;
}

int waitForUpgradeAck(const char *ack)
{
  upgrade_timestamp = millis();
  while (upgrade_timestamp && (millis() - upgrade_timestamp < UPGRADE_ERASE_TIMEOUT))//2000ms because erase is taking more than a second
  {
    if (STM_UART.available() )
    {
      parse_serial();
    }
    delay(1);
  }
  if (upgrade_timestamp)
  {
    debug_println(F("ERROR: upgrade timeout"));
    STM_UART.flush();
    return 1;
  }
  //check acknowledge
  if (strcmp(upgrade_ack, ack))
  {
    DEBUG_UART.printf("ERROR ACK: %s != %s\n\r", upgrade_ack, ack);
    STM_UART.flush();
    return 2;
  }
  return 0;
}

int upgradeStartCommand(const char *filename)
{
  size_t len;
  char buf[100];
  // open file for reading
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    debug_println(F("file open failed"));
    return -1;
  }

  //get fl2 file header at last 6 bytes
  int ret = f.seek(6, SeekEnd); //offset was negative in 2.3.0 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  
  DEBUG_UART.printf("File position result :%d, position: %d\n\r", ret, f.position());
  len = f.readBytes(buf, 6); //read last 6 bytes
  debug_print(F("Header: "));
  for (size_t i = 0; i < len; i++)
  {
    debug_hex(buf[i] ^ 86);
    debug_print(" ");
  }
  debug_println();

  upgrade_file_lines = (buf[0] ^ 86) + (buf[1] ^ 86) * 256;
  int upgrade_type = (buf[2] ^ 86) + (buf[3] ^ 86) * 256;
  int version = (buf[4] ^ 86) + (buf[5] ^ 86) * 256;
  DEBUG_UART.printf("File info:\n\rType   :%d\n\rVersion:%d\n\rLines  :%d\n\r", upgrade_type, version, upgrade_file_lines);

  if(upgrade_type != IHUB_LITE_APP_FILE_TYPE && upgrade_type != IHUB_LITE_BOOTLOADER_FILE_TYPE)
  {
    debug_print(F("UPGRADE Type ERROR:"));    
    debug_println(upgrade_type);
    f.close();
    return -1;
  }
  //SEND START UPGRADE
  serial_write(buf, 6, UPGRADE_TYPE, UPGRADE_START); //send upgrade start command
  if (waitForUpgradeAck("UP ACK"))
  {
    debug_println(F("UPGRADE START FAILED"));
    f.close();
    return -1;
  }
  f.close();
  return 0;
}

int upgradeVerify(const char *filename)
{
  int ret;
  char buf[100];
  //SEND VERIFY COMMAND
  serial_write(buf, 6, UPGRADE_TYPE, UPGRADE_VERIFY); //send upgrade verify command
  if (waitForUpgradeAck("UP ACK"))
  {
    debug_println(F("UPGRADE VERIFY COMMAND FAILED"));
    //f.close();
    return -1;
  }

  ret = upgradeSendFile(filename);
  return ret;
}

int upgradeSendFile(const char *filename)
{
  size_t len;
  int lines = 0;
  char buf[100];
  int verify_failed = 0;

  // open file for reading
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    debug_println(F("file open failed"));
    return -1;
  }

  debug_println(F("====== Reading from SPIFFS file ======="));

  //f.seek(0, SeekSet); //back to start
  //while (f.available()) {
  while (f.position() < f.size() - 6) {
    size_t line_size = 2;
    //Lets read line by line from the file
    len = f.readBytes(buf, line_size); //W and line size
    if ((buf[0] ^ 86) != 'W' || (buf[1] ^ 86) > 100 || len != line_size)
    {
      DEBUG_UART.printf("ERROR line start:0x%02x size:0x%02x len:%d\n\r", buf[0] ^ 86, buf[1] ^ 86, len);
      f.close();
      return -2;
    }

    line_size = (buf[1] ^ 86) * 2 + 5; //size is in words + 5 bytes (address ans csum)
    len = f.readBytes(buf + 2, line_size); //W and line size

    //#define DEBUG_UPGRADE
#ifdef DEBUG_UPGRADE
    for (int i = 0; i < len + 2; i++)
    {
      debug_hex(buf[i] ^ 86);
      debug_print(" ");
    }
    debug_println();
#endif

    serial_write(buf, len + 2, UPGRADE_TYPE, UPGRADE_DATA);
    lines++;
    debug_print(".");
    if (waitForUpgradeAck("UP ACK"))
    {
      debug_println(F("UPGRADE DATA FAILED"));
      verify_failed = 1;
      break;
    }
  }//while file

  f.close();

  if (upgrade_file_lines != lines)
  {
    verify_failed = 1;
    debug_print(F("Sent lines: "));
    debug_println(lines);
  }
  return verify_failed;
}

int upgradeEraseFlash()
{
  char buf[100];
  //SEND ERASE COMMAND
  serial_write(buf, 6, UPGRADE_TYPE, UPGRADE_ERASE); //send upgrade verify command
  if (waitForUpgradeAck("UP ACK"))
  {
    debug_println(F("UPGRADE ERASE COMMAND FAILED"));
    //f.close();
    return -1;
  }
  return 0;
}

int upgradeProgram(const char *filename)
{
  int ret;
  char buf[100];
  //SEND PROGRAM COMMAND
  serial_write(buf, 6, UPGRADE_TYPE, UPGRADE_PROGRAM); //send upgrade verify command
  if (waitForUpgradeAck("UP ACK"))
  {
    debug_println(F("UPGRADE PROGRAM COMMAND FAILED"));
    //f.close();
    return -1;
  }

  ret = upgradeSendFile(filename);
  return ret;
}


