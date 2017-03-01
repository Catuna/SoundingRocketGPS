import sys
import os
import serial
import threading

COMPORT = "COM3" #Update accordingly!
BAUDRATE = 115200

def monitor():
    print ("Start serial monitoring")
    while(1):
        #Try/excepts loop is in case we start this program before the arduino-program which sends to the given port.
        try:
            ser = serial.Serial(COMPORT, BAUDRATE, timeout = 0)
            break;
        except:
            print("waiting for a part to connect to")
            continue;
    readJunk = 0;
    while(1):

        msgAsByte = ser.read()
        if (msgAsByte != "" and msgAsByte !='' and msgAsByte != "\n" and msgAsByte != b"" and msgAsByte !=b'' and msgAsByte != b"\n"):
            msgAsUint8 =  int.from_bytes(msgAsByte, byteorder='little');
            readJunk +=1;
            if readJunk >= 2500: ##Hack as hell. Hver gang man begynner aa lytte (uansett hvem som starter), saa leser den feil i starten.
                #if(msgAsUint8 != 192):
                #print(msgAsUint8);
                with open("meldingFraNyttelast.txt", "a") as text_file:
                    text_file.write(str(msgAsUint8)+"\n")
                    
    print ("Stop serial monitoring")

monitor()

