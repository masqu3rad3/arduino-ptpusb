/*
  Vinculum.h - Vinculum library for Wiring - implementation
  Copyright (c) 2009 Martin Schneider.  All right reserved.

 *  Vinculum is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  libptp2 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libptp2; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "Vinculum.h"

#define DELAY_BETWEEN_READ (1)

#if 1
#define DEBUGLOG0(x) Serial.print(x)
#define DEBUGLOGLN0(x) Serial.println(x)
#define DEBUGLOGHEX0(x) Serial.print(x,16)
#else
#define DEBUGLOG0(x) 
#define DEBUGLOGLN0(x) 
#define DEBUGLOGHEX0(x)
#endif



// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

#ifdef VNCL_SERIAL
Vinculum::Vinculum(int rxPort,int txPort):usbSerial(rxPort,txPort)
{
}
#endif
#ifdef VNCL_SPI
Vinculum::Vinculum(int aSsPort,int aMosiPort,int aMisoPort,int aSckPort)
{
	ssPort = aSsPort;
	mosiPort = aMosiPort;
	misoPort = aMisoPort;
	sckPort = aSckPort;

	spi_initialize();
}

#endif

void Vinculum::init()
{
#ifdef VNCL_SERIAL
  usbSerial.begin(9600);
#endif
//  delay(2000);
//  ecs_command((uint8_t *)"IPH");
//  ecs_command((uint8_t *)"SCS");
  errorState=false;
}

//void Vinculum:setDebugPort(Print aPrinter)
//{
//	debugPort = aPrinter;
//}

const uint8_t vnc11_ecsString[3]={
  0x11,0x0d,0x00};

// Sendet Daten über VNC1L
// Return: Status
int Vinculum::send(uint8_t *data,uint16_t length) 
{
  DEBUGLOGLN0("Send");
  errorState=false;
  byte_out(0x83);
  byte_out(0x20);
  byte_out(length);
  byte_out(0x0d);
  for (uint16_t i =0;i<length;i++) 
  {  
     byte_out(data[i]);
  }
  DEBUGLOGLN0("Send finished");
  delay(DELAY_BETWEEN_READ);
  read_prompt();  
  return 0;
}

// Daten vom VNC1L empfangen
// Return: Status
int Vinculum::receive(uint8_t *data, uint16_t *length) 
{
  int result=0;
  int receivedBytes=0;
  uint8_t bytesToRead=0;
  uint8_t carriageByte=0;

  // SCS-Daten lesen
  byte_out(0x84);
  byte_out(0x0D);

  wait_for_data(&bytesToRead);
  wait_for_data(&carriageByte);
  if (carriageByte == 0x0d) 
  {
    for (;receivedBytes<bytesToRead;receivedBytes++)
    {
      //    if (receivedBytes>=*length)
      //      break;
      uint8_t receivedByte;
      if (!wait_for_data( &receivedByte))
      {
	    signal_error(1);
        DEBUGLOGLN0("out of data");
        break;
      }
	  if (((uint16_t)receivedBytes) < *length)
	      data[receivedBytes]=receivedByte;  
	  else
	      DEBUGLOG0("-");

	  DEBUGLOGHEX0((int)receivedByte);
      DEBUGLOG0(" ");
#ifdef VNCL_SERIAL
	  delay(DELAY_BETWEEN_READ);
#endif
    }
  } 
  else {
    result=-1;
  }
  read_prompt();  

  *length=receivedBytes;

  return 0;
}

bool Vinculum::hasErrors()
{
	return errorState;
}

// Private Methods

bool Vinculum::ecs_command(const uint8_t *command)
{
#ifdef VNCL_SERIAL
  usbSerial.print((const char*)command);      
  usbSerial.print(13, BYTE);          // return character to tell vdip its end of message
#endif
#ifdef VNCL_SPI
	spi_send_string((char*)command);
	spi_send_char((char)13);
#endif

  DEBUGLOG0("ECS:");
  DEBUGLOGLN0((const char *) command);
  read_prompt();  
  return true;
}

bool Vinculum::scs_command(const uint8_t *command)
{
  DEBUGLOG0("SCS:");
  for (int i=0;command[i] != 0 ;i++) {
#ifdef VNCL_SERIAL
    usbSerial.print(command[i],BYTE);            
#endif
#ifdef VNCL_SPI
	spi_send_char(command[i]);
#endif
	DEBUGLOG0((int) command[i]);
    DEBUGLOG0(" ");
  }
  DEBUGLOG0("");
  read_prompt();
  return true;
}

void Vinculum::byte_out(uint8_t aByte)
{
//  DEBUGLOGHEX0(aByte);
//  DEBUGLOG0(" ");
#ifdef VNCL_SERIAL
  usbSerial.print(aByte,BYTE);
#endif
#ifdef VNCL_SPI
  spi_send_char(aByte);
#endif
}

#ifdef VNCL_SPI
void Vinculum::spi_send_string(char *aString) 
{
    while(*aString) {
        spi_send_char(*aString);
        aString++;
    }
}

void Vinculum::spi_send_char(char aChar) 
{
	spi_xfer_char(false,false,&aChar);
	delay(10);
//	while(!spi_xfer_char(false,false,&aChar));
}

bool Vinculum::spi_read_char (uint8_t *aByte) 
{
	return spi_xfer_char(true,false,(char*)aByte);
}

bool Vinculum::spi_read_statusregister(uint8_t *aByte)
{
	return spi_xfer_char(true,true,(char*)aByte);
}


#ifdef SPI_IN_HARDWARE
bool Vinculum::spi_xfer_char(bool readOnly, bool readRegisterData, char *pSpiData)
{
    // the 2 loops below each spin 4 usec with a 2 MHz SPI clock
    uint8_t reply1;
	uint8_t reply2;
	uint8_t send1,send2;
	send1 = (*pSpiData & 0b11111000) >> 3;
	send2 = (*pSpiData & 0b00000111) << 5;
    digitalWrite(ssPort, 1);
	SPDR = send1 | (readOnly?0b11000000:0b10000000);
    while (!(SPSR & _BV(SPIF)))
        ;
    reply1 = SPDR;

	SPDR = send2;
    while (!(SPSR & _BV(SPIF)))
        ;
    reply2 = SPDR;

	if (readOnly) 
	{
		*pSpiData = ((reply1 & 0b00011111) << 3) | ((reply2 & 0b11100000) >> 5);
	}
	digitalWrite(ssPort, 0);

	return !(reply2 & 0b00010000);
}
#else

void Vinculum::spiDelay()
{
//	delay(1);
}

bool Vinculum::spi_xfer_char(bool readOnly, bool readStatusRegister, char *pSpiData)
{
	unsigned char retData;
	unsigned char bitData;
	
	// CS goes high to enable SPI communications
	digitalWrite(ssPort,HIGH);

	// Clock 1 - Start State
	digitalWrite(mosiPort,HIGH);

	spiDelay();
	digitalWrite(sckPort,HIGH);
	spiDelay();
	digitalWrite(sckPort,LOW);

	// Clock 2 - Direction
	digitalWrite(mosiPort,readOnly?HIGH:LOW);

	spiDelay();
	digitalWrite(sckPort,HIGH);
	spiDelay();
	digitalWrite(sckPort,LOW);

	// Clock 3 - Address
	digitalWrite(mosiPort,readStatusRegister?HIGH:LOW);

	spiDelay();
	digitalWrite(sckPort,HIGH);
	spiDelay();
	digitalWrite(sckPort,LOW);

	// Clocks 4..11 - Data Phase
	bitData = 0x80;
	if (readOnly)
	{
		// read operation
		retData = 0;

		while (bitData)
		{
			digitalWrite(mosiPort,LOW);
			spiDelay();
			retData |= (digitalRead(misoPort)==HIGH)?bitData:0;
			digitalWrite(sckPort,HIGH);
			spiDelay();
			digitalWrite(sckPort,LOW);
			bitData = bitData >> 1;
		}

		*pSpiData = retData;
	}
	else
	{
		// write operation
		retData = *pSpiData;

		while (bitData)
			{
			digitalWrite(mosiPort,(retData & bitData)?HIGH:LOW);
			spiDelay();
			digitalWrite(sckPort,HIGH);
			spiDelay();
			digitalWrite(sckPort,LOW);
			bitData = bitData >> 1;
		}
	}

	// Clock 12 - Status bit

	spiDelay();
	bitData = digitalRead(misoPort);
	digitalWrite(sckPort,HIGH);
	spiDelay();
	digitalWrite(sckPort,LOW);

	// CS goes high to enable SPI communications
	digitalWrite(ssPort,LOW);


	// Clock 13 - CS low

	spiDelay();
	digitalWrite(sckPort,HIGH);
	spiDelay();
	digitalWrite(sckPort,LOW);

    return bitData==LOW;
}
#endif

#endif

bool Vinculum::byte_in(uint8_t *aByte)
{
#ifdef VNCL_SERIAL
  if (usbSerial.available()>0) 
  {
    *aByte = usbSerial.read();
    return true;
  }  
  return false;
#endif
#ifdef VNCL_SPI
  return spi_read_char ( aByte );
#endif
}

bool Vinculum::available()
{
#ifdef VNCL_SERIAL
	return usbSerial.available()>0;
#endif
#ifdef VNCL_SPI
	uint8_t aByte;
	if (!spi_read_statusregister(&aByte))
		return false;
	if (aByte & 1) 
		return false;
	return true;
#endif
}


bool Vinculum::wait_for_data(uint8_t *aByte)
{
  unsigned long time = millis();
#ifdef VNCL_SERIAL
  while (!usbSerial.available()) {
    if ((millis()-time)>3000) {
	  signal_error(1);
      DEBUGLOGLN0("Time out while waiting for data!");
      return false;
    }
  }
  *aByte = usbSerial.read();
#endif
#ifdef VNCL_SPI
  while (!spi_read_char(aByte)) {
    if ((millis()-time)>3000) {
	  signal_error(1);
      DEBUGLOGLN0("Time out while waiting for data!");
      return false;
    }
  }
#endif
  return true;
}

bool Vinculum::read_prompt()
{
  DEBUGLOG0("Read prompt:");
#ifdef VNCL_SERIAL
  while (usbSerial.available()>0) {
	  uint8_t incomingUsbByte =usbSerial.read() ;
    if (incomingUsbByte == 13) 
      DEBUGLOGLN0("");
    else {
      DEBUGLOGHEX0((int) incomingUsbByte);
      DEBUGLOG0(" ");
    }
  }
#endif
#ifdef VNCL_SPI
  uint8_t incomingUsbByte;
  while (available() & spi_read_char(&incomingUsbByte))
  {
	if (incomingUsbByte == 13) 
      DEBUGLOGLN0("");
    else {
#ifdef sonicht
      DEBUGLOGHEX0((int) incomingUsbByte);
      DEBUGLOG0(" ");
#else
      Serial.print(incomingUsbByte,BYTE);
#endif
    }

  }
  DEBUGLOGLN0("read prompt done");
#endif
  return true;
}
#ifdef VNCL_SPI
void  Vinculum::spi_initialize () {
    pinMode(ssPort, OUTPUT);
    pinMode(mosiPort, OUTPUT);
    pinMode(misoPort, INPUT);
    pinMode(sckPort, OUTPUT);

    digitalWrite(ssPort, 0);

#ifdef SPI_IN_HARDWARE

#if F_CPU <= 10000000
    // clk/4 is ok for the RF12's SPI
    SPCR = _BV(SPE) | _BV(MSTR);
#else
    // use clk/8 (2x 1/16th) to avoid exceeding RF12's SPI specs of 2.5 MHz
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
    SPSR |= _BV(SPI2X);
#endif

#else
	// Configure initial pin states

    digitalWrite(ssPort, 0);
    digitalWrite(mosiPort, 0);
    digitalWrite(sckPort, 0);
#endif
}
#endif

// Setzen der Callback Function
void Vinculum::setErrorCallback(vnc_callback_t pFunc)
{
	ErrorCallbackFunc = pFunc;
}


void Vinculum::signal_error(int aCode)
{
	// Aufruf irgendwo in rtc.c
	errorState=true;
	ErrorCallbackFunc();
}
