
//set output pin to high for 64us
//set output pin to low  for 64us
//read 8 pins.
//repeat with a frequency of 2232 Hz.


uint8_t FrameStart = 13;


micros()
unsigned long lastUpdate = 0;
unsigned long updateInterval = (unsigned long)1/2232;

void setup()
{
  Serial.begin(9600);
  pinMode(FrameStart, OUTPUT);

}


void loop(){
  unsigned long now = micros();
  if (now >= lastUpdate + updateInterval){
    //Do the sequence...
    
  }
}


