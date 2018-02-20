#include <VirtualWire.h>
#include <LowPower.h>

#define REPEAT 3
#define WAIT 700
#define THRESHOLD 10

byte buffer[2] = { 0xA4, 0x01 };
int txPin = 6,
	sensePin = A0,
	LED1 = 12,
	LED2 = 13;

unsigned long num = 0,
			  ave = 0;

void setup() 
{
    Serial.begin(9600);
    vw_set_tx_pin(txPin);
    vw_setup(4800);       // Bits per sec

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
	pinMode(sensePin, INPUT);
	
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);

    ave = analogRead( sensePin );
}

void loop() 
{
	int maxval = 0;
	pinMode(sensePin, INPUT);	

	for( int i = 0; i < 5000; i++ )
	{                
		int tmp = analogRead(sensePin);
		if( tmp > maxval )
			maxval = tmp;         
	}
	if( ( maxval - ave ) >= THRESHOLD && maxval >= 10 )
	{
		digitalWrite(LED2, HIGH);
		broadcast(REPEAT);
		pinMode(sensePin, OUTPUT);
		digitalWrite(sensePin, LOW);
		digitalWrite(LED2, LOW);  
	} 

	ave = ( ave * 5 + maxval ) / 6;

	LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF); 
}

	// Helper Functions \\ 

void broadcast( int i )
{
	for( int j = 0; j < i; j++ )
	{
		vw_send( ( uint8_t * ) buffer, 2 );
		vw_wait_tx(); 
	}
}