


/*  ER DETTE NOE VI SKAL BRY OSS OM??????
    Dokumentasjon
    Versjon 1.0
    CRV-7 Studentrakett 8
    MERK!
    Pinne D7 i JPD1 er i versjon 2.0 definert som
    en utgang som endrer tilstand fra hoey til lav naar
    navlestrengen blir separert fra nyttelasten.
    Dette kan brukes for aa
    avslutte kalibrering i
    instrument og ogsaa leses av i PCM-form
    atet for aa raskt kunne soeke frem til T-0.
*/
//



/*  Pin layout
    Pinne nr    Pinne nr    Navn
    Rakett      Arduino     rakett          xplained
    5           3           FrameStart      
    7           7           D0 (LSB)         PA0
    8           8           D1               PA1
    9           9           D2               PA2
    10          10          D3               PA3
    11          11          D4               PA4
    12          12          D5               PA5
    13          13          D6               PA6
    14          6           D7 (MSB)    (Arduiono doesn't have pin nr. 14)
*/


uint8_t FrameStart = 3;
uint8_t D0 = 7;
uint8_t D1 = 8;
uint8_t D2 = 9;
uint8_t D3 = 10;
uint8_t D4 = 11;
uint8_t D5 = 12;
uint8_t D6 = 13;
uint8_t D7 = 6;


unsigned long lastUpdate = 0;
unsigned long updateInterval = (unsigned long)1 / 2232*1000000; // in us
//unsigned long updateInterval = 1 * 1000000;

void setup()
{

    Serial.begin(115200);
    pinMode(FrameStart, OUTPUT);
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    pinMode(D3, INPUT);
    pinMode(D4, INPUT);
    pinMode(D5, INPUT);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D3, LOW);
    digitalWrite(D4, LOW);
    digitalWrite(D5, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);

}

void loop() {
    unsigned long now = micros();
    uint8_t msg  = 0;
    if (now >= lastUpdate + updateInterval) {
        digitalWrite(FrameStart, HIGH);
        delayMicroseconds(64);
        digitalWrite(FrameStart, LOW);
        delayMicroseconds(64);
        lastUpdate = now; //We don't want to let the execution time of the subsequent code affect when it should be read the next time.
        // Read the 8 pins. (If this goes too slow, we can install digitalReadFast2() or read the registers directly)
        msg = (((uint8_t)digitalRead(D7)) << 7) | (((uint8_t)digitalRead(D6)) << 6) | (((uint8_t)digitalRead(D5)) << 5) | (((uint8_t)digitalRead(D4)) << 4)  | (((uint8_t)digitalRead(D3)) << 3 ) | (((uint8_t)digitalRead(D2)) << 2 ) | (((uint8_t)digitalRead(D1)) << 1 ) | (((uint8_t)digitalRead(D0)) << 0 );
        Serial.write(msg);
    }
}


