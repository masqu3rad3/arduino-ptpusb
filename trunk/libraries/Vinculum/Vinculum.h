/*
 * Vinculum.h - Vinculum library for Wiring 
 *
 *  Copyright (c) 2009 Martin Schneider <schmart@gmx.de>
 *  All rights reserved.
 *
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 */

// ensure this library description is only included once
#ifndef Vinculum_h
#define Vinculum_h

//#define VNCL_SPI
#define VNCL_SERIAL

//#define SPI_IN_HARDWARE

// include types & constants of Wiring core API

#include "WProgram.h"
#include "wiring.h"

#ifdef VNCL_SERIAL
#include "NewSoftSerial.h"
#endif

typedef void (*vnc_callback_t) (void);


// library interface description
class Vinculum
{
  // user-accessible "public" interface
  public:
#ifdef VNCL_SPI
    Vinculum(int,int,int,int);
#endif
#ifdef VNCL_SERIAL
    Vinculum(int,int);
#endif
	void init();	
	int receive(uint8_t *data, uint16_t *length);
	int send(uint8_t *data,uint16_t length);

	bool ecs_command(const uint8_t *);
	bool scs_command(const uint8_t *);
	bool read_prompt();

	void byte_out(uint8_t);
	bool byte_in(uint8_t *);
	bool available();
	void setErrorCallback(vnc_callback_t pFunc);
	bool hasErrors();

	// library-accessible "private" interface
  private:
#ifdef VNCL_SERIAL
	NewSoftSerial usbSerial;
#endif
	bool errorState;
	bool wait_for_data(uint8_t *);
	void signal_error(int aCode);
	vnc_callback_t ErrorCallbackFunc;
#ifdef VNCL_SPI
	void spi_initialize ();
	void spi_send_string(char *aString);
	void spi_send_char(char aChar);
	bool spi_read_char(uint8_t  *aChar);
	bool spi_read_statusregister(uint8_t  *aChar);
#ifndef SPI_IN_HARDWARE
	void spiDelay();
#endif
	bool spi_xfer_char(bool readOnly, bool readRegisterData, char *pSpiData);


	int ssPort;
	int mosiPort;
	int misoPort;
	int sckPort;
#endif

};

#endif

