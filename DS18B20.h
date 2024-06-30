#include "OneWire.h"
class DS18B20
{
  private:
      int   g_signalPin;
      byte  g_chipType;
      byte  g_address[8];
      OneWire g_oneWire;

  public:
    DS18B20(int signalPin);
    void init();
    int16_t getRawTemperature();
};
DS18B20::DS18B20(int signalPin)
{
  g_signalPin = signalPin;
  g_oneWire = OneWire(g_signalPin);
}
void DS18B20::init()
{
  g_chipType = 0;
  if ( !g_oneWire.search(g_address)) 
  {
    g_oneWire.reset_search();
    delay(250);
    g_chipType = 0;
    return;
  }
   
  // the first ROM byte indicates which chip
  switch (g_address[0]) 
  {
    case 0x10:
      g_chipType = 1;
      break;
    case 0x28:
      g_chipType = 0;
      break;
    case 0x22:
      g_chipType = 0;
      break;
    default:
      g_chipType = 0;
      return;
  } 
  return;
}
int16_t DS18B20::getRawTemperature()
{
  byte i;
  byte data[12];
  g_oneWire.reset();
  g_oneWire.select(g_address);
  g_oneWire.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(750);     // maybe 750ms is enough, maybe not
  
  g_oneWire.reset();
  g_oneWire.select(g_address);    
  g_oneWire.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) data[i] = g_oneWire.read();
  int16_t raw = (data[1] << 8) | data[0];
  if (g_chipType) 
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10)  raw = (raw & 0xFFF0) + 12 - data[6];
  }
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  return raw;
  
}
