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

//Send messages
byte send_break[] = {0x01, 0x42, 0x30, 0x03, 0x71};
byte send_sign[] = {0x2F, 0x3F, 0x21, 0x0D, 0x0A};
byte send_nullpetena[] = {0x06, 0x30, 0x35, 0x31, 0x0D, 0x0A};

byte rec_LISA_key[] = {0x2F, 0x4C, 0x31, 0x35, 0x41, 0x5F, 0x49, 0x44, 0x0D, 0x0A};
byte rec_pZero[] = {0x01, 0x50, 0x30, 0x02, 0x28, 0x00, 0x29, 0x03, 0x60};

//STATES
typedef enum e_state_machine {
    CONNECT,
    CONF_CONNECT,
    WAIT_FOR_P0,
    CHECK_RX,
    WAIT_RX,

    WRITE
} enumSTAT;

typedef enum e_rx_arr_len {
    LisaKeyLen = 10,
    pZeroLen = 9
} enumArrLen;

typedef struct stc_state_machine {

    enumSTAT state = CONNECT;
    enumSTAT from;
    enumSTAT next;
    boolean newData = false;
    byte endMarker = 0x0A; //now is \n , but can be /r

    byte *checkArr;
    char receivedChars[MAX_REC_ARR_LEN]; // an array to store the received data
    int recArrLen;
} STATES;

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
    fillST(CONF_CONNECT, CONNECT, WAIT_RX, rec_LISA_key);
}
/*
    //wait for return message lisa
    rtn_len = recvWithendMarker();

    showNewData();
    rtn_func = checkIfCorrectData(ST.receivedChars, rtn_len, rec_LISA_key, sizeof(rec_LISA_key)/sizeof(byte));
    if (rtn_func == true) {
        debug_println("state is now CONF_CONNECT");
        ST.state = CONF_CONNECT;

    } 
}
*/

void fillST(enumSTAT nextState, enumSTAT fromState, enumSTAT currState, byte *checkArr) 
{
    ST.state = WAIT_RX;
    ST.from = CONNECT;
    ST.next = CONF_CONNECT;

}

void confConnect() 
{
    boolean rtn_func;
    int rtn_len;

    //SEND message 051
    debug_println("Send 051"); 
    debug_array(send_nullpetena, sizeof(send_nullpetena));
    Serial.write(send_nullpetena, sizeof(send_nullpetena));
    
    delay(3000);
    //wait for return message lisa
    rtn_len = recvWithendMarker();
    if (rtn_len == 0){
        debug_println("rtn_len is 0");
        ST.state = WAIT_FOR_P0;
        return; 
    }
    
    showNewData();
    //rtn_func = checkIfCorrectData(ST.receivedChars, rtn_len, rec_pZero, sizeof(rec_pZero)/sizeof(byte));
    rtn_func = true;
    if (rtn_func == true) {
        debug_println("state is now WIRTE");
        ST.state = WRITE;
    } else {
        debug_println("We didn't recive P01, so reset");
        debug_println("state is now CONNECT");
        ST.state = CONNECT;
    }
}

void recvWithendMarker() 
{
    static byte ndx = 0;
    int rtn_len = 0;
    char rc;

    debug_println("Start to read:");
    debug_println(ST.newData);
    while (Serial.available() > 0 && ST.newData == false) {
        rc = Serial.read();
        debug_hex(rc);
        debug_print(" ");

        if (rc != (char) ST.endMarker) {
            ST.receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= MAX_REC_ARR_LEN) {
                ndx = MAX_REC_ARR_LEN - 1;
                debug_println("array leak!!!");
            }
        }
        else {
            ST.receivedChars[ndx] = '\0'; // terminate the string
            ST.recArrLen = ndx;
            ndx = 0;
            ST.newData = true;
            debug_println("END");
        }
    }
}

void waitRX() 
{
    debug_println("waitRX()");
    recvWithendMarker();
    delay(50); //wait a bit for buffer to fill
    if(ST.newData == true) {
        //next from curr checkARR
        fillST(CONF_CONNECT, CONNECT, WAIT_RX, rec_LISA_key);
        ST.state = checkRX();
    }
}

void checkRX()
{
    debug_println("waitRX()");
    rtn_func = checkIfCorrectData(ST.receivedChars, rtn_len, rec_pZero, sizeof(rec_pZero)/sizeof(byte));




}

void breakLISA() 
{
    delay(100);
    debug_println("BREAK_LISA");
    debug_array(send_break, sizeof(send_break));
    Serial.write(send_break, sizeof(send_break));
    delay(900); // MUST be 900 ms !!!!
}

void waitForP0()
{
    boolean rtn_func;
    int rtn_len = 0;
    rtn_len = recvWithendMarker();
    if (rtn_len == 0){
        debug_println("rtn_len is 0");
        return; 
    }
    
    showNewData();
    //rtn_func = checkIfCorrectData(ST.receivedChars, rtn_len, rec_pZero, sizeof(rec_pZero)/sizeof(byte));
    rtn_func = true;
    if (rtn_func == true) {
        debug_println("state is now WIRTE");
        delay(10000);
        ST.state = WRITE;
    } else {
        debug_println("We didn't recive P01, so reset");
        debug_println("state is now CONNECT");
        ST.state = CONNECT;
    }
}

void serialFlash() 
{
    while(Serial.available() > 0) {
        char t = Serial.read();
    }
}

bool checkIfCorrectData(char *t, int len_t, byte *r, int len_r) 
{
    //check lenght of arrays
    int idx;
    debug_println(len_t);
    debug_println(len_r);
    if (len_t+1 == len_r) {
        debug_println("Arrays are equal len");
        for (idx = 0; idx < len_t; idx ++){
            debug_hex(t[idx]);
            debug_print("=");
            debug_print(r[idx]);
            debug_print(", ");
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


void showNewData() 
{
    if (ST.newData == true) {
        debug_println("we got data:");
        debug_println(ST.receivedChars);
        ST.newData = false;
    }
}

void loop()
{
    //STATE MACHINE
    switch(ST.state) {
        case CONNECT:
            debug_println("V1.1");
            debug_println("We are in CONNECT");
           // serialFlash();
            connectToLisa();
        break;

        case CONF_CONNECT:
            delay(3000);
            debug_println("We are in CONF_CONNECT");
            confConnect();
        break;

        case WAIT_FOR_P0:
            waitForP0();
        break;

        case CHECK_RX:
            checkRX();
        break;

        case WAIT_RX:
            waitRX();
        break;

        case WRITE:
            debug_println("We are is ST.state WRITE");
            ST.state = CONNECT;
            delay(5000);
        break;

        default:
            debug_println("default ST.state"); 
        }
    delay(2000);
}
