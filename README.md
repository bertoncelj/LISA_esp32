LOG:
26.3 : CONCLUSIONS: 
        1. Serial is on pin Rx Tx and it was using for debuging on serial monitor of arduino
           because it is also set on USB C mini, so PC can see it.
           For outSerial (sending message out to lISA)
           

SERIAL_DEBUG:
     gtkterm -p /dev/ttyUSB1 -s 19200

## CONNECT PROTOCOL:
BREAK
01 42 30 03 71  " B0 q"

SIGN
2F 3F 21 0D 0A    "/?!"

2F 4C 31 35 41 5F 49 44 0D 0A   "/L15A_ID"

06 30 35 31 0D 0A    " 051"

01 50 30 02 28 00 29 03 60   P0 (<0>) 

READ reg(00 0400)  odg 0007

R1 00:04:00() g (0007)  
01 52 31 02 30 30 3A 30 34 3A 30 30 28 29  
03 67 02 28 30 30 30 37 29 03 05 



WRITE reg ( 00 052E) -> vred 7625
01 57 31 02 30 30 3A 30 35 3A 32 45 28 31 44 43 39 29 03 1B 06 01 52 31 02 30 30 3A 30 35 3A 32 45 28 29 03 11 02 28 31 44 43 39 29 03 0D 
  R1 00:05:2E()   (1DC9) 
