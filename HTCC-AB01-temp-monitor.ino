bool chitchat = false;
#define SLEEPTIME 300000
#include "Arduino.h"
#include "DS18B20.h"
#include "LoRaWanMinimal_APP.h"

DS18B20 dS18B20(GPIO5);
union CubeData
{
  struct
  {
      uint16_t rawVoltage;
      int16_t rawTemp;
  };
  uint8_t buffer[4];
};
CubeData cubedata;

// blmc0002
static uint8_t devEui[] = { 0x62, 0x6C, 0x6D, 0x63, 0x30, 0x30, 0x30, 0x32 };
static uint8_t appEui[] = { 0x55, 0x68, 0x2c, 0x17, 0x1c, 0x56, 0xd1, 0xd0 };
static uint8_t appKey[] = { 0xa0, 0x13, 0xda, 0x08, 0x79, 0x57, 0x48, 0xd8, 0x49, 0xdf, 0xf5, 0x85, 0x54, 0xbd, 0x30, 0xbd};

uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

static uint8_t counter=0;
static uint16_t adcLevel;

///////////////////////////////////////////////////
//Some utilities for going into low power mode
TimerEvent_t sleepTimer;
//Records whether our sleep/low power timer expired
bool sleepTimerExpired;

static void wakeUp()
{
  sleepTimerExpired=true;
}

static void lowPowerSleep(uint32_t sleeptime)
{
  sleepTimerExpired=false;
  TimerInit( &sleepTimer, &wakeUp );
  TimerSetValue( &sleepTimer, sleeptime );
  TimerStart( &sleepTimer );
  //Low power handler also gets interrupted by other timers
  //So wait until our timer had expired
  while (!sleepTimerExpired) lowPowerHandler();
  TimerStop( &sleepTimer );
}

void setup() 
{
  if (chitchat) Serial.begin(115200);
  dS18B20.init();
  LoRaWAN.begin(LORAWAN_CLASS, ACTIVE_REGION);
  
  //Enable ADR
  LoRaWAN.setAdaptiveDR(true);

  while (1) 
  {
    if (chitchat) Serial.print("Joining... ");
    LoRaWAN.joinOTAA(appEui, appKey, devEui);
    if (!LoRaWAN.isJoined()) 
    {
      //In this example we just loop until we're joined, but you could
      //also go and start doing other things and try again later
      if (chitchat) Serial.println("JOIN FAILED! Sleeping for 30 seconds");
      lowPowerSleep(30000);
    } else 
    {
      if (chitchat) Serial.println("JOINED");
      break;
    }
  }
}

void loop() 
{
  counter++; 

  cubedata.rawVoltage = getBatteryVoltage();
  cubedata.rawTemp = dS18B20.getRawTemperature();
  float temp = (float) cubedata.rawTemp / 16.0;
  if (chitchat) 
  {
    Serial.print("Raw voltage = ");
    Serial.println(cubedata.rawVoltage);
    Serial.print("Temp        = ");
    Serial.println(temp);
    Serial.print("Raw Temp    = ");
    Serial.println(cubedata.rawTemp);
    Serial.printf("\nSending packet with counter=%d\n", counter);  
  }
  bool requestack=counter<5?true:false;
  if (LoRaWAN.send(4, (uint8_t*) cubedata.buffer, 2, requestack)) 
  {
    if (chitchat)Serial.println("Send OK");
  } 
  else 
  {
    if (chitchat)Serial.println("Send FAILED");
  }
  delay(1000);
  lowPowerSleep(SLEEPTIME); 
  
}
