#include <SoftwareSerial.h>

#define DEBUG_UART Serial1

//debug is serial 1
#define DEBUG_IHL
#ifdef DEBUG_IHL
#define debug_print(msg) Serial1.print(msg)
#define debug_hex(msg) Serial1.print(msg, HEX)
#define debug_println(msg) Serial1.println(msg)
#define debug_hexln(msg) Serial1.println(msg, HEX)
#define debug_array(msg, size) Serial1.write(msg, size)
#else
#define debug_print(msg)
#define debug_hex(msg)
#define debug_println(msg)
#define debug_hexln(msg)
#define debug_array(msg, size)
#endif

//SoftwareSerial Serial1(13,15);
const byte numChars = 128;

//Send messages
byte send_break[] = {0x01, 0x42, 0x30, 0x03, 0x71};
byte send_sign[] = {0x2F, 0x3F, 0x21, 0x0D, 0x0A};
byte send_nullpetena[] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};
byte rec_LISA_key[] = {0x2F, 0x4C, 0x31, 0x35, 0x41, 0x5F, 0x49, 0x44, 0x0D, 0x0A};
byte send_read[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x30, 0x28, 0x29};

byte debug_arr[] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};

char receivedChars[numChars]; // an array to store the received data

boolean newData = false;
boolean nextStep = false;

byte incomingByte = 0;
char firstRecFromLisa[numChars];

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(19200, SERIAL_8N2);  //morta bit 2 stop bita
    DEBUG_UART.begin(19200, SERIAL_8N2);
    while (!Serial);
}

void connectToLisa() {
    int rtn_len;
    int tryGetLisa = 0;
   // while(true) { //more is equal true
        tryGetLisa ++;
        //send break
        delay(100);
        Serial1.write(send_break, sizeof(send_break));
        Serial.write(send_break, sizeof(send_break));
        delay(900); // MUST be 900 ms !!!!
        debug_println();
        //send sign
        
        Serial1.write(send_sign, sizeof(send_sign));
        Serial.write(send_sign, sizeof(send_sign)); 

        //wait for return message lisa

        rtn_len = recvWithendMarker();
        checkIfCorrectData(receivedChars,rtn_len,  rec_LISA_key, sizeof(rec_LISA_key)/sizeof(byte));
        showNewData();
    //}
}

bool checkIfCorrectData(char *t, int len_t, byte *r, int len_r) {
    //check lenght of arrays
    int idx;
    debug_println(len_t);
    debug_println(len_r);
    if (len_t == len_r) {
        debug_println("Arrays are equal len");
        for (idx = 0; idx < len_t; idx ++){
            if(t[idx] != r[idx]) return false;
        }
        debug_println("Array CORRECT");
        return true;
    }
    else if (len_t == 0 ) {
        debug_println("Test array Is empty");
        return false;
    }
    else {
        debug_println("ERROR: arrays are diff len");
        return false;
    }
}

int recvWithendMarker() {
    static byte ndx = 0;
    int rtn_len;
    byte endMarker = 0x0A; //can be /r
    char rc;

    debug_println("Start to read:");
    debug_println(newData);
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        debug_hex(rc);
        debug_print(" ");

        if (rc != (char) endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
                debug_println("array leak!!!");
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            rtn_len = ndx;
            ndx = 0;
            newData = true;
            debug_println("END");
        }
    }
    return rtn_len;
}

void showNewData() {
    if (newData == true) {
        debug_println("we got data:");
        debug_println(receivedChars);
        newData = false;
    }
}

void loop(){
    connectToLisa();
    delay(1000);
    delay(1000);
}
/*
void loop() {
  nextStep = false;
  Serial.write(send_break, sizeof(send_break));
  Serial.write(send_sign, sizeof(send_sign));
  delay(3000);
 
 recvWithEndMarker();
 showNewData();
 delay(1000);
 if(nextStep == true){
  Serial.write(send_nullpetena, sizeof(send_nullpetena));
  delay(1000);
  recvWithEndMarker();
  showNextData();
  Serial.println("konc");
 }
}

void weInRead(){
  while(1){
    newData = true;
    delay(1000);
    Serial.write(send_read, sizeof(send_read));
    delay(1000);
 recvWithEndMarker();
 showNewData();
 delay(1000);     
        
  }
}


void showNextData() {
  Serial.print("We got it ...");
 Serial.println(receivedChars);
 weInRead();
 if(receivedChars[4] == jedan) {
 Serial.println("we got lisa in"); 
 nextStep = true;
 }}}
 */


