#include <SoftwareSerial.h>
#include "param.h"

//WIFI
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/*
const char* ssid = "AndroidAP";
const char* password = "urur2377";
#define ROUTE_ADDRESS "http://92.37.97.225:56721/"
*/

//const char* ssid = "TP-LINK";
//const char* password = "poljchSpodnjiGeslo";

const char* ssid = "TP-LINK_A23BA4";
const char* password = "tamalasobca";
//#define ROUTE_ADDRESS "http://poljch.home.kg:41856/"
#define ROUTE_ADDRESS "http://92.37.26.33:41856/"

bool doneReadAll = false;
const int led = 13;

String stringEmptySpace;
stMachine ST;

MSG *tmpStc;

//=======================================================================

//WIFI setup
HTTPClient http;

void setup() 
{
    // Open serial communications and wait for port to open:
    Serial.begin(19200, SERIAL_8N2);  //morta bit 2 stop bita
    DEBUG_UART.begin(19200, SERIAL_8N2);
    while (!Serial);
    debug_println("V1.6003");
    debug_println("BREAKSIGN CONNECT");
    updateST(CONNECT, EMPTY, EMPTY); 
    
    //WIFI setup
    WiFi.begin(ssid, password);   
}

void loop()
{
    //STATE MACHINE
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

        case SAVE_IN_ARR:
            saveInArr();
        break;

        case NULLPENT:
           send(&m_nullPent);
           ST.future = WEB_REQ;
        break;

        case WEB_REQ:
            static boolean readAllReg = false;
            static int saveMeasVal = 0;
            static int indx = 0; 
            static boolean sendReqForIndx = false;

            if (readAllReg == false ) {
                tmpStc->save_inArr = false;
                //debug_print("indx: ");
                //debug_print(saveMeasVal);
                //debug_print(" <-> ");
                //debug_println(tmpStc->save_message);
                sendReg(r_arr_index);
                sendReqForIndx = true;
            }
            
            if(tmpStc->save_message == saveMeasVal) delay(500);
            if (tmpStc->save_message != saveMeasVal && readAllReg == false)  {
                readAllReg = true;
                saveMeasVal = tmpStc->save_message;
                return;

            }

            if (readAllReg == true) {
                //debug_println("Read Register");
                tmpStc->save_inArr = true;
                sendReg(r_arr_names[indx]);
                if ( indx != 0) {
                    //if (indx == 1) debug_println("-----------");
                    //debug_println("/////////////");
                    //debug_print(indx - 1);
                    //debug_print(": ");
                    //debug_println(tmpStc->save_message);
                    //debug_println("/////////////");
                }
                indx ++;
                

                if (indx == NUM_ARR_INT) {
                    indx = 0;
                    readAllReg = false;
                }
            }
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

void sendReg(byte *newArr) {
    
    m_lisaReg.send_message = newArr;
    //m_lisaReg.check_message = 0x30;
    

    tmpStc = &m_lisaReg;
    //debug_println("Send request");
    //debug_array(tmpStc->send_message, tmpStc->send_message_len);
    Serial.write(tmpStc->send_message, tmpStc->send_message_len);
    ST.next = WAIT_RX;
}

void saveInArr()
{
    static int indxSave = 0;
    static int numArrInt = 0; 
    static boolean weGotGraph = true;

    //debug_print("SaveInArr: ");
    //debug_print(numArrInt);
    //debug_print(" ");
    //debug_println(indxSave);

    saveAllArr[numArrInt][indxSave] = tmpStc->save_message;
    indxSave ++;
    if (indxSave == NUM_ARR_INT) {
        indxSave = 0;
        numArrInt ++;
        debug_println(numArrInt);
        weGotGraph = true;
    }

    //start to connect wifi 2 mesures before 10 second time window
    if(numArrInt == REG_MAX_LEN - 3) {
        WiFi.forceSleepWake();
        WiFi.mode(WIFI_STA);
        //wifi_station_connect();
        WiFi.begin(ssid, password);   
    }
    
    if (weGotGraph == true) {
        if (numArrInt  == 18 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }

        if (numArrInt  == 36 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 54 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 72 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 90 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 108 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 126 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 144 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 162 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
        if (numArrInt  == 180 ) {
            debug_println("going in waitARR");
            waitRXArr();
            weGotGraph = false;
        }
    }

    if(numArrInt == REG_MAX_LEN) {
        //debug_println("--------------------------");
        numArrInt = 0;
        //printAllReg();
        WifiSend();
    }
    ST.next = WEB_REQ;
}

void printAllReg()
{

    debug_println(".................");
    int i,j;
    for(i = 0; i < REG_MAX_LEN; i++) {
        for(j = 0; j < NUM_ARR_INT; j++) {
            debug_print(saveAllArr[i][j]);
            debug_print(", ");
        }
        debug_println("");
    }
    debug_println(".................");
}

void waitRX() 
{
    //debug_println("IN RX"); 
    //delay(200);
    //two types of recive with endMark or fixLenght
    //debug_print("get_t_endMark_f_len: "); 
    //debug_println(tmpStc->get_t_endMark_f_len); 
    if(tmpStc->get_t_endMark_f_len == true) {
        //endMark
        recvWithendMarker();
    }else {
        //fixLen
        recWithFixLenght();
    }

    //////////
    static int count_missRX = 0;
    //delay(200); //wait a bit for buffer to fill
    //debug_println("choose if statment");
    if(tmpStc->get_allDataRecv == true && tmpStc->check_checkRecvMessage == true) {
        ST.next = FULL_CHECK_RX;
    } else if(tmpStc->get_allDataRecv == true && tmpStc->check_checkRecvMessage == false ) {
        //for just read RX, don't check RX arr
        ST.next = SAVE_RX;
    } else if(tmpStc->get_allDataRecv == false && tmpStc->check_checkRecvMessage == true ) {
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
    ESP.reset();
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
    int count_RX_data = 0;
    byte rc;
    int index_count_len = 0;
    int stop_count = 0;
    //debug_println("in recWithFixLen");
    while(index_count_len < m_lisaReg.check_message_len + 1) {
        if (stop_count == 200000){
            debug_println("error wait time");
            Serial.write(tmpStc->send_message, tmpStc->send_message_len);
            index_count_len = 0;
            stop_count = 0;
            error_state();

        }
        while (Serial.available() > 0) {
            rc = Serial.read();
            //debug_hex(rc);
            //debug_print("_");
            //debug_print(index_count_len);
            //debug_print(" ");
            //save into index
            m_lisaReg.get_message[index_count_len] = rc;

            //incremnet save index
            index_count_len ++;
        }
        stop_count ++;
    }
    tmpStc->get_message_len = index_count_len;
    tmpStc->get_allDataRecv = true;
    //debug_println(" ");
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
    int incIndx = 0; 

    t = tmpStc->check_message;
    r = tmpStc->get_message;
    debug_println(len_t);
    debug_println(len_r);

    //for error lenght 11 insted of 10
    // we will skip first one by incrementing array by one

    if (len_t + 1 == len_r) {
        debug_println("adding one") ;
        ESP.reset();
        incIndx = 1;
        delay(5000);
        } 

    if (len_t + incIndx == len_r) {
        debug_println("Arrays are equal len");
        for (idx = 0; idx < len_t; idx ++){
            debug_hex(t[idx]);
            debug_print("=");
            debug_hex(r[idx]);
            debug_print(", ");
            if(t[idx + incIndx] != r[idx]) return false;
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
        if(len_t + 1 == len_r){
            debug_println("one bigger in connect");
        }

        debug_println("ERROR: arrays are diff len");
        //server.send(200, "text/plain", "RESET");
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
    //debug_print("HEX to int: ");
    //debug_println(convertedToInt);

    //restet global arr to all vals to 0
    for (idx = 0; idx < idx_saveValue; idx ++){
        arrsaveValue[idx] = 0;
    }
    idx_saveValue = 0; //reset to 0 cuz is static

    //all data gone
    tmpStc->get_allDataRecv = false;

    m_lisaReg.save_message = convertedToInt;
    //ST.state = ST.next;
    //ST.newData = false;
    //debug_print("GOT VALUE: ");
    //debug_println(m_lisaReg.save_message);
    //debug_println("");
    //debug_println("");

    if(tmpStc->save_inArr == true){
        ST.next = SAVE_IN_ARR;
    } else {
        ST.next = WEB_REQ;
    }
    return true;
}

int hexToInt(int *arrSaveValue, int arr_len) {
    int rtnInt = 0; 
    int i;
    int multiplayer[4] = {4096, 256,16, 1};
    int mult = 0;

    //debug_println("HexToInt");
    for (i = 0; i < arr_len; i++){
        //debug_println(arrSaveValue[i]);
    }
    //debug_println("");
    for(i = 0; i < arr_len; i++){
        rtnInt = arrSaveValue[i] * multiplayer[i] + rtnInt;
    }
    //debug_println("RTN INT");
    //debug_println(rtnInt);
    return rtnInt;
}

void waitRXArr ()
{
    debug_println("waitRXArr");
    debug_println("Send for graph()");
    Serial.write(r_arr_graph, sizeof(r_arr_graph)); 

    //index of number of array graph points save in buffer
    static int saveGraphsIndex = 0; 
    int runningSaveIndx = 0;
    byte rc;

    if (saveGraphsIndex == MAX_GRAPH_ARR) {
        saveGraphsIndex = 0;
    }

    //must work super fast else buffer my fill up to 536
    while (runningSaveIndx < SAVE_GRAPH_POINTS){
        //debug_print(runningSaveIndx);
        //debug_print(" ");
        while (Serial.available() > 0) {
             get_arr_graph[saveGraphsIndex][runningSaveIndx] = Serial.read();
            runningSaveIndx ++;
        }
    }
    saveGraphsIndex ++;
    while (Serial.available() > 0) { 
        Serial.read();
        debug_println("error: buffer not empty!");
    }

    //printGraph(0);
    //printGraph(1);
    //printGraph(2);
}

void printGraph(int whichOne)
{
    debug_print("Grap");

    int i;
    for(i = 0; i < SAVE_GRAPH_POINTS;i++) {
        debug_print(get_arr_graph[whichOne][i]);
        debug_print(" ");
    }
}

void WifiSendGraph(){

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        debug_println("Connecting..");
    }

    if(WiFi.status() == WL_CONNECTED) {
       
        //String setup
        int i,j;
        for(i = 0; i < MAX_GRAPH_ARR; i++) {
            http.begin(ROUTE_ADDRESS);      //Specify request destination
            http.addHeader("Content-Type", "text/plain");  //Specify content-type header
            stringEmptySpace = String("Graph: ");
            for(j = 0; j < SAVE_GRAPH_POINTS; j++) {
                stringEmptySpace += get_arr_graph[i][j];
                stringEmptySpace += " ";
            }
            stringEmptySpace += ";";
            debug_println("send Graph");
            http.POST(stringEmptySpace);   //Send the request
            //String payload = http.getString();                  //Get the response payload
        http.end();  //Close connection
        }
    } else {
        //there shouldn't be error never!
        debug_println("ERROR: GRAPH DIDN't connect!");
    }
}

void WifiSend() {
    //Waiting for connction, 
    //before WiFi.status Wifi.begin must be called for connect
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        debug_println("Connecting..");
    }

    String payload;

    stringEmptySpace = String("Measure: ");
    //now must be connected but beter check again
    if(WiFi.status() == WL_CONNECTED) {
        debug_println("WiFi connected");
        debug_println("IP address: ");
        debug_println(WiFi.localIP());
    
        int counterSendReg = 0;
        int i,j;
        for(i = 0; i < REG_MAX_LEN; i++) {
        
            //String setup
            for(j = 0; j < NUM_ARR_INT; j++) {
                stringEmptySpace += saveAllArr[i][j];
                stringEmptySpace += " ";
            }
            stringEmptySpace += ";";
            if((i+1) % 10 == 0.0){

                debug_println("start");
                http.begin(ROUTE_ADDRESS);      //Specify request destination
                debug_println("begin");
                http.addHeader("Content-Type", "text/plain");  //Specify content-type header
                debug_println("Send Reg");
                http.POST(stringEmptySpace);   //Send the request

                debug_println("Send after");
                //payload = http.getString();                  //Get the response 
                //reset string
                stringEmptySpace = String("Measure ");
                stringEmptySpace += counterSendReg;
                stringEmptySpace += String(" : ");
                counterSendReg ++;
                http.end();  //Close connection
                debug_println("http.end");
            }

        }

 
        //debug_println(httpCode);   //Print HTTP return code
        //debug_println(payload);    //Print request response payload
 

        //Send graph
        WifiSendGraph();

        //Shut down wifi, send it to slep mode
        WiFi.disconnect();
        WiFi.mode( WIFI_OFF );
        WiFi.forceSleepBegin();
    }
}
