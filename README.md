LOG:
26.3 : CONCLUSIONS: 
        1. Serial is on pin Rx Tx and it was using for debuging on serial monitor of arduino
           because it is also set on USB C mini, so PC can see it.
           For outSerial (sending message out to lISA)
           

SERIAL_DEBUG:
     gtkterm -p /dev/ttyUSB1 -s 19200 
            Must have SERIAL_8N2


CONNECT: 
    RJAVA_MODRA_CRNA on LISA board.
    TTL on pin D4 and GND for debug
