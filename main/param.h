#ifndef param_h
#define param_h

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
#define NUM_ARR_INT 14
#define REG_MAX_LEN 3

// teperature
//
// graphCall
byte r_arr_graph[] = {0x01, 0x4F, 0x02, 0x28, 0x29, 0x03, 0x4F};
byte get_fixSize_msg[15];
byte get_arr_graph[600];

//
//index which increase every measures
byte r_arr_index[] ={0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x41, 0x28, 0x29, 0x03, 0x16};

byte r_arr_Temp[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x37, 0x28, 0x29, 0x03, 0x66}; 

// battery Voltage in mV
byte r_arr_Vbat[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x32, 0x3A, 0x30, 0x39, 0x28, 0x29, 0x03, 0x68}; 

// Voltage U1
byte r_arr_U1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x30, 0x28, 0x29, 0x03, 0x67}; 

//Voltage U2
byte r_arr_U2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x31, 0x28, 0x29, 0x03, 0x66}; 

//Voltage Upov average
byte r_arr_Upov[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x35, 0x3A, 0x33, 0x30, 0x28, 0x29, 0x03, 0x65}; 

//ANG angle U1 U2
byte r_arr_ANG[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x39, 0x28, 0x29, 0x03, 0x6E}; 

//ANG_1
byte r_arr_ANG1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x33, 0x28, 0x29, 0x03, 0x64}; 

//ANG_2
byte r_arr_ANG2[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x34, 0x28, 0x29, 0x03, 0x63}; 

//Volage U3
byte r_arr_U3[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x35, 0x28, 0x29, 0x03, 0x62}; 

//Voltage U4
byte r_arr_U4[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x36, 0x28, 0x29, 0x03, 0x61}; 

//ANG_tot1 total
byte r_arr_ANG_tot1[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x35, 0x3A, 0x33, 0x31, 0x28, 0x29, 0x03, 0x64}; 

//ANG_3
byte r_arr_ANG3[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x37, 0x28, 0x29, 0x03, 0x60}; 

//ANG_4
byte r_arr_ANG4[] = {0x01, 0x52, 0x31, 0x02, 0x30, 0x30, 0x3A, 0x30, 0x34, 0x3A, 0x30, 0x38, 0x28, 0x29, 0x03, 0x6F}; 


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


byte *r_arr_names[] = {r_arr_Temp, r_arr_Vbat, r_arr_U1, r_arr_U2, r_arr_Upov, r_arr_ANG, r_arr_ANG1, r_arr_ANG2, r_arr_U3, r_arr_U4, r_arr_ANG_tot1, r_arr_ANG3, r_arr_ANG4, r_arr_index};

int saveAllArr[REG_MAX_LEN][NUM_ARR_INT];
int *export_int;
int SaveTemp;

//STATES
/*
typedef struct stc_connection {
    byte *stcArr;
    int lenArr;
    byte endMarker;
    boolean check;
    int saveValue;
} connList;
*/

typedef enum e_state_machine {
    CONNECT,
    BREAK,
    SIGN,
    NULLPENT,
    CONF_CONNECT,
    WAIT_FOR_P0,
    FULL_CHECK_RX,
    NOT_CHECK_RX,
    SAVE_RX,
    SAVE_IN_ARR,
    WAIT_RX,
    WAIT_RX_ARR,
    WEB_REQ,
    SEND,
    GET_GRAPH,
    READ,
    EMPTY
} enumSTAT;

typedef struct stc_state_machine {
    enumSTAT next;
    enumSTAT future;
    enumSTAT from;
} stMachine;

typedef struct connection {
    //send params
    byte *send_message;
    int send_message_len;

    //recive params
    byte *get_message;
    int get_message_len;
    byte  get_endMark;
    boolean get_t_endMark_f_len;
    boolean get_allDataRecv;

    //check
    byte *check_message;
    int check_message_len;
    boolean check_checkRecvMessage;

    //save arr
    boolean save_inArr;
    int save_message;

} MSG;


typedef struct global_temp_save_stc {
    MSG *saveSTC;
}TEMP_SAVE_STC;


void updateST(enumSTAT nextState, enumSTAT futureState, enumSTAT fromState);


MSG m_LisaKeyRX = {
                .send_message = send_sign,
                .send_message_len = sizeof(send_sign),
                .get_message = get_fixSize_msg,   //save fix arr   
                .get_message_len = 0,
                .get_endMark = 0x0A,
                .get_t_endMark_f_len = true,
                .get_allDataRecv = false,
                .check_message = rec_LISA_key,
                .check_message_len = 10,
                .check_checkRecvMessage = true
                };

MSG m_nullPent = {
                .send_message = send_nullpetena,
                .send_message_len = sizeof(send_nullpetena),
                .get_message = get_fixSize_msg, //save fix arr
                .get_message_len = 0,
                .get_endMark = 0x60,
                .get_t_endMark_f_len = true,
                .get_allDataRecv = false,
                .check_message = rec_pZero,
                .check_message_len = 9,
                .check_checkRecvMessage = true
                };

MSG m_lisaReg = {
                .send_message = send_nullpetena,
                .send_message_len = 16, //all have fix len of 16
                .get_message = get_fixSize_msg, //save fix arr
                .get_message_len = 0,
                .get_endMark = 0x00,
                .get_t_endMark_f_len = false,
                .get_allDataRecv = false,
                .check_message = rec_pZero,
                .check_message_len = 8,
                .check_checkRecvMessage = false,
                .save_inArr = false

};
/*
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

*/

#endif //param.h
