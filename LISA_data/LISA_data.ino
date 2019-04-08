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

#define MAX_REC_ARR_LEN 128
// teperature
byte r_arr_Temp[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x37, 0x28, 0x29, 0x03, 0x66}; 
int lisa_temp;

// battery Voltage in mV
byte r_arr_Vbat[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x39, 0x28, 0x29, 0x03, 0x68}; 
int lisa_Vbat;

// Voltage U1
byte r_arr_U1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x30, 0x28, 0x29, 0x03, 0x67}; 
int lisa_U1;

//Voltage U2
byte r_arr_U2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x31, 0x28, 0x29, 0x03, 0x66}; 
int lisa_U2;

//Voltage Upov average
byte r_arr_Upov[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x35, 0x3A, 0x33, 0x30, 0x28, 0x29, 0x03, 0x65}; 
int lisa_Upov;

//ANG angle U1 U2
byte r_arr_ANG[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x39, 0x28, 0x29, 0x03, 0x6E}; 
int lisa_ANG;

//ANG_1
byte r_arr_ANG1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x33, 0x28, 0x29, 0x03, 0x64}; 
int lisa_ANG1;

//ANG_2
byte r_arr_ANG2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x34, 0x28, 0x29, 0x03, 0x63}; 
int lisa_ANG2;

//Volage U3
byte r_arr_U3[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x35, 0x28, 0x29, 0x03, 0x62}; 
int lisa_U3;

//Voltage U4
byte r_arr_U4[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x36, 0x28, 0x29, 0x03, 0x61}; 
int lisa_U4;

//ANG_tot1 total
byte r_arr_ANG_tot1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x35, 0x3A, 0x33, 0x31, 0x28, 0x29, 0x03, 0x64}; 
int lisa_ANG_tot1;

//ANG_3
byte r_arr_ANG3[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x37, 0x28, 0x29, 0x03, 0x60}; 
int lisa_ANG3;

//ANG_4
byte r_arr_ANG4[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x38, 0x28, 0x29, 0x03, 0x6F}; 
int lisa_ANG4;


//Send messages
byte send_break[] = {0x01, 0x42, 0x30, 0x03, 0x71};
byte send_sign[] = {0x2F, 0x3F, 0x21, 0x0D, 0x0A};
byte send_nullpetena[] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};

//Send read
byte send_read_U1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x37, 0x28, 0x29};   //read temperature 000207
//byte send_read_temp2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x30, 0x28, 0x29, 0x03, 0x67};


byte send_read_temp2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x39, 0x28, 0x29, 0x03, 0x68}; //branje bat 00 0209



byte rec_LISA_key[] = {0x2F, 0x4C, 0x31, 0x35, 0x41, 0x5F, 0x49, 0x44, 0x0D, 0x0A};
byte rec_pZero[] = {0x01, 0x50, 0x30, 0x02, 0x28, 0x00, 0x29, 0x03, 0x60};


byte *r_arr_names[] = {r_arr_Temp, r_arr_Vbat, r_arr_U1, r_arr_U2, r_arr_Upov, r_arr_ANG, r_arr_ANG1, r_arr_ANG2, r_arr_U3, r_arr_U4, r_arr_ANG_tot1, r_arr_ANG3, r_arr_ANG4};

int *lisa_names[] = {&lisa_temp, &lisa_Vbat, &lisa_U1, &lisa_U2, &lisa_Upov, &lisa_ANG, &lisa_ANG1, &lisa_ANG2, &lisa_U3, &lisa_U4, &lisa_ANG_tot1, &lisa_ANG3, &lisa_ANG4};

int *export_int;

int SaveTemp;

//STATES
typedef struct stc_data {
    byte *stcArr;
    int lenArr;
    byte endMarker;
    boolean check;
    int saveValue;
} STC_LIST;

typedef enum e_state_machine {
    CONNECT,
    CONF_CONNECT,
    WAIT_FOR_P0,
    FULL_CHECK_RX,
    NOT_CHECK_RX,
    SAVE_RX,
    WAIT_RX,

    READ
} enumSTAT;

typedef struct stc_state_machine {

    enumSTAT state = CONNECT;
    enumSTAT from;
    enumSTAT next;
    boolean newData = false;
    byte receivedChars[MAX_REC_ARR_LEN]; // an array to store the received data
    int LenRecArr;

    STC_LIST recArr;

} STATES;

STC_LIST arrLisaKeyRX = {
                        .stcArr = rec_LISA_key,
                        .lenArr = 10,
                        .endMarker = 0x0A,
                        .check = true
                        };

STC_LIST arrPZeroRX = { 
                        .stcArr = rec_pZero,
                        .lenArr = 9,
                        .endMarker = 0x60,
                        .check = true
                      };

STC_LIST ReadTemp = {  
                        .stcArr = send_read_temp2,
                        .lenArr = 8,
                        .endMarker = 0xFF,
                        .check = false,
                    };


STATES ST;

//=======================================================================

void setup() 
{
    // Open serial communications and wait for port to open:
    Serial.begin(19200, SERIAL_8N2);  //morta bit 2 stop bita
    DEBUG_UART.begin(19200, SERIAL_8N2);
    while (!Serial);
}

void connectToLisa() 
{
    int rtn_len;
    boolean rtn_func;
    delay(3000);
    //send break
    breakLISA();

    //send sign
    debug_array(send_sign, sizeof(send_sign));
    Serial.write(send_sign, sizeof(send_sign)); 

    delay(3000);

    //next from curr checkARR
    fillST(CONF_CONNECT, CONNECT, WAIT_RX, arrLisaKeyRX);
}

void fillST(enumSTAT nextState, enumSTAT fromState, enumSTAT currState, STC_LIST checkArr) 
{
    ST.state = currState;
    ST.from = fromState;
    ST.next = nextState;
    ST.recArr = checkArr;
}

void confConnect() 
{
    //SEND message 051
    debug_println("Send 051"); 
    debug_array(send_nullpetena, sizeof(send_nullpetena));
    Serial.write(send_nullpetena, sizeof(send_nullpetena));
    
    delay(3000);
    //wait for return message lisa

    //next from curr checkARR
    fillST(READ, CONF_CONNECT,  WAIT_RX, arrPZeroRX);
    
}

void recvWithendMarker() 
{
    static byte ndx = 0;
    int rtn_len = 0;
    byte rc;
    
    debug_println("Start to read:");
    debug_println(ST.newData);
    while (Serial.available() > 0 && ST.newData == false) {
        rc = Serial.read();
        debug_hex(rc);
        debug_print(" ");

        if (rc !=  ST.recArr.endMarker && ndx < ST.recArr.lenArr) {
            ST.receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= MAX_REC_ARR_LEN) {
                ndx = MAX_REC_ARR_LEN - 1;
                debug_println("array leak!!!");
            }
        }
        else {
            ST.receivedChars[ndx] = rc; // terminate the string
            ST.LenRecArr = ndx + 1;
            ndx = 0;
            ST.newData = true;
            debug_println("END");
        }
    }
    debug_println("");
}

void waitRX() 
{
    debug_println("waitRX()");
    recvWithendMarker();
    delay(50); //wait a bit for buffer to fill
    if(ST.newData == true && ST.recArr.check == true) {
        ST.state = FULL_CHECK_RX;
    } else if(ST.newData == true && ST.recArr.check == false) {
        //for just read RX, don't check RX arr
        ST.state = SAVE_RX;
    } else {
        debug_println("ERROR STATE!!!");

    }
}

void checkRX()
{
    debug_println("waitRX()");
    checkIfCorrectData();
}

void breakLISA() 
{
    delay(100);
    debug_println("BREAK_LISA");
    debug_array(send_break, sizeof(send_break));
    Serial.write(send_break, sizeof(send_break));
    delay(900); // MUST be 900 ms !!!!
}

void serialFlash() 
{
    while(Serial.available() > 0) {
        char t = Serial.read();
    }
}

bool checkIfCorrectData() 
{
    debug_println("In check");
    //check lenght of arrays
    byte *t;
    byte *r;
    int len_t = ST.recArr.lenArr;
    int len_r = ST.LenRecArr;
    int idx;

    t =  ST.recArr.stcArr;
    r = ST.receivedChars;
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
        debug_println("Array CORRECT");
        ST.state = ST.next;
        ST.newData = false;
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
    debug_println("//////////////////////");
}

void sendRead() {
    //statc count list;
    //list  = [asd, asd, aasd]
    // for elm in list:
    static int nextRead = 0;
    int len_arr = 12;

    debug_println("..................");
    debug_print("READ NEXT position: ");
    debug_println(nextRead);
    ST.recArr.stcArr = r_arr_names[nextRead];
    ST.recArr.lenArr = 8;
    ST.recArr.endMarker = 0xFF;
    ST.recArr.check = false;

    export_int = lisa_names[nextRead];
    nextRead ++;
    if (nextRead == len_arr) {
        nextRead = 0;
        printAllLisa();
        delay(5000);
    }

    STC_LIST arrREADS[3] = {ReadTemp};
    debug_array(ST.recArr.stcArr, 16);
    Serial.write(ST.recArr.stcArr, 16);
    delay(200); // MUST be 900 ms !!!!

    //next from curr checkARR
    fillST(READ, READ, WAIT_RX, arrREADS[0]);
}

int hexToInt(int *arrSaveValue, int arr_len) {
    int rtnInt = 0; 
    int i;
    int multiplayer[4] = {4096, 256,16, 1};
    int mult = 0;
    debug_println("HexToInt");
    for (i = 0; i < arr_len; i++){
        debug_println(arrSaveValue[i]);
    }
    debug_println("");
    for(i = 0; i < arr_len; i++){
        rtnInt = arrSaveValue[i] * multiplayer[i] + rtnInt;
    }
    debug_println("RTN INT");
    debug_println(rtnInt);
    return rtnInt;
}

boolean saveRX() {
    byte *r;
    int len_r = ST.LenRecArr;
    int idx;
    bool inSaveModeBetweenParam = false;

    static int idx_saveValue = 0;
    static int arrsaveValue[10];

    r = ST.receivedChars;

    debug_println("RX SAVED: ");
    for (idx = 0; idx < len_r; idx ++){
        if (r[idx] == 0x29) inSaveModeBetweenParam = false;
        if (inSaveModeBetweenParam == true) {
            debug_print("SaveInSaveVAl ");
            debug_print(r[idx]);
            debug_println("");
            r[idx] -= 0x30;
            if(r[idx] > 10) r[idx] -= 0x07;
            arrsaveValue[idx_saveValue] += r[idx];
            idx_saveValue ++;
        }
        if (r[idx] == 0x28) inSaveModeBetweenParam = true;  
        
        debug_print(r[idx]);
        debug_print(", ");
    }
    debug_println();
    int rtn_int = hexToInt(&arrsaveValue[0], idx_saveValue);

    //restet global arr to all vals to 0
    for (idx = 0; idx < idx_saveValue; idx ++){
        arrsaveValue[idx] = 0;
    }
    idx_saveValue = 0; //reset to 0 cuz is static

    *export_int = rtn_int;
    ST.state = ST.next;
    ST.newData = false;
    return true;
}

void loop()
{
    //STATE MACHINE
    debug_print("curState:"); 
    debug_println(ST.state);
    switch(ST.state) {
        case CONNECT:
            debug_println("V1.2");
            debug_println("We are in CONNECT");
           // serialFlash();
            connectToLisa();
        break;

        case CONF_CONNECT:
            delay(3000);
            debug_println("We are in CONF_CONNECT");
            confConnect();
        break;

        case FULL_CHECK_RX:
            checkRX();
        break;

        case SAVE_RX:
            saveRX();
        break;

        case WAIT_RX:
            waitRX();
        break;

        case READ:
            debug_println("We are is ST.state READ");
            sendRead();
        break;

        default:
            debug_println("default ST.state"); 
        }
}
