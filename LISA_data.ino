#include <SoftwareSerial.h>

//SoftwareSerial Serial1(13,15);
const byte numChars = 128;

//Send messages
byte send_break[] = {0x01, 0x42, 0x30, 0x03, 0x71};
byte send_sign[] = {0x2F, 0x3F, 0x21, 0x0D, 0x0A};
byte send_nullpetena[] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};
byte send_read[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x30, 0x28, 0x29};


char receivedChars[numChars]; // an array to store the received data

boolean newData = false;
boolean nextStep = false;

byte incomingByte = 0;
char firstRecFromLisa[numChars]  = "1";
char jedan = '1';

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(19200, SERIAL_8N2);  //morta bit 2 stop bita
    Serial1.begin(19200, SERIAL_8N2);
    while (!Serial);

}
void connectToLisa() {
    int tryGetLisa = 0;
   // while(true) { //more is equal true
        tryGetLisa ++;
        //send break
        delay(100);
        Serial1.write(send_break, sizeof(send_break));
        delay(100);

        //send sign
        Serial1.write(send_sign, sizeof(send_sign)); 
        
        //wait for return message lisa

        recvWithendMarker();
        //checkifcorrectdata();
        showNewData();
    
    //}
}

void recvWithendMarker() {
    static byte ndx = 0;
    char endMarker = '\n'; //can be /r
    char rc;

    Serial.println("Start to read:");
    Serial.println(newData);
    while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();
        Serial.print(rc, HEX);
        Serial.print(" ");

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
                Serial.println("array leak!!!");
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
            Serial.println("END");
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial.print("we got data:");
        Serial.println(receivedChars);
        newData = false;
    }
}

void loop(){
    Serial.println("NewLoop");
    connectToLisa();
    delay(1000);
    delay(1000);
    Serial1.println("OUT");
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


