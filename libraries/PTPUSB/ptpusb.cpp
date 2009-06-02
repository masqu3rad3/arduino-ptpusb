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

// include core Wiring API

// include this library's description file

#include "PTPUSB.h"


#if 0
#define DEBUGLOG(x) Serial.print(x)
#define DEBUGLOGLN(x) Serial.println(x)
#define DEBUGLOGHEX(x) Serial.print(x,16)
#else
#define DEBUGLOG(x) 
#define DEBUGLOGLN(x) 
#define DEBUGLOGHEX(x)
#endif

const uint32_t shutterSpeeds[]= { 
	  0x00010FA0, //(69536)      [1/4000]
	  0x00010C80,// (68736)      [1/3200]
	  0x000109C4,// (68036)      [1/2500]
	  0x000107D0,// (67536)      [1/2000]
	  0x00010640,// (67136)      [1/1600]
	  0x000104E2,// (66786)      [1/1250]
	  0x000103E8,// (66536)      [1/1000]
	  0x00010320,// (66336)      [1/800]
	  0x00010280,// (66176)      [1/640]
	  0x000101F4,// (66036)      [1/500]
	  0x00010190,// (65936)      [1/400]
	  0x00010140,// (65856)      [1/320]
	  0x000100FA,// (65786)      [1/250]
	  0x000100C8,// (65736)      [1/200]
	  0x000100A0,// (65696)      [1/160]
	  0x0001007D,// (65661)      [1/125]
	  0x00010064,// (65636)      [1/100]
	  0x00010050,// (65616)      [1/80]
	  0x0001003C,// (65596)      [1/60]
	  0x00010032,// (65586)      [1/50]
	  0x00010028,// (65576)      [1/40]
	  0x0001001E,// (65566)      [1/30]
	  0x00010019,// (65561)      [1/25]
	  0x00010014,// (65556)      [1/20]
	  0x0001000F,// (65551)      [1/15]
	  0x0001000D,// (65549)      [1/13]
	  0x0001000A,// (65546)      [1/10]
	  0x00010008,// (65544)      [1/8]
	  0x00010006,// (65542)      [1/6]
	  0x00010005,// (65541)      [1/5]
	  0x00010004,// (65540)      [1/4]
	  0x00010003,// (65539)      [1/3]
	  0x000A0019,// (655385)     [10/25]
	  0x00010002,// (65538)      [1/2]
	  0x000A0010,// (655376)     [10/16]
	  0x000A000D,// (655373)     [10/13]
	  0x00010001,// (65537)      [1/1]
	  0x000D000A,// (851978)     [13/10]
	  0x0010000A,// (1048586)    [16/10]
	  0x00020001,// (131073)     [2/1]
	  0x0019000A,// (1638410)    [25/10]
	  0x00030001,// (196609)     [3/1]
	  0x00040001,// (262145)     [4/1]
	  0x00050001,// (327681)     [5/1]
	  0x00060001,// (393217)     [6/1]
	  0x00080001,// (524289)     [8/1]
	  0x000A0001,// (655361)     [10/1]
	  0x000D0001,// (851969)     [13/1]
	  0x000F0001,// (983041)     [15/1]
	  0x00140001,// (1310721)    [20/1]
	  0x00190001,// (1638401)    [25/1]
	  0x001E0001,// (1966081)    [30/1]
	  0xFFFFFFFF,// (4294967295) [bulb]
	};




// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

#ifdef VNCL_SERIAL
PTPUSB::PTPUSB(int aRx,int aTx):vdip1(aRx,aTx)
{
}
#endif

#ifdef VNCL_SPI
PTPUSB::PTPUSB(int aSsPort,int aMosiPort,int aMisoPort,int aSckPort):vdip1(aSsPort,aMosiPort,aMisoPort,aSckPort)
{
}
#endif

void PTPUSB::init()
{
	vdip1.init();

	SessionID=0;
	Transaction_ID=0;
}

void PTPUSB::initPtpContainer()
{
  memset(&myPtpContainer,0,sizeof(myPtpContainer));
}

// Senden von Commands
int PTPUSB::send_command(uint16_t aCommandCode,uint8_t numParam,uint32_t param1,uint32_t param2,uint32_t param3,uint32_t param4,uint32_t param5)
{
  initPtpContainer();
  myPtpContainer.length=PTP_USB_BULK_REQ_LEN-(sizeof(uint32_t)*(5-numParam));
  myPtpContainer.code=aCommandCode;
  myPtpContainer.type=PTP_USB_CONTAINER_COMMAND;
  myPtpContainer.payload.params.param1 = param1;
  myPtpContainer.payload.params.param2 = param2;
  myPtpContainer.payload.params.param3 = param3;
  myPtpContainer.payload.params.param4 = param4;
  myPtpContainer.payload.params.param5 = param5;
  myPtpContainer.trans_id=Transaction_ID++;
  int ret=vdip1.send((uint8_t *)&myPtpContainer, myPtpContainer.length);

  return ret;
}

// Senden der Daten
int PTPUSB::send_data(uint16_t aCommandCode,byte *data, uint16_t size) 
{
  int ret;
  initPtpContainer();
  myPtpContainer.length=PTP_USB_BULK_HDR_LEN+size;
  myPtpContainer.code=aCommandCode;
  myPtpContainer.type=PTP_USB_CONTAINER_DATA;
  myPtpContainer.trans_id=Transaction_ID;

  memcpy(myPtpContainer.payload.data,data,
  (size<PTP_USB_BULK_PAYLOAD_LEN)?size:PTP_USB_BULK_PAYLOAD_LEN);
  /* send first part of data */
  ret=vdip1.send((uint8_t *)&myPtpContainer, PTP_USB_BULK_HDR_LEN+
    ((size<PTP_USB_BULK_PAYLOAD_LEN)?size:PTP_USB_BULK_PAYLOAD_LEN));
  if (size<=PTP_USB_BULK_PAYLOAD_LEN) return ret;
  /* if everything OK send the rest */
  ret=vdip1.send((uint8_t *) data+PTP_USB_BULK_PAYLOAD_LEN,size-PTP_USB_BULK_PAYLOAD_LEN);

  return ret;

}

// Empfangen von Daten
int PTPUSB::receive_response()
{
  initPtpContainer();
  uint16_t length=sizeof(PTPUSBBulkContainer);
  vdip1.receive((byte *)&myPtpContainer,&length);
  return length;
}

// Empfangen von Daten
int PTPUSB::receive_all_response()
{
  int result=0;
  do 
  {
    result = receive_response();
  } while(result>0);
    
  return result;
}

// Empfangen von Daten
int PTPUSB::receive_data(uint32_t *aData)
{
  initPtpContainer();
  uint16_t length=sizeof(PTPUSBBulkContainer);
  int ret=vdip1.receive((byte *)&myPtpContainer,&length);
  *aData = myPtpContainer.payload.params.param1;
  return ret;
}

// Empfangen von Daten
uint16_t PTPUSB::receive_event_data(PTPEventData **eventData)
{
  initPtpContainer();
  uint16_t length=sizeof(PTPUSBBulkContainer);
  vdip1.receive((byte *)&myPtpContainer,&length);
  char *aData = (char *) &myPtpContainer.payload.data;
  uint16_t *count = (uint16_t*)&(aData[0]);
  *eventData = (PTPEventData*)&(aData[2]);
  return *count;
}

// ************************************************************************************************
// PPT Funktionen - Midlevel Abstrakte Schicht
// *************************************************************************************************

// PPT Funktionen - Anwendung

int PTPUSB::openSession()
{
  int result = send_command(PTP_OC_OpenSession,1,1,0,0,0,0);
  receive_all_response();
  return result;
}

int PTPUSB::closeSession()
{
  int result = send_command(PTP_OC_CloseSession,1,1,0,0,0,0);
  receive_all_response();
  return result;
}

// Auslösen
int PTPUSB::capture()
{
  openSession();
  int result=send_command(PTP_OC_InitiateCapture,2,0,0,0,0,0);
//  ppt_receive_all_response();
  closeSession();
  return result;
}

int PTPUSB::set_property(uint16_t aPropertyCode, uint32_t aValueCode) 
{
  send_command(PTP_OC_SetDevicePropValue,1,aPropertyCode,0,0,0,0);
  int resultdata=send_data(PTP_OC_SetDevicePropValue,(byte *)&aValueCode,sizeof(aValueCode));
  receive_all_response();

  // sicherheitshalber noch die events lesen
  checkForCaptureEvent();
  return resultdata;
}

int PTPUSB::get_property(uint16_t aPropertyCode, uint32_t *aValueCode) 
{
  int result=send_command(PTP_OC_GetDevicePropValue,1,aPropertyCode,0,0,0,0);
  receive_data(aValueCode);
  receive_all_response();

  // sicherheitshalber noch die events lesen
  checkForCaptureEvent();
  return result;
}

// Events von der Kamera holen
bool PTPUSB::checkForCaptureEvent()
{
  bool ret=false;
  //bool err=false;
  
  send_command(PTP_OC_NIKON_CheckEvent,0,0,0,0,0,0);
  PTPEventData *eventData;  
  uint16_t count=receive_event_data(&eventData);
  if (vdip1.hasErrors())
	   return true;
  for (uint16_t i=0;i<count;i++) 
  {
    if (eventData[i].eventCode == PTP_EC_CaptureComplete)
      ret = true;
	Serial.print("Event:");
	Serial.println(eventData[i].eventCode,16);
  }
  receive_all_response();
  return ret;  
}

// aus der Tabelle die am besten passendste Belichtungszeit auswählen
uint32_t PTPUSB::calcShutterSpeedFromFraction(uint16_t aZaehler, uint16_t aNenner)
{
  float time=(float) aZaehler/aNenner;
  return calcShutterSpeedFromFloat(time);
}

uint32_t PTPUSB::calcShutterSpeedFromFloat(float time)
{
  int index=0;
  float minDist=100000;
  for (int i=0;shutterSpeeds[i] != 0xffffffff;i++)
  {
     int tabZaehler,tabNenner;
     tabZaehler = shutterSpeeds[i] / 65536;  
     tabNenner = shutterSpeeds[i] % 65536;  

     float tabTime = (float) tabZaehler / (float) tabNenner;
     float tabDistTime= tabTime-time;
	 if (tabDistTime<0) 
	 {
		tabDistTime = (-1)*tabDistTime ;
	 }
     if (tabDistTime < minDist) {
       minDist = tabDistTime;
       index= i;
     }
  }
  return shutterSpeeds[index];
}

// Belichtungszeit setzen
int PTPUSB::setShutter(uint16_t aZaehler, uint16_t aNenner)
{
  openSession();
  uint32_t value = calcShutterSpeedFromFraction(aZaehler,aNenner);
  set_property(PTP_DPC_NIKON_ExposureTime,value);
  closeSession();
  return 0;
}

// Belichtungszeit setzen
int PTPUSB::setShutter(float aTime)
{
  openSession();
  uint32_t value = calcShutterSpeedFromFloat(aTime);
  set_property(PTP_DPC_NIKON_ExposureTime,value);
  closeSession();
  return 0;
}

// Belichtungszeit von der Kamera holen
float PTPUSB::getShutter()
{
  openSession();
  uint32_t shutter;
  get_property(PTP_DPC_NIKON_ExposureTime,&shutter);
  uint16_t zaehler,nenner;
  zaehler = shutter / 65536;
  nenner = shutter % 65536;
  closeSession();
  return (float) zaehler / (float) nenner;
}

float PTPUSB::getBracketDistance()
{
  openSession();
  uint32_t shutter;
  get_property(PTP_DPC_NIKON_ExposureBracketingIntervalDist,&shutter);
  return 0.0;
}
