/*
 * PTPUSB.h - PTPUSB library for Wiring - implementation
 *
 *  Copyright (c) 2009 Martin Schneider <schmart@gmx.de>
 *  All rights reserved.
 *
 *  PTPUSB is free software; you can redistribute it and/or modify
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

// ensure this library description is only included once
#ifndef PTPUSB_h
#define PTPUSB_h

// include types & constants of Wiring core API

#include "WProgram.h"
#include "wiring.h"

#include "NewSoftSerial.h"
#include "Vinculum.h"
#include "ptp.h"

typedef void (*vnc_callback_t) (void);


// library interface description
class PTPUSB
{
  // user-accessible "public" interface
  public:
#ifdef VNCL_SERIAL
    PTPUSB(int,int);
#endif

#ifdef VNCL_SPI
    PTPUSB(int,int,int,int);
#endif

	Vinculum vdip1;
	void init();	

	int openSession();
	int closeSession();
	int capture();
	int set_property(uint16_t aPropertyCode, uint32_t aValueCode);
	int get_property(uint16_t aPropertyCode, uint32_t *aValueCode);
	bool checkForCaptureEvent();
	uint32_t calcShutterSpeedFromFraction(uint16_t aZaehler, uint16_t aNenner);
	uint32_t calcShutterSpeedFromFloat(float time);
	int setShutter(uint16_t aZaehler, uint16_t aNenner);
	int setShutter(float aTime);
	float getShutter();

	float getBracketDistance();


  private:

	PTPUSBBulkContainer myPtpContainer;
	uint32_t SessionID;
	uint32_t Transaction_ID;
	void initPtpContainer();
	int send_command(uint16_t aCommandCode,uint8_t numParam,uint32_t param1,uint32_t param2,uint32_t param3,uint32_t param4,uint32_t param5);
	int send_data(uint16_t aCommandCode,byte *data, uint16_t size);
	int receive_response();
	int receive_all_response();
	int receive_data(uint32_t *aData);
	uint16_t receive_event_data(PTPEventData **eventData);

};

#endif

