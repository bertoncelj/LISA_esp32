LOG:
26.3 : CONCLUSIONS: 
        1. Serial is on pin Rx Tx and it was using for debuging on serial monitor of arduino
           because it is also set on USB C mini, so PC can see it.
           For outSerial (sending message out to lISA)
## INFO:           
    BUY LIST:
    * Male pins for displays

## SERIAL_DEBUG:

    On ESP Connect TTL cable to:
    ESP       TTL
    D4    ->  white one (Rx)
    GND   ->  GND

    In terminal: 

    ```
    gtkterm -p /dev/ttyUSB1 -s 19200
    ```
    Here you choose ttyUSB0 or ttyUSB1 later in config. bar uptop.

## SSH connect to Server 
    
### Outside:
    Connect To RPi:
    
    ```
     ssh anzepi@poljch.home.kg -p 12322
    ```

    From RPi to Server connect by:

    ```
     ssh anze@192.168.1.30
    ```

    In server check if running;

    ```
    tmux ls
    ```

    Or connect directly to server to see content:

    ```
    tmux a
    ```
    
     
### Local:
    Connect To RPi:
    
    ```
     ssh anzepi@192.168.1.88
    ```

    From RPi to Server connect by:

    ```
     ssh anze@192.168.1.30
    ```

    In server check if running;

    ```
    tmux ls
    ```

    Or connect directly to server to see content:

    ```
    tmux a
    ```

## CONNECT PROTOCOL for ALISA:

BREAK
01 42 30 03 71  " B0 q"

SIGN
2F 3F 21 0D 0A    "/?!"

2F 4C 31 35 41 5F 49 44 0D 0A   "/L15A_ID"

06 30 35 31 0D 0A    " 051"

01 50 30 02 28 00 29 03 60   P0 (<0>) 

READ reg(00 0400)  odg 0007

R1 00:04:00() g (0007)  
01 52 31 02 30 30 3A 30 34 3A 30 30 28 29 03 67
02 28 30 30 30 37 29 03 05 



WRITE reg ( 00 052E) -> vred 7625
01 57 31 02 30 30 3A 30 35 3A 32 45 28 31 44 43 39 29 03 1B 06 01 52 31 02 30 30 3A 30 35 3A 32 45 28 29 03 11 02 28 31 44 43 39 29 03 0D 
  R1 00:05:2E()   (1DC9) 

READ COMMANDS:
    Send
    Rx

01 52 31 02 30 30 3A 30 32 3A 30 37 28 29 03 66 //branje temp 00 0207
02 28 30 30 42 45 29 03 05

01 52 31 02 30 30 3A 30 32 3A 30 39 28 29 03 68 //branje bat 00 0209
02 28 30 45 37 30 29 03 70

01 52 31 02 30 30 3A 30 34 3A 30 30 28 29 03 67 //branje U1 00 0400
02 28 30 30 30 30 29 03 02

01 52 31 02 30 30 3A 30 34 3A 30 31 28 29 03 66 //branje U2 00 0401
02 28 30 30 30 32 29 03 00

01 52 31 02 30 30 3A 30 35 3A 33 30 28 29 03 65 //branje Upov 00 0530
02 28 30 30 30 31 29 03 03

01 52 31 02 30 30 3A 30 34 3A 30 39 28 29 03 6E //branje ANG_ 00 0409
02 28 30 30 39 33 29 03 08

01 52 31 02 30 30 3A 30 34 3A 30 33 28 29 03 64 //branje ANG_1 00 0403
02 28 30 30 43 37 29 03 76

01 52 31 02 30 30 3A 30 34 3A 30 34 28 29 03 63 //branje ANG_2 00 0404
02 28 30 30 43 36 29 03 77

01 52 31 02 30 30 3A 30 34 3A 30 35 28 29 03 62 //branje U3 00 0405
02 28 30 30 30 30 29 03 02

01 52 31 02 30 30 3A 30 34 3A 30 36 28 29 03 61 //branje U4 00 0406
02 28 30 30 30 30 29 03 02

01 52 31 02 30 30 3A 30 35 3A 33 31 28 29 03 64 //branje ANG_tot1 00 0531
02 28 30 30 39 42 29 03 79

01 52 31 02 30 30 3A 30 34 3A 30 37 28 29 03 60 //branje ANG_3 00 0407
02 28 30 31 35 44 29 03 72

01 52 31 02 30 30 3A 30 34 3A 30 38 28 29 03 6F //branje ANG_4 00 0408
02 28 30 30 37 39 29 03 0C
