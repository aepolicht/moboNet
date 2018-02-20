#define DEBUG

#include <ArduinoJson.h>
#include <FastLED.h>
#ifdef DEBUG
#include <SoftwareSerial.h>
#endif
// #include "LEDFunc.ino"

/*
LED NODE JSON Structure
{  
    "Properties": 
    {  
      "Dev-ID": "LEDNode - v01",  
      "UID":    00000001,  
      "Net-ID": 00000000,
      "NLEDs":25
    }, 
    "Network": 
    {  
      "SSID": "",
      "SSID-PW": "",
      "LAN-IP": "127.0.0.1"
    }, 
    "State": 
    {
      "Mode": 1,  
      "Red": 00000000,  
      "Green": 00000000,`
      "Blue": 00000000,  
      "Period": 00000000,  
      "Intensity": 00000000      
    } 
}  
*/

#define DEFAULT_TIMEOUT_PERIOD 2000
#define WIFI_RESET_PIN 10
#define LED_PIN 13
#define LED_COM_PIN 5
#define LED_CLK_PIN 6
#define N_LEDS 32
#define LED_FRAME_RATE 500

// LedStates
#define LS_SOLID  0
#define LS_PULSE  1
#define LS_STROB  2
#define LS_AMBIA  3
#define LS_HUEST  4

uint8_t LED_STATE = 4;
int HuePtr = 0;

uint32_t LastTramsmit = 0;
uint32_t LastFrame = 0;

uint8_t STREAMINGOVERRIDE = 1;

CRGB leds[N_LEDS];

StaticJsonBuffer<150> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();
JsonObject& properties = root.createNestedObject("Properties");
JsonObject& state = root.createNestedObject("State");

String received = ""; 
char inBuffer[100];
int bytesinbuf = 0;

void parseUART()
{
  boolean message_Processed = false;

  while(Serial.available() > 0)
  {
    FastLED.delay(10);

    int numbytes = Serial.available();

    // Serial.println(numbytes, DEC);

    bytesinbuf += numbytes;

    Serial.readBytes(inBuffer, numbytes);
  }

  if(bytesinbuf > 0)
  {
    received = String(inBuffer);

    for(int i=0; i < 100; i++)
    {
      inBuffer[i] = NULL;          
    }

    bytesinbuf = 0;
    // Serial.println(received);
  }



  // while(Serial.available() > 0 && !message_Processed) 
  // { 
    // received.concat(char(Serial.read())); 
    // Serial.println("=======INCOMING=======");

 // Example Message: 
 // "+IPD,0,12:{"mode","1"}"

  if(received.indexOf("+IPD") > -1)
  {
    // #ifdef DEBUG
    Serial.println("=======INCOMING=======");
    Serial.println(received);
    // #endif

    // int start = millis();
    // FastLED.delay(50);

    int IPDStart = received.indexOf("+IPD");
    int CIDX1 = received.indexOf(',', IPDStart);
    int CIDX2 = received.indexOf(',', CIDX1 + 1);
    int CIDX3 = received.indexOf(':', CIDX2 + 1);

    // #ifdef DEBUG
    Serial.print("IDX1 = " + String(CIDX1));
    Serial.print(" - IDX2 = " + String(CIDX2));
    Serial.println(" - IDX3 = " + String(CIDX3));
    // // #endif

    int messageChannel = (received.substring(CIDX1 + 1, CIDX2)).toInt();
    int messageLength = received.substring(CIDX2 + 1, CIDX3).toInt();

    while(received.length() < messageLength)
    {
      if(Serial.available() > 0)
      {
        int numbytes = Serial.available();

        Serial.println(numbytes, DEC);

        bytesinbuf += numbytes;

        Serial.readBytes(inBuffer, numbytes);
      }

      if(bytesinbuf > 0)
      {
        received.concat(inBuffer);

        for(int i=0; i < 100; i++)
        {
          inBuffer[i] = NULL;          
        }

        bytesinbuf = 0;
        Serial.println(received);
      }      
    }

    String data = received.substring(CIDX3 + 1, CIDX3 + 1 + messageLength);



    // // String response = "Sup?";
    // // send("AT+CIPSEND=" + String(messageChannel) + "," + String(response.length()), DEFAULT_TIMEOUT_PERIOD, ">");
    // // send(response, DEFAULT_TIMEOUT_PERIOD, "OK");
    // // send("AT+CIPCLOSE", 0, "OK");
    
    // // #ifdef DEBUG
    Serial.println("messageChannel: " + String(messageChannel));
    Serial.println("messageLength: " + String(messageLength));
    Serial.println("data: " + String(data));
    // // #endif

    // message_Processed = true;

    int datalength = data.length()+1;
    char char_data[datalength];

    for(int i=0; i<datalength; i++)
    {
      char_data[i] = ' ';
    }

    data.toCharArray(char_data, datalength);
    // Parse incoming JSON object
    JsonObject& inroot = jsonBuffer.parseObject(char_data);

    for(int i=0; i<datalength; i++)
    {
        Serial.print(char_data[i]);
    }
    Serial.println("");

    if(inroot.success())
    {
      for (JsonObject::iterator it=inroot.begin(); it!=inroot.end(); ++it)
      {
        if(state.containsKey(it->key))
        {
          state[it->key] = it->value;
        }
        // #ifdef DEBUG
        Serial.println(it->key);
        Serial.println(String(it->value.asString()));
        // #endif
      }
      // #ifdef DEBUG
      root.prettyPrintTo(Serial);
      // #endif
    }

    // #ifdef DEBUG
    Serial.println("=======EOM=======");
    // #endif

    received = "";
  } 

  // FastLED.delay(1);
  // }
}

void setup() 
{
  // Initialize Serial information
  Serial.begin(9600);

  Serial.println("Hello.");

  // send("AT+CIOBAUD?", DEFAULT_TIMEOUT_PERIOD, "OK");
  // send("AT+CIOBAUD=115200", DEFAULT_TIMEOUT_PERIOD, "OK");

  // Setup pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(WIFI_RESET_PIN, OUTPUT);


  /*********************************/
  /*          JSON SETUP           */
  /*********************************/
  properties["Dev-ID"] = "LEDNode-v01";
  properties["UID"] = "0";
  properties["Net-ID"] = "0";
  properties["NLEDs"] = String(N_LEDS);

  state["Mode"] = "0";  
  state["Red"] = "0";  
  state["Green"] = "100";
  state["Blue"] = "255";  
  state["Period"] = "10000";  
  state["Intensity"] = "255";   
  state["huewidth"] = "175";  
  // #ifdef DEBUG
  root.prettyPrintTo(Serial);
  // #endif

  /*********************************/
  /*         Setup LED array       */
  /*********************************/
  // FastLED.addLeds<WS2812B, LED_COM_PIN, GRB>(leds, N_LEDS); 
  FastLED.addLeds<WS2801, LED_COM_PIN, LED_CLK_PIN, RGB>(leds, N_LEDS);

  // connectAP(ss, spw);

  // while(!send("AT+CIPSERVER=1,8080", DEFAULT_TIMEOUT_PERIOD, "OK"))
  // {
  //   reset();
  //   connectAP(ss, spw);
  //   FastLED.delay(50);
  // }
  // send("AT+CIFSR", DEFAULT_TIMEOUT_PERIOD, "OK");
}

uint8_t leapframes = 0;

void loop() 
{
  parseUART();
  
  const char * i1 = state["Intensity"];
  int Iv = (String(i1)).toInt();

  FastLED.setBrightness(Iv);

  if((millis() - LastFrame) >= 1000.0/LED_FRAME_RATE)
  {
    switch(LED_STATE)
    {
        case LS_SOLID:
        {
            solid();
            break;
        }
        case LS_PULSE:
        {
            const char * strperiod = state["Period"];
            double period = atof(strperiod);

            pulse(period);
            solid();
            break;
        }
        case LS_STROB:
        {
            strob();
            break;
        }
        case LS_AMBIA:
        {
            ambia();
            break;
        }
        case LS_HUEST:
        {
            const char * strperiod = state["Period"];
            double period = atof(strperiod);
            const char * strhuewidth = state["huewidth"];
            double huewidth = atof(strhuewidth);

            // assuming 8-bit precision (256) and period in ms
            double deltperframe = (huewidth/(((period)/1000.0)*(double)LED_FRAME_RATE)); 

            if(deltperframe < 1)
            {
                if (leapframes >= 1/deltperframe)
                {
                    HuePtr = huest(HuePtr, 1);
                    leapframes = 0;
                }
                else
                {
                    leapframes++;
                }
            }
            else
            {
                HuePtr = huest(HuePtr, (uint8_t) deltperframe);
            }
        }
            break;
        default:
        {
            solid();
            break;
        }
    }
    FastLED.show(); 
    LastFrame = millis();
  }
}