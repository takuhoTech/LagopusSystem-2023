#include "XIAOBLE.h"
XIAOBLE SensorBLE;

void setup(void)
{
  Serial.begin(115200);
  while (!Serial);

  Serial1.setTX(8);
  Serial1.setRX(9);
  Serial1.begin(115200);
  SensorBLE.init(Serial1);
}

void loop()
{
  SensorBLE.update();
  Serial.print(SensorBLE.getCadence());
  delay(1000);
}
