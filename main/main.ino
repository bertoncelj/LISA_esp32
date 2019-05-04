#include <SoftwareSerial.h>
#include "param.h"

//WIFI
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


bool doneReadAll = false;
const int led = 13;

stMachine ST;
//TEMP_SAVE_STC tmpSaveStc;
//tmpSTC *tmpStc; 
MSG *tmpStc;

//=======================================================================

void setup() 
{
    // Open serial communications and wait for port to open:
    Serial.begin(19200, SERIAL_8N2);  //morta bit 2 stop bita
    DEBUG_UART.begin(19200, SERIAL_8N2);
    while (!Serial);
    debug_println("V1.5");
    debug_println("BREAKSIGN CONNECT");
    updateST(CONNECT, EMPTY, EMPTY); 
}

void loop()
{
    //STATE MACHINE
    debug_print("curState:"); 
    debug_println(ST.next); 
    switch(ST.next) {
        case CONNECT:
            debug_println("We in CONNECT");
            connect_breakSign();
        break;

        case BREAK:
            debug_println("We in BREAK");
            breakLISA();
        break;

        case SIGN:
            debug_println("We in SIGN");
            signLISA();
        break;

        case WAIT_RX:
            waitRX();
        break;

        case FULL_CHECK_RX:
            checkIfCorrectData();
        break;

        case SAVE_RX:
            saveRX();
        break;

        case NULLPENT:
           send(&m_nullPent);
           ST.future = WEB_REQ;
        break;

        case WEB_REQ:
            debug_println("WIN!");
        break;
/*
        case GET_GRAPH:
            sendGraphRequest();
        break;

        case WAIT_RX_ARR:
            waitRXArr();
        break;

        case READ:
            debug_println("We are is ST.state READ");
            sendRead();
        break;

*/
        default:
            debug_println("default ST.state"); 
        }
}

void send(MSG *newLIST)
{
    debug_println("send message");
    debug_println(newLIST->check_message_len);
    debug_array(newLIST->send_message, newLIST->send_message_len);
    Serial.write(newLIST->send_message, newLIST->send_message_len);

    //tmpSaveStc.saveSTC = &newLIST;
    tmpStc = newLIST;
    ST.next = WAIT_RX;
}

void waitRX() 
{
    debug_println("IN RX"); 
    delay(200);
    //two types of recive with endMark or fixLenght
    debug_print("get_t_endMark_f_len: "); 
    debug_println(tmpStc->get_t_endMark_f_len); 
    if(tmpStc->get_t_endMark_f_len == true) {
        //endMark
        recvWithendMarker();
    }else {
        //fixLen
        recWithFixLenght();
    }

    //////////
    static int count_missRX = 0;
    delay(200); //wait a bit for buffer to fill
    debug_println("choose if statment");
    if(tmpStc->get_allDataRecv == true && tmpStc->check_checkRecvMessage == true) {
        ST.next = FULL_CHECK_RX;
    } else if(tmpStc->get_allDataRecv == true && tmpStc->check_checkRecvMessage == false ) {
        //for just read RX, don't check RX arr
        ST.next = SAVE_RX;
    } else if(tmpStc->get_allDataRecv == false && tmpStc->check_checkRecvMessage == false ) {
        //TODO: Miss Error Handler
        count_missRX ++;
        debug_print("miss: ");
        debug_println(count_missRX);
    } else {
        debug_println("ERROR STATE!!!");
        debug_println("Connecet again");
        error_state();
    }
}

void connect_breakSign() 
{
    //breakLISA();
    //send sign
    //debug_array(send_sign, sizeof(send_sign));
    //Serial.write(send_sign, sizeof(send_sign)); 

    //flush serial bufffer
    while (Serial.available() > 0) 
        Serial.read();
    updateST(BREAK, SIGN, CONNECT);
}

void breakLISA() 
{
    delay(100);
    debug_println("BREAK_LISA");
    debug_array(send_break, sizeof(send_break));
    Serial.write(send_break, sizeof(send_break));
    debug_println("");
    delay(900); // MUST be 900 ms !!!!
    if (ST.from == CONNECT) {
        nextST();
    } else {
        //add new state to go from break
        error_state();
    }
}

void signLISA()
{
    send(&m_LisaKeyRX);
    ST.future = NULLPENT;
}

void updateST(enumSTAT nextState, enumSTAT futureState, enumSTAT fromState) 
{
    ST.next = nextState;
    ST.future = futureState;
    ST.from = fromState;
}

void nextST() 
{
    if (ST.future != EMPTY) {
        ST.next = ST.future;
        ST.future = EMPTY;
    } else {
        error_state();
    }
}

void error_state() 
{
    while(1)
        debug_println("ERORR STATE");
}

void connect_second_nullPetEna() 
{
    //SEND message 051
    debug_println("Send 051"); 
    debug_array(send_nullpetena, sizeof(send_nullpetena));
    Serial.write(send_nullpetena, sizeof(send_nullpetena));
    
    delay(3000);
    //wait for return message lisa

    //next from curr checkARR
    updateST(READ, CONF_CONNECT,  WEB_REQ);
}

void recWithFixLenght() { 
    int set_fix_len = 8;
    int count_RX_data = 0;
    byte rc;

    while(set_fix_len < 8) {
        while (Serial.available() > 0) {
            rc = Serial.read();

            debug_hex(rc);
            debug_print("_");
            debug_print(count_RX_data);
            debug_print(" ");
        }
    }
}
void recvWithendMarker() 
{
    static byte ndx = 0;
    int rtn_len = 0;
    byte rc;
    
    debug_print("if allData: ");
    debug_println(tmpStc->get_allDataRecv);
    debug_println("Start to read:");
    while (Serial.available() > 0 && tmpStc->get_allDataRecv == false) {
        rc = Serial.read();
        debug_hex(rc);
        debug_print(" ");

        debug_println("here");
        if (rc !=  tmpStc->get_endMark && ndx < tmpStc->check_message_len) {
            
            debug_println("here if");
            tmpStc->get_message[ndx] = rc;
            debug_print("ndx: ");
            debug_println(ndx);
            ndx++;
            //leak detect over 128
            if (ndx >= MAX_REC_ARR_LEN) {
                ndx = MAX_REC_ARR_LEN - 1;
                debug_println("array leak!!!");
            }
        }
        else {
            tmpStc->get_message[ndx] = rc;  // terminate the string  
            tmpStc->get_message_len = ndx + 1;
            ndx = 0;
            tmpStc->get_allDataRecv = true;
            debug_println("END");
        }
    }
    debug_println("");
    debug_println("here end");
}

bool checkIfCorrectData() 
{
    debug_println("In check");
    //check lenght of arrays
    byte *t;
    byte *r;
    int len_t = tmpStc->check_message_len; //fix lenght
    int len_r = tmpStc->get_message_len;  //recive len
    int idx;

    t = tmpStc->check_message;
    r = tmpStc->get_message;
    debug_println(len_t);
    debug_println(len_r);

    if (len_t == len_r) {
        debug_println("Arrays are equal len");
        for (idx = 0; idx < len_t; idx ++){
            debug_hex(t[idx]);
            debug_print("=");
            debug_hex(r[idx]);
            debug_print(", ");
            if(t[idx] != r[idx]) return false;
        }
        //correct OK
        debug_println("Array CORRECT");
        ST.next = ST.future;
        tmpStc->get_allDataRecv = false;
        return true;
    }
    else if (len_t == 0 ) {
        debug_println("Test array Is empty");
        return false;
    }
    else {
        debug_println("ERROR: arrays are diff len");
        //server.send(200, "text/plain", "RESET");
        delay(5000);
        //next from curr checkARR
        //flush serial bufffer
        while (Serial.available() > 0) 
            Serial.read();
        error_state();
        return false;
    }
}

boolean saveRX() {
    byte *r;
    int len_r = tmpStc->get_message_len;
    int idx;
    bool inSaveModeBetweenParam = false;

    static int idx_saveValue = 0;
    static int arrsaveValue[10]; //care 10 max

    r = tmpStc->get_message;

    //debug_println("RX SAVED: ");
    for (idx = 0; idx < len_r; idx ++){
        if (r[idx] == 0x29) inSaveModeBetweenParam = false;
        if (inSaveModeBetweenParam == true) {
            //debug_print("SaveInSaveVAl ");
            //debug_print(r[idx]);
            //debug_println("");
            r[idx] -= 0x30;
            if(r[idx] > 10) r[idx] -= 0x07;
            arrsaveValue[idx_saveValue] += r[idx];
            idx_saveValue ++;
        }
        if (r[idx] == 0x28) inSaveModeBetweenParam = true;  
        
        //debug_print(r[idx]);
        //debug_print(", ");
    }
    //debug_println();
    int convertedToInt = hexToInt(&arrsaveValue[0], idx_saveValue);

    //restet global arr to all vals to 0
    for (idx = 0; idx < idx_saveValue; idx ++){
        arrsaveValue[idx] = 0;
    }
    idx_saveValue = 0; //reset to 0 cuz is static

    *export_int = convertedToInt;
    //ST.state = ST.next;
    //ST.newData = false;
    return true;
}

int hexToInt(int *arrSaveValue, int arr_len) {
    int rtnInt = 0; 
    int i;
    int multiplayer[4] = {4096, 256,16, 1};
    int mult = 0;

    //debug_println("HexToInt");
    for (i = 0; i < arr_len; i++){
        debug_println(arrSaveValue[i]);
    }
    debug_println("");
    for(i = 0; i < arr_len; i++){
        rtnInt = arrSaveValue[i] * multiplayer[i] + rtnInt;
    }
    //debug_println("RTN INT");
    //debug_println(rtnInt);
    return rtnInt;
}

/*
//for graph
void waitRXArr ()
{
    debug_println("waitRXArr()");
    int saveArrIndex = 0;
    byte rc;
    while (saveArrIndex < 536){
        debug_println(saveArrIndex);
        debug_println(" ");
        while (Serial.available() > 0) {
            rc = Serial.read();
            //debug_hex(rc);
           // debug_print(" ");
            get_arr_graph[saveArrIndex] = rc;
            saveArrIndex ++;
        }
    }

    debug_println(" ");
    debug_println(saveArrIndex);
    delay(2000);
    //print all 
    int i;
    char val[4];
    debug_println("VREDNOSTI!!!!");
    for ( i = 0; i < 536; i++) {
        debug_println(get_arr_graph[i]);
        debug_print(" ");
    }

    debug_println("We got : ");
    debug_println(saveArrIndex);
    debug_println(" elements");
    saveArrIndex = 0;
    ST.state = WEB_REQ;
}

void checkRX()
{
    debug_println("waitRX()");
    checkIfCorrectData();
}


void serialFlash() 
{
    while(Serial.available() > 0) {
        char t = Serial.read();
    }
}


void showNewData() 
{
    if (ST.newData == true) {
        debug_println("we got data:");
        debug_println("DATA ARRIVED");
        ST.newData = false;
    }
}

void printAllLisa() {
    debug_println("//////////////////////");
    debug_println(lisa_index);
    debug_println(lisa_temp);
    debug_println(lisa_Vbat);
    debug_println(lisa_U1);
    debug_println(lisa_U2);
    debug_println(lisa_Upov);
    debug_println(lisa_ANG);
    debug_println(lisa_ANG1);
    debug_println(lisa_ANG2);
    debug_println(lisa_U3);
    debug_println(lisa_U4);
    debug_println(lisa_ANG_tot1);
    debug_println(lisa_ANG3);
    debug_println(lisa_ANG4);
    debug_println("//////////////////////");
}

void sendRead()
{
    static int nextRead = 0;
    int len_arr = 14;

    //flush serial bufffer
    if (nextRead == 0) {
        while (Serial.available() > 0) 
            Serial.read();
    }
    debug_println("..................");
    debug_print("READ NEXT position: ");
    debug_println(nextRead);
    STC_LIST arrREADS[3] = {ReadTemp};
    ST.recArr.stcArr = r_arr_names[nextRead];
    ST.recArr.lenArr = 8;
    ST.recArr.endMarker = 0xFF;
    ST.recArr.check = false;

    export_int = lisa_names[nextRead];
    nextRead ++;
    if (nextRead == len_arr) {
        doneReadAll = true;
        nextRead = 0;
        //printAllLisa();

        //next   from   curr checkARR
        handleRoot();
        fillST(WEB_REQ, READ, WEB_REQ, arrREADS[0]);
        return;
    }

    debug_println("Send next data:");
    debug_array(ST.recArr.stcArr, 16);
    Serial.write(ST.recArr.stcArr, 16);

    //next   from   curr checkARR
    fillST(READ, READ, WAIT_RX, arrREADS[0]);
}



void sendGraphRequest()
{
    //send sign
    //debug_array(r_arr_graph, sizeof(r_arr_graph));
    Serial.write(r_arr_graph, sizeof(r_arr_graph)); 
    
    fillST(WEB_REQ, CONF_CONNECT,  WAIT_RX_ARR, arrPZeroRX);
}

/////////////////////////WIFI////////////////////////////////

void handleData() {
    Serial.println("Sending root page");
    digitalWrite(led, 1);
    char temp[400];


    snprintf(temp, 400,
           " %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
            
            lisa_index,
            lisa_U1, lisa_Upov, lisa_U2, lisa_ANG1, lisa_ANG_tot1, lisa_ANG2,
            lisa_U3, lisa_U4, lisa_ANG3, lisa_ANG4,
            lisa_Vbat, lisa_temp
    );
    server.send(200, "text/html", temp);
    digitalWrite(led, 0);
}

void handleMain() {
    //next from curr checkARR
    debug_println("if!");
    if ( doneReadAll == false) {
        debug_println("return!");
        fillST(READ, WEB_REQ, READ, arrLisaKeyRX);
    } else {
        fillST(WEB_REQ, WEB_REQ, WEB_REQ, arrLisaKeyRX);
        digitalWrite(led, 1);
        char temp[3000];

        debug_println("print website !");
        snprintf(temp, 2500, 
        "<html>\
        <head>\
        <meta http-equiv='refresh' content='8'/>\
        <style>\
        table, th, td {\
        \
        border-collapse: collapse;\
        }\
        th, td {\
        padding: 15px;\
        text-align: center;\
        }\
        table#t01 {\
        width: 100%;    \
        background-color: #f1f1c1;\
        }\
        </style>\
        </head>\
        <body>\
        \
        <h1 align=\"center\" style=\"color:red;\">ALISA DATA</h1>\
        <h3>Measurements: %d </h3>\
        <h3 align=\"center\" style=\"color:blue;\">-----------  Measurements ----------</h3>\
        <table style=\"width:100%\">\
        <tr>\
            <th style=\"color:blue;\">Left</th>\
            <th></th> \
            <th style=\"color:blue;\">Right</th>\
        </tr>\
        <tr>\
            <td>U1: <font size=\"6\"><b>%d</b></font></td>\
            <td>Upop: <font size=\"6\"><b>%d</b></font></td>\
            <td>U2: <font size=\"6\"><b>%d</b></font></td>\
        </tr>\
        <tr>\
            <td>ANG1: <font size=\"6\"><b>%d</b></font></td>\
            <td>ANG_tot: <font size=\"6\"><b>%d</b></font></td>\
            <td>ANG2: <font size=\"6\"><b>%d</b></font></td>\
        </tr>\
        <tr>\
        </tr>\
        </table>\
        <br>\
        <h3 align=\"center\" style=\"color:blue;\">-----------  Measurements amplified ----------</h3>\
        <table id=\"t01\">\
        <tr>\
            <th style=\"color:blue;\">Left</th>\
            <th></th> \
            <th style=\"color:blue;\">Right</th>\
        </tr>\
        <tr>\
            <td>U3: <font size=\"6\"><b>%d</b></font></td>\
            <td></td>\
            <td>U4: <font size=\"6\"><b>%d</b></font></td>\
        </tr>\
        <tr>\
            <td>ANG3: <font size=\"6\"><b>%d</b></font></td>\
            <td></td>\
            <td>ANG4: <font size=\"6\"><b>%d</b></font></td>\
        </tr>\
        <tr>\
        \
        </tr>\
        </table>\
        <h3></h3>\
        <h3 style=\"color:blue;\">----------- General Inforamtion ----------</h3>\
        <h3>Battery Voltage: %d mV</h3>\
        <h3>Temperature: %d C</h3>\
        </body>\
        </html>\
            ",
            lisa_index,
            lisa_U1, lisa_Upov, lisa_U2, lisa_ANG1, lisa_ANG_tot1, lisa_ANG2,
            lisa_U3, lisa_U4, lisa_ANG3, lisa_ANG4,
            lisa_Vbat, lisa_temp
        );

        server.send(200, "text/html", temp);
        digitalWrite(led, 0);
        doneReadAll = false;
    }
}

void handleRoot() {
    fillST(WEB_REQ, WEB_REQ, WEB_REQ, arrLisaKeyRX);
    digitalWrite(led, 1);
    char temp[3000];

    debug_println("print website !");
    snprintf(temp, 2500, 
    "<html>\
    <head>\
    <meta http-equiv='refresh' content='8'/>\
    <style>\
    table, th, td {\
    \
    border-collapse: collapse;\
    }\
    th, td {\
    padding: 15px;\
    text-align: center;\
    }\
    table#t01 {\
    width: 100%;    \
    background-color: #f1f1c1;\
    }\
    </style>\
    </head>\
    <body>\
    \
    <h1 align=\"center\" style=\"color:red;\">ALISA DATA</h1>\
    <h3>Measurements: %d </h3>\
    <h3 align=\"center\" style=\"color:blue;\">-----------  Measurements ----------</h3>\
    <table style=\"width:100%\">\
    <tr>\
        <th style=\"color:blue;\">Left</th>\
        <th></th> \
        <th style=\"color:blue;\">Right</th>\
    </tr>\
    <tr>\
        <td>U1: <font size=\"6\"><b>%d</b></font></td>\
        <td>Upop: <font size=\"6\"><b>%d</b></font></td>\
        <td>U2: <font size=\"6\"><b>%d</b></font></td>\
    </tr>\
    <tr>\
        <td>ANG1: <font size=\"6\"><b>%d</b></font></td>\
        <td>ANG_tot: <font size=\"6\"><b>%d</b></font></td>\
        <td>ANG2: <font size=\"6\"><b>%d</b></font></td>\
    </tr>\
    <tr>\
    </tr>\
    </table>\
    <br>\
    <h3 align=\"center\" style=\"color:blue;\">-----------  Measurements amplified ----------</h3>\
    <table id=\"t01\">\
    <tr>\
        <th style=\"color:blue;\">Left</th>\
        <th></th> \
        <th style=\"color:blue;\">Right</th>\
    </tr>\
    <tr>\
        <td>U3: <font size=\"6\"><b>%d</b></font></td>\
        <td></td>\
        <td>U4: <font size=\"6\"><b>%d</b></font></td>\
    </tr>\
    <tr>\
        <td>ANG3: <font size=\"6\"><b>%d</b></font></td>\
        <td></td>\
        <td>ANG4: <font size=\"6\"><b>%d</b></font></td>\
    </tr>\
    <tr>\
    \
    </tr>\
    </table>\
    <h3></h3>\
    <h3 style=\"color:blue;\">----------- General Inforamtion ----------</h3>\
    <h3>Battery Voltage: %d mV</h3>\
    <h3>Temperature: %d C</h3>\
    </body>\
    </html>\
        ",
        lisa_index,
        lisa_U1, lisa_Upov, lisa_U2, lisa_ANG1, lisa_ANG_tot1, lisa_ANG2,
        lisa_U3, lisa_U4, lisa_ANG3, lisa_ANG4,
        lisa_Vbat, lisa_temp
    );

    server.send(200, "text/html", temp);
    digitalWrite(led, 0);
    doneReadAll = false;
}

void handleManualReset(){
        server.send(200, "text/plain", "manual reset ");
        delay(5000);
        ESP.restart();  
}

void handleGraph() {

  int i;
  String message;

  for(i = 24; i < 536; i++) {
    message += String(get_arr_graph[i]);
    message += " ";
  }
  server.send(200, "text/html", message);
  fillST(GET_GRAPH, WEB_REQ, GET_GRAPH, arrLisaKeyRX);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
*/
