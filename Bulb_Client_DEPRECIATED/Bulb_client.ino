#include <VirtualWire.h>
#include <EEPROM.h>

#define UID 0x01
#define LEDHEADER 0x90
#define SOLID     0x01
#define STROBE    0x02
#define PULSE     0x03
#define FADE      0x04
#define AMBIANCE  0x05
#define MUSIC     0x06
#define real64_T float
#define BL 32

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

byte modCode, redVal, greenVal, blueVal, speedVal, submode, groupNum, inten, pps, musicthres;

unsigned long lastx = 0,
			  maxtimeout = 0;

// Pins
int redPin   = 3, 
	bluePin  = 6, 
	greenPin = 5, 
	rxpin    = 7,
	micPin   = A0;

// Values
int red    = 0, 
	green  = 0, 
	blue   = 0, 
	flash  = 100,
	notif  = 0,
	musAve = 0,
	musCnt = 0,
	val = 0,
	maxAmp = 0,
	Ts = 166;            // sampling period in microseconds [6000 Hz (max of ~9000Hz)]

int data_IN[BL];

// Time keeping
unsigned long millislast = 0,
			  notiflast  = 0,
			  timeKeeper = 0;

#define BL 32
const real64_T LP[32] = {
  7.721061893432e-05,0.0005722734696623, 0.001426237191501, 0.003052593168767,
   0.005674100021448, 0.009558090186268,  0.01488990809316,   0.0217362872309,
    0.02999783988332,  0.03938382675721,  0.04941269219336,  0.05944344335524,
    0.06873701141268,  0.07653832561754,  0.08217073615837,  0.08512397533896,
    0.08512397533896,  0.08217073615837,  0.07653832561754,  0.06873701141268,
    0.05944344335524,  0.04941269219336,  0.03938382675721,  0.02999783988332,
     0.0217362872309,  0.01488990809316, 0.009558090186268, 0.005674100021448,
   0.003052593168767, 0.001426237191501,0.0005722734696623,7.721061893432e-05
};

void setup() 
{
	pinMode(redPin,   OUTPUT);  
  	pinMode(greenPin, OUTPUT);  
  	pinMode(bluePin,  OUTPUT); 
  	pinMode(13,       OUTPUT);

  	digitalWrite(13, LOW);

    vw_set_rx_pin(rxpin);
    vw_setup(4800);	 // Bits per sec
    vw_rx_start();       // Start the receiver PLL running

	sbi(ADCSRA,ADPS2);
	cbi(ADCSRA,ADPS1);
	cbi(ADCSRA,ADPS0);

	analogWrite(redPin,   50);
	analogWrite(greenPin, 0);
	analogWrite(bluePin,  0);  
	delay(500);
	analogWrite(redPin,   0);
	analogWrite(greenPin, 50);
	analogWrite(bluePin,  0);  
	delay(500);
	analogWrite(redPin,   0);
	analogWrite(greenPin, 0);
	analogWrite(bluePin,  50);  
	delay(500);
    analogWrite(redPin,   0);
	analogWrite(greenPin, 0);
	analogWrite(bluePin,  0);  

	 modCode = EEPROM.read(0);
	  redVal = EEPROM.read(1);
	greenVal = EEPROM.read(2);
	 blueVal = EEPROM.read(3);
	   inten = EEPROM.read(4);
	   flash = EEPROM.read(5);
}

void loop() 
{
    uint8_t values[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

	if( vw_get_message(values, &buflen) )
	{
		if( values[0] == LEDHEADER )
		{
			modCode  = (byte) values[1];
			redVal   = (byte) values[2];
			greenVal = (byte) values[3];
			blueVal  = (byte) values[4];
			pps      = (byte) values[5];
			flash    = (int)  values[6];

			EEPROM.write(0,  modCode);
			EEPROM.write(1,   redVal);
			EEPROM.write(2, greenVal);
			EEPROM.write(3,  blueVal);
			EEPROM.write(4,    inten);
			EEPROM.write(5,    flash);

			if( modCode == MUSIC )
				timeKeeper = millis();

			red = (int) redVal;
			green = (int) greenVal;
			blue = (int) blueVal;
			inten = (int) pps;
		}
	}

	switch( modCode ) // Check Modcode
	{
		case SOLID:    Solid();    break;
		case STROBE:   Strobe();   break;
		case PULSE:    Pulse();    break;
		case FADE:     Fade();     break;
		case AMBIANCE: Ambiance(); break;
		case MUSIC:    Music(800); break;
		default:       Solid();
	}	

	analogWrite(redPin,   red   * inten / 255);
	analogWrite(greenPin, green * inten / 255);
	analogWrite(bluePin,  blue  * inten / 255);  
}

void Fade()
{
	if(millis()-millislast >= (flash / 10))
	{
		if ((red < 255 && green == 0 && blue == 0) || (red < 255 && green == 255 && blue == 255) )
	    	red += 1;    
	  	if ((red > 0 && green == 255 && blue == 0) || (red > 0 && green == 0 && blue == 255) )
		    red -= 1;

	  	if (red == 255 && green < 255 && blue == 0)
		    green += 1;    
	  	if (red == 255 && green > 0 && blue == 255)
		    green -= 1;

	  	if (red == 0 && green == 255 && blue < 255)
		    blue += 1;    
	  	if (red == 0 && green == 0 && blue > 0)
		    blue -= 1;

		if ( red > 255) 
		    red = 255;
		if ( green > 255) 
		    green = 255;
		if ( blue > 255) 
		    blue = 255;

		if ( red < 0) 
		    red = 0;
		if ( green < 0) 
		    green = 0;
		if ( blue < 0) 
		    blue = 0;

		millislast = millis();
	}
}

void Solid()
{
	red = (int) redVal;
	green = (int) greenVal;
	blue = (int) blueVal;
}

void Strobe()
{
	if(millis()-millislast >= 3*flash)
	{
		red = 0;
		green = 0;
		blue = 0;
	}
	else
	{
		red = (int) redVal;
		green = (int) greenVal;
		blue = (int) blueVal;		
	}

	if(millis()-millislast >= 6*flash)
		millislast = millis();
}

void Pulse()
{
	red   = ( pow( sin( ((float) (millis()-millislast) / (flash * 50) ) * 3.14 ), 8) ) * (int) redVal;
	green = ( pow( sin( ((float) (millis()-millislast) / (flash * 50) ) * 3.14 ), 8) ) * (int) greenVal;
	blue  = ( pow( sin( ((float) (millis()-millislast) / (flash * 50) ) * 3.14 ), 8) ) * (int) blueVal;

	if(millis()-millislast >= (flash * 50))
		millislast = millis();
}

void Ambiance()
{

}

void Music(int offset)
{
	
	/********************************************************************
	 submode:
		1. Low Mode (Random)
		 	1. Color mus_Pulse red-+green, green-+blue, blue-+red  (10 sec)
		 	2. mus_Fade speed 1 (10 sec)
		 	4. Color mus_Skip to color on beat (5 sec)
		 	3. mus_Fade speed 2 (10 sec)
		2. High Mode
		 	1. Color mus_Pulse red-+green, green-+blue, blue-+red  (10 sec)
		 	2. mus_Fade speed 1 (15 sec)
		 	3. mus_Fade speed 2 (15 sec)
	*********************************************************************/

	// if(false) // time for a new submode
	// {
	// 	submode = rand() % 5 + 1;
	// 	if(submode == 1)
	// 		val = rand() % 6 + 1;
	// }

	// tone = getTone();
	// volume = getVolume(tone);

	// if(submode == 5) // volume == 0 for the last 2 seconds
	// {
	// 	red = green = blue = 0;
	// } 
	// else if(submode == 1) // submode == Color Pulse
	// {
	// 	// Multiple submodes randomly selected
	// 	// r -> g, r-> b, g -> r, g -> b, b -> r, b -> g 
	// 	//val is random seed for pulse (change this upon submode selection)
	// 	colorPulse(val);
	// }
	// else if(submode == 2) // submode == Color Pulse
	// {
	// 	flash = 20;
	// 	inten = volume;
	// 	Fade();
	// }
	// else if(submode == 3)
	// {
	// 	flash = 170;
	// 	inten = volume;
	// 	Fade();
	// }	
	// else if(submode == 4)
	// {
	// 	inten = 255;
	// 	colorBounce();
	// }
	
	int output = 0;

	// Sample Mic
	for( int i = 0; i < BL; i++ )
	{
		while( (micros()-lastx) < Ts ){;}
		data_IN[(BL-1)-i] = analogRead(micPin)-512;
		lastx = micros();
	}

	// Calculate next output w/ group delay of N/2 so Y[n+N/2]
	for( int i = 0; i < BL; i++ )
		output += data_IN[i] * LP[i];

	int c = abs(output);
	int b = abs(data_IN[0]);
	

	// calculate the running average and relative max for the last 
	// ave = ((ave*49)+b)/50; 
	
	if( c > maxAmp )
	{
		maxAmp = c;
		maxtimeout = millis();
	} else if( millis() - maxtimeout > 4000 )
		maxAmp = c+1;

	int led  = map(c, 0, maxAmp, 0, pps); //  Map the amplitude between 0 and the max brightness set by user

	if( c > 15 && (maxAmp-c) < (maxAmp/2) ) // this cuttoff needs to be > 70 for bass and > 10 for highs
	{
		if( led > inten )
			inten = led;
		else
			inten -= 10;
	}
	else
		if( inten - 10 > 0 )
			inten -= 10;
		else
			inten = 0;

	Fade();
}

void getTone()
{

}

void getVolume(int tone)
{

}

void mus_Fade()
{

}

void mus_Skip()
{

}

void mus_Pulse()
{

}