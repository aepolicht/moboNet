#include <VirtualWire.h>
#include <EEPROM.h>
#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h> 

#define REPEAT 3
#define CLOCK 100
#define WAIT 600
#define LEDpin 8
#define PRINTTHETIME false

#define LEDHEADER 0x90
#define SOLID     0x01
#define STROBE    0x02
#define PULSE     0x03
#define FADE      0x04
#define AMBIANCE  0x05
#define MUSIC     0x06

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 99);
unsigned int localPort = 8888; 
char  ReplyBuffer[] = "Packet Recieved"; 
const int NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
const int UDP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
byte ntppacketBuffer[NTP_PACKET_SIZE];
byte packetBuffer[UDP_PACKET_SIZE]; 
EthernetUDP Udp;

byte storage[16][7];

unsigned long lastXmit  = 0,
			  lastTimeCheck = 0,
			  lastNotif = 0,
			  currentDay = 0,
			  currentHour = 0,
			  currentMinute = 0,
			  currentSecond = 0,
			  countDown = 0;

int txPin = 3,
	txPin_315 = 6,
	rxPin = 5;


bool alarmweek[] = {false,false,false,false,false,false,false},
	 notifyFlag  = false;
int alarmTime[2] = {EEPROM.read(100), EEPROM.read(101)}; // Hour : Minute : Days


IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov NTP server

void setup() 
{
    Serial.begin(115200);

    byte days = EEPROM.read(102);    
	for(int i = 0; i < 7; i++)	
		alarmweek[i] = GetBit(days,i);

    vw_set_tx_pin(txPin);
    vw_set_rx_pin(rxPin);
    vw_setup(4800);       // Bits per sec
    vw_rx_start();  

    Ethernet.begin(mac,ip);
 	Udp.begin(localPort);
}

void loop() 
{
    uint8_t values[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

	if( Udp.parsePacket() ) // Check for UDP message
	{
		Udp.read(packetBuffer,UDP_PACKET_SIZE);
		
		switch(packetBuffer[0])
		{
			case 0xA1:
				sendLED( packetBuffer );
			break;
			case 0xA2:
				sendAC(packetBuffer[1], packetBuffer[2]);
			break;
			case 0xA3:				
				setAlarm();
			break;
			case 0xB1:				
				// Program groups
			break;

			default:
				getTime();		
				if(currentSecond < 10)		
					parseAlarm();
			break;
		}		
	}
	if( millis() - lastTimeCheck >= 10000 )
	{
		sendNTPpacket( timeServer );
		lastTimeCheck = millis();
	}
	if( vw_get_message( values, &buflen ) )
	{
		if( values[0] == 0xA4 )
			sendNotification( values[1] );
	}
	if( millis() - lastNotif >= 3400 )
	{
		if( notifyFlag )
			eraseNotif();
	}
}


	// Helper Functions \\ 

void sendLED( byte *packetBuffer )
{
	byte _buffer[7];
	_buffer[0] = LEDHEADER;
	for(int i=1; i<7; i++)
		_buffer[i] = packetBuffer[i+1]; // Recieve RGB values

	broadcast( _buffer, 7 );
}

void sendNotification( byte notif )
{
	byte _buffer[7];

	_buffer[0] = LEDHEADER;
	_buffer[1] = (byte) 3;
    switch( notif )
    {
      case 0x01: _buffer[2] = 0;   _buffer[3] = 255; _buffer[4] = 0;   break;
      case 0x02: _buffer[2] = 255; _buffer[3] = 0;   _buffer[4] = 0;   break;
      case 0x03: _buffer[2] = 0;   _buffer[3] = 0;   _buffer[4] = 255; break;
      case 0x04: _buffer[2] = 255; _buffer[3] = 255; _buffer[4] = 0;   break;
      case 0x05: _buffer[2] = 0;   _buffer[3] = 255; _buffer[4] = 255; break;
      case 0x06: _buffer[2] = 255; _buffer[3] = 0;   _buffer[4] = 255; break;
    }    
    _buffer[5] = (byte) 255;
    _buffer[6] = (byte) 25;

    notifyFlag = true;
    lastNotif = millis();

	broadcast( _buffer, 7 );
}

void broadcast( byte _buffer[7], int numBytes )
{
	for(int j = 0; j < REPEAT; j++)
	{
		vw_send((uint8_t *)_buffer, numBytes);
		vw_wait_tx(); 
	}

	if(numBytes == 8 && notifyFlag == false)
		for(int i=0; i<7; i++)
		{
			EEPROM.write(i, _buffer[i]);
			storage[0][i] = _buffer[i];
		}

	lastXmit = millis();
}

void eraseNotif()
{
	byte _buffer[7];
	_buffer[0] = LEDHEADER;
	notifyFlag = false;
	for(int i =1; i<7; i++)
		_buffer[i] = storage[0][i];
	broadcast(_buffer, 7);
}

unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
  memset(ntppacketBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  ntppacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntppacketBuffer[1] = 0;     // Stratum, or type of clock
  ntppacketBuffer[2] = 6;     // Polling Interval
  ntppacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntppacketBuffer[12]  = 49; 
  ntppacketBuffer[13]  = 0x4E;
  ntppacketBuffer[14]  = 49;
  ntppacketBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(ntppacketBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}

void setAlarm()
{
	alarmTime[0] = packetBuffer[1];
	alarmTime[1] = packetBuffer[2];
	byte days    = packetBuffer[3];

	for(int i = 0; i < 7; i++)
		alarmweek[i] = GetBit(days, i);

	EEPROM.write(100, alarmTime[0]);
	EEPROM.write(101, alarmTime[1]);
	EEPROM.write(102, days);
}

void parseAlarm()
{
	bool flag = false;
	byte _buffer[7];

	_buffer[0] = LEDHEADER;

	for(int i = 0; i<7; i++)
		flag = flag || ( currentDay && alarmweek[i] );

	if( flag )
	{
		if( mintillAlarm() == 30 )
		{
			_buffer[1] = SOLID;
			_buffer[2] = 0x00;
			_buffer[3] = 0xFF;
			_buffer[4] = 0x00;
			_buffer[5] = (byte) 255*1/4;
			_buffer[6] = 0xFF;
			broadcast(_buffer, 7);
		} else if ( mintillAlarm() == 20 )
		{
			_buffer[1] = SOLID;
			_buffer[2] = 0xFF;
			_buffer[3] = 0xFF;
			_buffer[4] = 0x00;
			_buffer[5] = (byte) 255*2/4;
			_buffer[6] = 0xFF;
			broadcast(_buffer, 7);
		} else if ( mintillAlarm() == 10 )
		{
			_buffer[1] = SOLID;
			_buffer[2] = 0xFF;
			_buffer[3] = 0x00;
			_buffer[4] = 0x00;
			_buffer[5] = (byte) 255*3/4;
			_buffer[6] = 0xFF;
			broadcast(_buffer, 7);

		} else if ( mintillAlarm() == 0 )
		{	
			_buffer[1] = STROBE;
			_buffer[2] = 0xFF;
			_buffer[3] = 0xFF;
			_buffer[4] = 0xFF;
			_buffer[5] = (byte) 255;
			_buffer[6] = 0x7F;
			broadcast(_buffer, 7);
			sendAC(0x01,true);
			sendAC(0x02,true);
			sendAC(0x03,true);
			sendAC(0x04,true);
		} else if ( mintillAlarm() == 1435 )
		{
			// return to previous state
			for(int i =1; i<7; i++)
				_buffer[i] = storage[0][i];
			broadcast(_buffer, 7);
		}
	}
}

int mintillAlarm()
{
	int minCurrent = currentHour*60 + currentMinute;
	int minAlarm   = alarmTime[0]*60 + alarmTime[1];

	if( minAlarm >= minCurrent )
		return minAlarm - minCurrent;
	else
		return 1440 - minCurrent + minAlarm;
}

void getTime()
{
	unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;        

    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;     
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;                     

    currentDay = (epoch / 86400L) % 7;

    if((epoch  % 86400L) / 3600 < 5)
    	currentHour = (epoch  % 86400L) / 3600 + 19;
	else
    	currentHour = (epoch  % 86400L) / 3600 - 5;
    currentMinute = (epoch  % 3600) / 60;
    currentSecond = epoch %60;

    if( PRINTTHETIME )
    {
	    // print the hour, minute and second:
	    Serial.print("The CST time is ");       // UTC is the time at Greenwich Meridian (GMT)
	    Serial.print(currentHour); // print the hour (86400 equals secs per day)
	    Serial.print(':');  
	    if ( currentHour < 10 ) {
	      // In the first 10 minutes of each hour, we'll want a leading '0'
	      Serial.print('0');
	    }
	    Serial.print(currentMinute); // print the minute (3600 equals secs per minute)
	    Serial.print(':'); 
	    if ( currentMinute < 10 ) {
	      // In the first 10 seconds of each minute, we'll want a leading '0'
	      Serial.print('0');
	    }
	    Serial.println(currentSecond); // print the second

	    Serial.print("Alarm set for ");
	    if ( alarmTime[0] < 10 ) {
	      // In the first 10 seconds of each minute, we'll want a leading '0'
	      Serial.print('0');
	    }
	    Serial.print(alarmTime[0]);
	    Serial.print(":");
	    if ( alarmTime[1] < 10 ) {
	      // In the first 10 seconds of each minute, we'll want a leading '0'
	      Serial.print('0');
	    }
	    Serial.println(alarmTime[1]);
	}
}

	// Functions for 315 MHz AC Relay \\

void sendAC(byte light, bool val)
{
  	byte head = 0;
    pinMode(txPin_315, OUTPUT);

  	switch(light)
	{
	  case 0x01: head = 0x0A; break;
	  case 0x02: head = 0x02; break; 
	  case 0x03: head = 0x08; break; 
	  case 0x04: head = 0x00; break;
	  default: head = 0x0F; break;
	}

  	if( val )
  	{
	    for(int i=0; i<10; i++)
	    {   
			// A->1010 B->0010 C->1000 D->0000
			for(int i = 4; i>=0; i--)
				sendBit(GetBit(head,i));

			// 10 1010 1100 0000 = 0x2AC0
			byte body = 0x2AC0;

			// for(int i = 13; i>=0; i--)
			//   sendBit(GetBit(body,i));

			// 1010
			sendBit(1);
			sendBit(0);
			sendBit(1);
			sendBit(0);
			// 1011
			sendBit(1);
			sendBit(0);
			sendBit(1);
			sendBit(1);
			// 0000
			sendBit(0);
			sendBit(0);
			sendBit(0);
			sendBit(0);
			// 00
			sendBit(0);
			sendBit(0);

			digitalWrite(txPin_315, LOW);

			delay(10);
		}	
	}
	else
	{
		for(int i=0; i<10; i++)
		{ 
			// A->1010 B->0010 C->1000 D->0000
			for(int i = 4; i>=0; i--)
				sendBit(GetBit(head,i));

			byte body = 0x2A30;
			// for(int i = 13; i>=0; i--)
			//   sendBit(GetBit(body,i));

			// 1010
			sendBit(1);
			sendBit(0);
			sendBit(1);
			sendBit(0);
			// 1000
			sendBit(1);
			sendBit(0);
			sendBit(0);
			sendBit(0);
			// 1100
			sendBit(1);
			sendBit(1);
			sendBit(0);
			sendBit(0);
			// 00
			sendBit(0);
			sendBit(0);

			digitalWrite(txPin_315, LOW);

			delay(10);
		}
 	}

    pinMode(txPin_315, INPUT);
	lastXmit = millis();	
}

void sendBit(bool val)
{
    digitalWrite(txPin_315, val);
    delayMicroseconds(WAIT);
    digitalWrite(txPin_315, LOW);
    delayMicroseconds(CLOCK);
    digitalWrite(txPin_315, HIGH);
    delayMicroseconds(CLOCK);
}

bool GetBit(byte b, int bitNumber) 
{
   return (b & (1 << bitNumber)) != 0;
}