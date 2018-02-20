void pulse(double period)
{
	FastLED.setBrightness(pow(sin(((float)(millis()%((int)period))) * 3.14/period ), 8) * 255); 
}

void strob()
{

}

void solid()
{
    const char * r1 = state["Red"];
    const char * g1 = state["Green"];
    const char * b1 = state["Blue"];

    int Rv = (String(r1)).toInt();
    int Gv = (String(g1)).toInt();
    int Bv = (String(b1)).toInt();

    for (int i=0; i < N_LEDS; i++) 
    {
      leds[i].setRGB(Rv, Gv, Bv);
    }  
}

int huest(int HuePtr, int HueDelta)
{
    for (int i=0; i < (N_LEDS-1); i++) 
    {
      leds[i] = leds[i+1];
    }  
    int newhue = HuePtr + HueDelta;
    leds[N_LEDS-1].setHue(newhue);
    return newhue;
}

void ambia()
{

}

void bootup()
{

}


/*

+IPD,0,13:{"Red":"255"}

{
  "Properties": {
    "Dev-ID": "LEDNode-v01",
    "UID": "00000000",
    "Net-ID": "00000000",
    "NLEDs": true
  },
  "Network": {
    "SSID": "Glacian",
    "SSID-PW": "absolution",
    "LAN-IP": "127.0.0.1"
  },
  "State": {
    "Mode": "0",
    "Red": "0",
    "Green": "100",
    "Blue": "255",
    "Period": "00000000",
    "Intensity": "150"
  }
}

*/