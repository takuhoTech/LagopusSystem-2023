#include "XIAOBLE.h"
XIAOBLE SensorBLE;

void setup(void)
{
  Serial.begin(115200);
  while (!Serial);

  Serial2.setTX(8);
  Serial2.setRX(9);
  Serial2.begin(115200);
  while (!Serial2);
  SensorBLE.init(Serial2);
}

void loop()
{
  SensorBLE.update();
  Serial.print(SensorBLE.AirMeterIsConnected());
  Serial.print("  ");
  Serial.println(SensorBLE.getAirMeterBat());

  delay(1000);
}
