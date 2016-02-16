# Overview #
This arduino sketch uses the ptp-usb protocol to control a digital camera. The Camera is connected via a vinculum Host-Controller. The prototyp consists out of

  1. Arduino Pro mini (5V Version with ATMEL 168)
  1. VDIP1 from FTDI
  1. an optional SRF02 sensor for measuring the distance of an object in front of the sensor

http://schneider-detmold.de/pictures/_DSC3623.JPG

The user interface is implemented via 3 Buttons, two LEDs and an on/off switch with integrated red status led.

I have implemented the following functions

  * Remote trigger (one short press on button C)

  * Bracketed trigger  (long press on button C)
    * You can setup the starting exposure time with Button A and B. A short press on button A displays the starting exposure time in the camera display. A long press reads the exposure time from the camera. Use Button B to setup and display the ending exposure time.

  * Timelapse (extra long press on button C for over 3 seconds). The duration of the button press is the time between the shoots.

  * Fokus-Trap: The fokus-trap is activated with an extra long press on button B. The distance to the object is measured with the SRF02 sensor. To setup the nearest and longest distance use button A and button B.


http://schneider-detmold.de/pictures/_DSC3628.JPG


There are some restrictions for the moment.

Today only nikon cameras are supported. I have tested it with a D50. There are great chances that other nikon cameras will work. I have used some nikon extension to the ptp-protocol. So there are great chances that non Nikon cameras will **not** work.

## Project files ##

.\ptp\_remote.pde		This arduino sketch uses the vinculum and ptp library

Copy the following libraries to the hardware/libraries directory.


.\libraries\PTPUSB\ptp.h		Taken from libptp library for linux
.\libraries\PTPUSB\ptpusb.h		Header file to the ptp-arduino library
.\libraries\PTPUSB\ptpusb.cpp 		Sourcecode for the ptp-arduino library

.\libraries\Vinculum\Vinculum.cc
> Low-Level Communication to the vinculum controller

.\libraries\Vinculum\Vinculum.h


## PIN-Assignments ##

```
2	RX-Port of Vinculum
3	TX-Port of Vinculum
4 	BUTTON_C 
5 	BUTTON_B 
6 	BUTTON_A 
11	SRF02_SELECT 
12 	ERROR_LED 
13 	STATUS_LED 
```

The SRF02 is connected via I2C (A4/A5).

## ToDo ##

  * Communicate via SPI to the vinculum. I have implemented the communication layer. It works with a usb-memory-stick. But when I connect the D50, no communication is possible.

  * Read out the possible exposure times from the camera. Today this is hardcoded in the Vinculum Library and possibly only valid for the Nikon D50.

  * Identify the manufacturer from the camera and ...

  * ... support non-Nikon cameras. If you adapt the code to cameras from another manufacturer please let me know.

  * Make a real PCB with eagle. This is something i have never done before. Perhaps someone want to make a eagle design? Let me know!
