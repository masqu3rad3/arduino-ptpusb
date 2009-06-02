
/*
 *  ptp-remote interface for Arduino
 *
 *  Copyright (c) 2009 Martin Schneider <schmart@gmx.de>
 *  All rights reserved.
 *
 *  ptp-remote is free software; you can redistribute it and/or modify
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

#include <NewSoftSerial.h>
#include <Vinculum.h>
#include <ptpusb.h>

#define SRF_ENABLED
//#define TEST_ENABLED
//#define RF12_ENABLED
#define HARDWARE_BUTTONS

#include <EEPROM.h>
#include "Wire.h"


#ifdef SRF_ENABLED
#include "SRF02.h"
SRF02 sensor(0x70, SRF02_CENTIMETERS);
#endif

/*
Belegung RFM12B Modul
Einstellung in RF12.cpp vornehmen
GND            schwarz
VCC            rot 
RFM_IRQ  2     grau
SPI_SS   10    blau
SPI_MOSI 11    braun
SPI_MISO 12    gruen
SPI_SCK  13    orange
*/

#ifdef RF12_ENABLED 
#include "RF12.h"
#include <util/crc16.h>
#include <avr/eeprom.h>
#endif


// Ports vom VDIP1

#ifdef VNCL_SERIAL

// Konflikt mit Funkempfaenger

#ifdef RF12_ENABLED
// RX liegt an VDIP1 Port 8, VDRIVE2 gelb
#define VDIP1_RX 14
// TX liegt an VDIP1 Port 6, VDRIVE2 orange
#define VDIP1_TX 15
#else
#define VDIP1_RX 2
#define VDIP1_TX 3
#endif
#endif

#ifdef VNCL_SPI
// VDRIVE Port 6  - grün
#define SPI_SS_PORT    9
// VDRIVE Port 4  - orange
#define SPI_MOSI_PORT  11
// VDRIVE Port 2  - braun
#define SPI_MISO_PORT  12 
// VDRIVE Port 5  - gelb
#define SPI_CLK_PORT   13
#endif
// Ports der Taster

// Taster A - Einstellung der Startzeit
#define BUTTON_A 6
#define BUTTON_B 5
#define BUTTON_C 4

#define BUTTON_EXTRA_LONG_PRESSED 3
#define BUTTON_LONG_PRESSED 2
#define BUTTON_SHORT_PRESSED 1
#define BUTTON_NOT_PRESSED 0

#ifdef RF12_ENABLED
#define STATUS_LED 7
#define ERROR_LED 8
#else
#ifdef VNCL_SPI
#define STATUS_LED 7
#define ERROR_LED 8
#define SRF02_SELECT 6
#else
#define STATUS_LED 13
#define ERROR_LED 12
#define SRF02_SELECT 11
#endif
#endif



#if 0
#define DEBUGLOG(x) Serial.print(x)
#define DEBUGLOGLN(x) Serial.println(x)
#else
#define DEBUGLOG(x) 
#define DEBUGLOGLN(x) 
#endif

// Konstanten zum Signalisieren des Zustandes

#define STATE_IDLE              0
#define STATE_CONNECTED         1
#define STATE_ACTIVE            2
#define STATE_ERROR             3

uint8_t board_state=STATE_IDLE;
uint32_t activity_timer=0;
bool activity_state=false;


uint32_t button_a_start=0;
uint32_t button_b_start=0;
uint32_t button_c_start=0;

uint32_t button_last_duration=0;

#ifdef SRF_ENABLED
bool srf02_enabled=false;
#endif


// set up a new serial port

#ifdef VNCL_SERIAL
PTPUSB ptpUsb(VDIP1_RX, VDIP1_TX);
#endif
#ifdef VNCL_SPI
PTPUSB ptpUsb(SPI_SS_PORT,SPI_MOSI_PORT,SPI_MISO_PORT,SPI_CLK_PORT);
#endif

// Prototypen

void handle_activity_led();
void set_error();
void eeprom_read(char *aMem,uint16_t aEepromIndex,uint16_t aSize);
void eeprom_save(char *aMem,uint16_t aEepromIndex,uint16_t aSize);
bool captureAndWait(float time);
void set_active();  
void set_idle();
void set_connected();

// Interrupthandler 
// Versorgt die StatusLed

ISR(TIMER2_OVF_vect)
{
  static uint8_t count=1;

  TCNT2 -= 250; //1000 Hz
  

  if(--count == 0) //100 Hz
  {
    count = 10;
    handle_activity_led();  
  }
}

#define CONFIG_TIMER_MAGIC  0x1312
#define CONFIG_RF12_MAGIC   0x1411
#define CONFIG_SRF_MAGIC    0x101B

typedef struct {

  int timer_magic;
  float startTimer;
  float endTimer;
  
#ifdef RF12_ENABLED
  int rf12_magic;
  uint8_t RF12_NodeId;
  uint8_t RF12_Group;
  uint8_t RF12_Band;
#endif

#ifdef SRF_ENABLED
  // bool srf02_enabled;
  int srf_magic;
  int shortDistance;
  int longDistance;
#endif
} 
RemoteConfig;

RemoteConfig Config;

void initializeTimerConfig()
{
  Config.startTimer=3.0;
  Config.endTimer=0.0025;
}

#ifdef RF12_ENABLED
void initializeRF12Config()
{
    Config.RF12_NodeId = 0x01; // node A1 @ 433 MHz
    Config.RF12_Group = 0xD4;
    Config.RF12_Band = 0x02; // 0x01 = 433 , 0x02 = 868
}
#endif

#ifdef SRF_ENABLED
void initializeSRFConfig()
{
    Config.shortDistance=150;  
    Config.longDistance=250;
}
#endif

void readConfig()
{
  eeprom_read((char*)&Config,0,sizeof(Config));  

  if (Config.timer_magic != CONFIG_TIMER_MAGIC)
  {
    initializeTimerConfig();
  }

#ifdef RF12_ENABLED
  if (Config.rf12_magic != CONFIG_RF12_MAGIC)
  {
    initializeRF12Config();
  }
#endif
#ifdef SRF_ENABLED
  if (Config.srf_magic != CONFIG_RF12_MAGIC)
  {
    initializeSRFConfig();
  }
#endif

}

void saveConfig()
{
  Config.timer_magic=CONFIG_TIMER_MAGIC;

#ifdef RF12_ENABLED
  Config.rf12_magic=CONFIG_RF12_MAGIC;
#endif
#ifdef SRF_ENABLED
  Config.srf_magic=CONFIG_RF12_MAGIC;
#endif
  
  eeprom_save((char*)&Config,0,sizeof(Config));  

}

void setup() {
  delay(500);

  Serial.begin(57600);	// opens serial port, sets data rate to 9600 bps

  // set the data rate for the SoftwareSerial port

  
  ptpUsb.init();
  ptpUsb.vdip1.setErrorCallback(&set_error);
  
  pinMode(BUTTON_A, INPUT);  
  pinMode(BUTTON_B, INPUT);
  
  pinMode(STATUS_LED, OUTPUT);  
  pinMode(ERROR_LED, OUTPUT);

  //init Timer2
  TCCR2B  = (1<<CS22); //clk=F_CPU/64
  TCNT2   = 0x00;
  TIMSK2 |= (1<<TOIE2); //enable overflow interupt

  //interrupts on
  sei();

  readConfig();
  

  // initial den Abstanzsensor ausschalten
#ifdef SRF_ENABLED
  pinMode(SRF02_SELECT,OUTPUT);

  if (srf02_enabled==true)
  {
    digitalWrite(SRF02_SELECT,HIGH);
  } else {
    digitalWrite(SRF02_SELECT,LOW);
  }

 Wire.begin();
#endif

#ifdef RF12_ENABLED
  // rf12_initialize(Config.RF12_NodeId,Config.RF12_Band,Config.RF12_Group);
#endif

  delay(1000);
  ptpUsb.vdip1.read_prompt();
//  ptpUsb.vdip1.ecs_command((const uint8_t*)"FWV");
  ptpUsb.vdip1.ecs_command((const uint8_t*)"IPH");
  ptpUsb.vdip1.ecs_command((const uint8_t*)"SCS");
}


// Ab hier folgt das eigentliche Anwendungscoding

bool capture_time(float time)
{  
    ptpUsb.setShutter(time);
    captureAndWait(time);
}

bool capture()
{  
    // 3 Sekunden max. warten
   captureAndWait(3);
}

bool captureAndWait(float time)
{
    set_active();  
    ptpUsb.capture();
    uint32_t mstimeToWait = (time*1000)+3000; // 3 Sekunden extra
    uint32_t eventDelay=50;
    for (int i=0;i<mstimeToWait/eventDelay;i++) 
    {
      if (ptpUsb.checkForCaptureEvent())
      {
        set_idle();
        return true;
      }
      delay(eventDelay);
    }
    return false;
}

void belichtungsReihe(float timeVon, float timeBis)
{
  set_active();
  for(float time = timeVon; time >= timeBis; time = time / 4 )
  {
    if (!capture_time(time))
    {
      Serial.println("error!");
      set_error();
      return;
    }
  }
  capture_time(timeBis);
  set_idle();
}

void timelapse(uint32_t aRepeatTime)
{  
  while(true) 
  {
    set_active();  
    uint32_t startTime=millis();
    capture();
    uint32_t delayTime = aRepeatTime- (millis()-startTime);
    set_connected();  
    delay(delayTime);
  }
}

long nextRead = 0;

#ifdef SRF_ENABLED

int distance=0;
//int shortDistance;
//int longDistance;

void activate_focusBat()
{
    set_active();  
    srf02_enabled=true;
    digitalWrite(SRF02_SELECT,HIGH);
    delay(100);
    SRF02::setInterval(250);
} 

void handle_focusBat()
{
  SRF02::update();
  if (millis() > nextRead)
  {
    distance = sensor.read();
//    Serial.println(distance);
    if ((distance>Config.shortDistance) & (distance<Config.longDistance)) 
    {
  //      Serial.println("S");
        capture();         
    } 
    
    nextRead = millis () + 250;
  }    
}

void measureShortDistance()
{
   Config.shortDistance=distance;
   saveConfig();
}

void measureLongDistance()
{
   Config.longDistance=distance-20;
   saveConfig();
}
#endif

// Auslesen und Setzen der Belichtungszeiten
void read_startTimer()
{
   set_active();  
   Config.startTimer = ptpUsb.getShutter();
   saveConfig();
   set_idle();
}

void read_endTimer()
{
   set_active();  
   Config.endTimer = ptpUsb.getShutter();
   saveConfig();
   set_idle();
}
  
void set_startTimer()
{
   set_active();  
   ptpUsb.setShutter(Config.startTimer);
   set_idle();
}

void set_endTimer()
{
   set_active();  
   ptpUsb.setShutter(Config.endTimer);
   set_idle();
}

// Buttons entprellen und auslesen
int handleButton(int aButton,uint32_t *time)
{
  uint32_t duration = millis()-(*time);
  if (digitalRead(aButton) == HIGH)
  {
    if (*time == 0)
      *time=millis();
  } else {
    if (*time > 0)
    {
      // Taster war gedrueckt und wurde geöffnet
      *time=0;
      button_last_duration=duration;
      if (duration>3000) 
      {
        return BUTTON_EXTRA_LONG_PRESSED;
      }
      if (duration>500) 
      {
        return BUTTON_LONG_PRESSED;
      }
      if (duration>5) 
      {
        return BUTTON_SHORT_PRESSED;
      }
    }   
  }
  return BUTTON_NOT_PRESSED;  
}

// Methoden zum Signalisieren der Aktivität
// Setzen der Aktivitätsled
// wird im Interrupthandler aufgerufen
void handle_activity_led()
{
  if (millis()>activity_timer)
  {
    if (activity_state==LOW)
    {
      activity_state=HIGH;
    } else {
      activity_state=LOW;      
    }
    digitalWrite(STATUS_LED,activity_state);      
#ifdef SRF_ENABLED
    if (srf02_enabled)
    {
      activity_timer = millis()+distance+10;
    } else {
#endif
      activity_timer = millis()+((4-board_state)*250);
#ifdef SRF_ENABLED
    }
#endif
  }
}

void set_active()
{
  board_state = STATE_ACTIVE;
}

void set_idle()
{
  board_state = STATE_IDLE;
}

void set_error()
{
  Serial.println("Error");
  board_state = STATE_ERROR;
}

void set_connected()
{
  board_state = STATE_CONNECTED;
}



// Speichern/Lesen der Timerwerte ins EEPROM


void eeprom_save(char *aMem,uint16_t aEepromIndex,uint16_t aSize)
{
  for (int i=0;i<aSize;i++)
  {
    EEPROM.write(i+aEepromIndex,aMem[i]);
  }
}

void eeprom_read(char *aMem,uint16_t aEepromIndex,uint16_t aSize)
{
  for (int i=0;i<aSize;i++)
  {
     aMem[i] = EEPROM.read(i+aEepromIndex);
  }
}


void loop()
{
  float shutter;
  // send data only when you receive data:
  uint8_t incomingUsbByte,incomingComByte;
  if (ptpUsb.vdip1.byte_in(&incomingUsbByte)) {
    if (incomingUsbByte == 13) 
      Serial.println("");
    else
      Serial.print(incomingUsbByte,BYTE);
  }
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingComByte = Serial.read();
#ifdef TEST_ENABLED
    switch (incomingComByte) {
      case '3' : 
      shutter = ptpUsb.getBracketDistance();
      Serial.print("D:");
      Serial.println(shutter*10000);
      break;
    case '4' : 
      shutter = ptpUsb.getShutter();
      Serial.print("S:");
      Serial.println(shutter*10000);
      break;
    case 'e' : 
      ptpUsb.checkForCaptureEvent();
      break;
    case 'c' : 
      capture();
      break;
    case '1':
        set_startTimer();
        break;
    case '2':
        set_endTimer();
        break;
      
    case '#':
      Serial.print(0x0d,BYTE);
      ptpUsb.vdip1.byte_out(0x0d);      
      break;
    default :
      Serial.print(incomingComByte,BYTE);
      ptpUsb.vdip1.byte_out(incomingComByte);      
      break;
    }
 #endif   
  }
  
#ifdef SRF_ENABLED
  if (srf02_enabled)
  {
    handle_focusBat();
  }
#endif

#ifdef HARDWARE_BUTTONS
  // Abfragen der 3 Buttons
  switch (handleButton(BUTTON_A,&button_a_start)) 
  {
    case BUTTON_LONG_PRESSED:
#ifdef SRF_ENABLED
        if (srf02_enabled) 
          measureShortDistance();
        else 
#endif
          read_startTimer();
        break;
    case BUTTON_SHORT_PRESSED:
        set_startTimer();
        break;
  }

  switch (handleButton(BUTTON_B,&button_b_start)) 
  {
    case BUTTON_EXTRA_LONG_PRESSED:
#ifdef SRF_ENABLED
        activate_focusBat();
#endif
        break;
    case BUTTON_LONG_PRESSED:
#ifdef SRF_ENABLED
        if (srf02_enabled) 
          measureLongDistance();
        else 
#endif
          read_endTimer();
        break;
    case BUTTON_SHORT_PRESSED:
        set_endTimer();
        break;
        
  }

  switch (handleButton(BUTTON_C,&button_c_start)) 
  {
    case BUTTON_EXTRA_LONG_PRESSED:
        timelapse(button_last_duration);
        break;
    case BUTTON_LONG_PRESSED:
        belichtungsReihe(Config.startTimer,Config.endTimer);
        break;
    case BUTTON_SHORT_PRESSED:
        capture();
        break;
  }
#endif
}
