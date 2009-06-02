Digital Camera Remote using PTP over USB

This arduino sketch uses the ptp-usb protocol to control a digital camera. The Camera is connected via a vinculum Host-Controller. The prototyp consists out of 

1) Arduino Pro mini (5V Version with ATMEL 168)
2) VDIP1 from FTDI 
3) an optional SRF02 sensor for measuring the distance of an object in front of the sensor


I have implemented the following functions

The user interface is implemented via 3 Buttons, two LEDs and an on/off switch with integrated red status led.

a) Remote trigger (one short press on button C) 


b) Bracketed trigger  (long press on button C)
- You can setup the starting exposure time with Button A and B. A short press on button A displays the starting exposure time in the camera display. A long press reads the exposure time from the camera. Use Button B to setup and display the ending exposure time.

c) Timelapse (extra long press on button C for over 3 seconds). The duration of the button press is the time between the shoots.

d) Fokus-Trap: The fokus-trap is activated with an extra long press on button B. The distance to the object is measured with the SRF02 sensor. To setup the nearest and longest distance use button A and button B. 


There are some restrictions for the moment.

Today only nikon cameras are supported. I have tested it with a D50. There are great chances that other nikon cameras will work. I have used some nikon extension to the ptp-protocol. So there are great chances that non Nikon cameras will *not* work.

Project files

.\ptp_remote.pde		This arduino sketch uses the vinculum and ptp library

Copy the following libraries to the hardware/libraries directory.


.\libraries\PTPUSB\ptp.h		Taken from libptp library for linux 
.\libraries\PTPUSB\ptpusb.h		Header file to the ptp-arduino library
.\libraries\PTPUSB\ptpusb.cpp 		Sourcecode for the ptp-arduino library

.\libraries\Vinculum\Vinculum.cc	Low-Level Communication to the vinculum controller

.\libraries\Vinculum\Vinculum.h


PIN-Assignments

2	RX-Port of Vinculum
3	TX-Port of Vinculum
4 	BUTTON_C 
5 	BUTTON_B 
6 	BUTTON_A 
11	SRF02_SELECT 
12 	ERROR_LED 
13 	STATUS_LED 

The SRF02 is connected via I2C (A4/A5). 

