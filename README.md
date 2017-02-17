# GPS module for tracking sounding rocket flight path
This project used a Venus838FLPx GPS receiver to track the flight path of a sounding rocket. The data from the GPS module was received and parsed using a ATmega644 MCU. The data was transmitted from the rocket to ground-control using a propriary S-band radiotransmitter developed by NAROM. (Nasjonalt senter for romrelatert opplæring)

# Equipment
* The SkyTraq Venus838FLPx was used for receiving GPS data from satellites
* An ATmega644 was used for recieving and parsing data from the Venus838FLPx, and sending it out to the radiotransmitter
* An MEGA-1284P Xplained was used for prototyping
