#include "XIAOBLE.h"

void XIAOBLE::init(Stream &stream)
{
  _serial = &stream;
}

void XIAOBLE::update()
{
  _serial->write(1); //request
  unsigned long LastTime = millis();
  while ((_serial->available() < sizeof(PACKET)) && (millis() - LastTime < 500));
  for (uint8_t i = 0; i < sizeof(PACKET); ++i)
  {
    //Serial.write(pos.bin[i]);
    packet.bin[i] = _serial->read();
  }
  while (_serial->available() > 0)
  {
    _serial->read();
  }
}

bool XIAOBLE::AirMeterIsConnected()
{
  return packet.AirMeterIsOpen;
}
float XIAOBLE::getAirSpeed()
{
  return packet.AirSpeed;
}
float XIAOBLE::getAirMeterBat()
{
  return packet.AirMeterBat;
}
bool XIAOBLE::PowerMeterIsConnected()
{
  return packet.PowerMeterIsOpen;
}
uint16_t XIAOBLE::getCadence()
{
  return packet.Cadence;
}
uint16_t XIAOBLE::getPowerAvg()
{
  return packet.PowerAvg;
}
uint16_t XIAOBLE::getPowerMax()
{
  return packet.PowerMax;
}
float XIAOBLE::getPowerMeterBat()
{
  return packet.PowerMeterBat;
}
