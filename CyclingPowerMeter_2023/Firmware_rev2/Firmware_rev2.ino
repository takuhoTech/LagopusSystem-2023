#include "LSM6DS3.h"
#include "Wire.h"

#define PIN_POWER A5

LSM6DS3 myIMU(I2C_MODE, 0x6A); //I2C device address 0x6A

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  while (myIMU.begin() != 0);
}

void loop()
{
  double angular_velocity_degree = 0, angular_velocity_radian = 0, degree = 0;
  typedef struct
  {
    double last = 0;
    double current = 0;
    double delta = 0;
    long count = 0;
  } Time;
  Time time;
  typedef struct
  {
    double raw = 0;
    long sum = 0;
    int avg = 0;
  } Cadence;
  Cadence cadence;
  typedef struct
  {
    const double SLOPE = 5.56209;
    const int OFFSET = 77;
    int strain = 0;
    int raw = 0;
    long sum = 0;
    int avg = 0;
  } Power;
  Power power;

  while (true)
  {
    do {
      angular_velocity_degree = myIMU.readFloatGyroZ(); //単位は度
      angular_velocity_radian = angular_velocity_degree * (PI / 180.0);
      cadence.raw = angular_velocity_radian * (30.0 / PI);
      cadence.sum += cadence.raw;

      time.current = micros();
      time.delta = time.current - time.last;
      time.last = time.current;
      degree += angular_velocity_degree * (time.delta / 1000000);
      time.count++;

      power.strain = analogRead(PIN_POWER) - power.OFFSET;
      if (power.strain < 0) {
        power.strain = 0;
      }
      power.raw = (power.strain / power.SLOPE) * angular_velocity_radian;
      if (power.raw < 0) {
        power.raw = 0;
      }
      power.sum += power.raw;

      //Serial.print(power.raw); Serial.print("  "); Serial.print(cadence.raw); Serial.print("  "); Serial.println(degree);
      delay(10);
    } while (degree < 360);
    degree = 0;
    cadence.avg = cadence.sum / time.count;
    power.avg = power.sum / time.count;
    cadence.sum = 0;
    power.sum = 0;
    time.count = 0;

    //Serial.println(cadence.avg);
  }
}
