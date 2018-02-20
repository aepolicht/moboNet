#include <VirtualWire.h>
#include <EEPROM.h>
#include <FastSPI_LED2.h>

static const uint8_t magic[] = {'A','e','P'};

#define UID 0x11

#define MAGICSIZE  sizeof(magic)
#define HEADERSIZE (MAGICSIZE + 3)
#define MODE_HEADER 0
#define MODE_HOLD   1
#define MODE_DATA   2
#define N_LEDS      25

#define UID 0x02
#define LEDHEADER 0x90
#define SOLID     0x01
#define STROBE    0x02
#define PULSE     0x03
#define FADE      0x04
#define AMBIANCE  0x05
#define MUSIC     0x06

uint8_t
  buffer[256],
  indexIn       = 0,
  indexOut      = 0,
  mode          = MODE_HEADER,
  hi, lo, chk, i, spiFlag;
int16_t
  bytesBuffered = 0,
  hold          = 0,
  c;
int32_t
  bytesRemaining;
unsigned long
  startTime;

byte modCode, redVal, greenVal, blueVal, speedVal, submode, groupNum, inten, pps;

// Pins
int rxpin    = 7,
    ledPin   = 8,
    dataPin  = 11,    // Green
    clockPin = 13,    // Yellow 
    micPin   = 0;

// Values
int red   = 255, 
    green = 0, 
    blue  = 0, 
    flash = 100,
    notif = 0;

// Time keeping
unsigned long millislast = 0,
              PulseMillis = 0,
              notiflast  = 0;

bool rfFlag = true;

CRGB leds[N_LEDS];

void setup()
{
  LEDS.setBrightness(255);
  LEDS.addLeds<WS2801>(leds, N_LEDS);

  Serial.begin(115200);
 
  pinMode(ledPin,       OUTPUT);
  digitalWrite(ledPin, LOW);

  vw_set_rx_pin(rxpin);
  vw_setup(4800);  // Bits per sec
  vw_rx_start();       // Start the receiver PLL running

  groupNum = EEPROM.read(0);
   modCode = EEPROM.read(1);
    redVal = EEPROM.read(2);
  greenVal = EEPROM.read(3);
   blueVal = EEPROM.read(4);
     inten = (byte) 190;
     flash = EEPROM.read(6);
}

void loop()
{
  uint8_t values[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  while(Serial.available() >= 1)
  {
    buffer[indexIn++] = Serial.read();
    bytesBuffered++;
  }

  if(bytesBuffered >= 81) 
  {
    if( buffer[0] == 'A' &&
        (buffer[1] == 'e' || buffer[1] == 'd') &&
        (buffer[2] == 'P' || buffer[2] == 'a') &&
        buffer[3] == byte((N_LEDS - 1) >> 8) &&
        buffer[4] == byte((N_LEDS - 1) & 0xff) &&
        buffer[5] == byte(buffer[3] ^ buffer[4] ^ 0x55)
      ) 
    {
      rfFlag = false;

      for (i=0; i < 75; i+=3) 
        leds[i/3] = CRGB(buffer[i+6], buffer[i+7], buffer[i+8]);
      LEDS.show();
    }
    bytesBuffered = 0;
    indexIn = 0;
  }

  if(vw_get_message(values, &buflen))
  {
    digitalWrite(ledPin, HIGH);

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

      inten = pps;
    }

    digitalWrite(ledPin, LOW);

    rfFlag = true;
  }

  if(rfFlag)
  {
    switch(modCode) // Check Modcode
    {
      case SOLID:    Solid();    break;
      case STROBE:   Strobe();   break;
      case PULSE:    Pulse();    break;
      case FADE:     Fade();     break;
      case MUSIC:    Music(500); break;
      case AMBIANCE: Notify();   break;
      default:       Solid();
    } 

    for (i=0; i < N_LEDS; i++) 
    {
      leds[i].r = red*inten/255; 
      leds[i].g = green*inten/255;
      leds[i].b = blue*inten/255;
    }    
    LEDS.show();    
    delayMicroseconds(300);
  }
}

void Fade()
{
  if(millis()-millislast >= (flash/10))
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

void Notify()
{
  if( millis()-notiflast >= 3000 )
  {
    switch(notif)
    {
      case 1: red = 0;   green = 255;   blue = 0;   break;
      case 2: red = 255; green = 0;   blue = 0;     break;
      case 3: red = 0;   green = 0;     blue = 255; break;
      case 4: red = 255; green = 255; blue = 0;     break;
      case 5: red = 0;   green = 255;   blue = 255; break;
      case 6: red = 255; green = 0;   blue = 255;   break;
    }
    flash = 175;
    Pulse();
  }
  else
  {
    groupNum = EEPROM.read(0);
     modCode = EEPROM.read(1);
      redVal = EEPROM.read(2);
    greenVal = EEPROM.read(3);
     blueVal = EEPROM.read(4);
       inten = EEPROM.read(5);
       flash = EEPROM.read(6);
  } 
}

void Pulse()
{
  red   = ( pow( sin( ((float) (millis()-PulseMillis) / (flash * 50) ) * 3.14 ), 8) ) * (int) redVal;
  green = ( pow( sin( ((float) (millis()-PulseMillis) / (flash * 50) ) * 3.14 ), 8) ) * (int) greenVal;
  blue  = ( pow( sin( ((float) (millis()-PulseMillis) / (flash * 50) ) * 3.14 ), 8) ) * (int) blueVal;

  if(millis()-PulseMillis >= (flash * 50))
    PulseMillis = millis();
}

void Ambiance()
{
}

void Music(int offset)
{
  Fade();
  inten = sin( ((float) (millis()-PulseMillis) / (flash * 50) ) * 3.14 ) * (int) 255;
  if(millis()-PulseMillis >= (flash * 50))
    PulseMillis = millis();
}

// void colorWipe(uint32_t c, uint8_t wait) 
// {
//   int i;
 
//   for (i=0; i < strip.numPixels(); i++) {
//       strip.setPixelColor(i, c);
//       strip.show();
//       delay(wait);
//   }
// }
 
/* Helper functions */
// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}