import sys
import os
import serial
import threading

COMPORT = "COM3" #Update accordingly!
BAUDRATE = 9600

def monitor():
    print ("Start serial monitoring")
    while(1):
        #Try/excepts loop is in case we start this program before the arduino-program which sends to the given port.
        try:
            ser = serial.Serial(COMPORT, BAUDRATE, timeout = 5)
            break;
        except:
            print("waiting for a part to connect to")
            continue;
    readJunk = 0;
    while(1):

        msgAsByte = ser.read()
        if (msgAsByte != "" and msgAsByte !='' and msgAsByte != "\n" and msgAsByte != b"" and msgAsByte !=b'' and msgAsByte != b"\n"):
            msgAsUint8 =  abs(int.from_bytes(msgAsByte, byteorder='little'));
            readJunk +=1;
            if readJunk >= 100: ##Hack as hell. Hver gang man begynner aa lytte (uansett hvem som starter), saa leser den feil i starten.
                #if(msgAsUint8 != 192):
                print(bin(msgAsUint8+0x100)+"  "+str(msgAsUint8));
                with open("meldingFraNyttelast.txt", "a") as text_file:
                    text_file.write(str(msgAsUint8)+"\n")
                    
    print ("Stop serial monitoring")

def test():
        ser = serial.Serial(COMPORT,BAUDRATE,timeout=5)
        run = True
        while(run):
            msgAsByte = ser.read()
            msgAsUint = abs(int.from_bytes(msgAsByte, byteorder='little'));
            print(bin(msgAsUint+0x100)+"  "+str(msgAsUint))
            
        ser.close()

def close():
        ser = serial.Serial(COMPORT,BAUDRATE,timeout=5)
            
        ser.close()
#close()
#test()
monitor()

