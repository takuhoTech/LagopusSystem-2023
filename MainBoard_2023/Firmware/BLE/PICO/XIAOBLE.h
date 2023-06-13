#ifndef XIAOBLE_h
#define XIAOBLE_h
#include "Arduino.h"

class XIAOBLE
{
  public:
    void init(Stream &stream);
    void update();
    bool AirMeterIsConnected();
    float getAirSpeed();
    float getAirMeterBat();
    bool PowerMeterIsConnected();
    uint16_t getCadence();
    uint16_t getPowerAvg();
    uint16_t getPowerMax();
    float getPowerMeterBat();
  private:
    Stream* _serial;
};
union PACKET
{
  struct {
    bool AirMeterIsOpen = 0;  //1byte
    float AirSpeed;      //4byte
    float AirMeterBat;   //4byte
    bool PowerMeterIsOpen = 0; //1byte
    uint16_t Cadence;    //2byte
    uint16_t PowerAvg;   //2byte
    uint16_t PowerMax;   //2byte
    float PowerMeterBat; //4byte
  };
  uint8_t bin[20];
};
static PACKET packet;

#endif
