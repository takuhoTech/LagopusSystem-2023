#include "PeripheralBLE.h"
#include <Adafruit_TinyUSB.h>
#include "LSM6DS3.h"
#include "Wire.h"

PeripheralBLE SerialBLE;

#define PIN_WAKE  7
#define PIN_POWER A5
#define PIN_BAT A0

#define PIN_HICHG 22 //D22 = P0.13 (BQ25100 ISET)

LSM6DS3 myIMU(I2C_MODE, 0x6A); //I2C device address 0x6A

void setup()
{
  //Serial.begin(9600); while (!Serial) yield();
  pinMode(PIN_HICHG, OUTPUT);
  digitalWrite(PIN_HICHG, LOW); //High Charging Current : 100mA

  SerialBLE.begin("PowerMeter"); // Sleep functions need the softdevice to be active.
  pinMode(PIN_WAKE,  INPUT_PULLUP_SENSE);    // this pin (PIN_WAKE) is pulled up and wakes up the feather when externally connected to ground.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);

  analogReadResolution(12); //0 - 4095

  while (myIMU.begin() != 0);
}

void loop()
{
  double BATvoltage = 0;
  double angular_velocity_degree = 0, angular_velocity_radian = 0, degree = 0;
  typedef struct
  {
    unsigned long last = 0;
    unsigned long current = 0;
    unsigned long delta = 0;
    unsigned long count = 0;
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
    const double SLOPE = 5.56209 * 4.0;
    const int OFFSET = 77 * 4;
    int strain = 0;
    int raw = 0;
    long sum = 0;
    int avg = 0;
    int max = 0;
  } Power;
  Power power;

  unsigned long LastTime;

  while (true)
  {
    power.max = 0;
    LastTime = millis();
    do {
      if (digitalRead(PIN_WAKE) == LOW) {
        digitalWrite(LED_BLUE, HIGH);
        delay(250);
        digitalWrite(LED_RED, LOW);
        delay(500);
        digitalWrite(LED_RED, HIGH);
        delay(250);
        sd_power_system_off();
      }
      angular_velocity_degree = myIMU.readFloatGyroZ(); //単位は度
      angular_velocity_radian = angular_velocity_degree * (PI / 180.0);
      cadence.raw = angular_velocity_radian * (30.0 / PI);
      cadence.sum += cadence.raw;

      time.current = micros();
      time.delta = time.current - time.last;
      time.last = time.current;
      degree += angular_velocity_degree * (time.delta / 1000000.0);
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

      if (power.raw > power.max)
      {
        power.max = power.raw;
      }

      if (SerialBLE.isOpen())
      {
        digitalWrite(LED_BLUE, HIGH);
      }
      else
      {
        digitalWrite(LED_BLUE, LOW);
      }
      //Serial.print(power.raw); Serial.print(" "); Serial.print(cadence.raw); Serial.print(" "); Serial.println(degree);
      delay(10);
    } while ((degree < 360) && (millis() - LastTime < 8000));
    degree = 0;
    cadence.avg = cadence.sum / time.count;
    power.avg = power.sum / time.count;
    cadence.sum = 0;
    power.sum = 0;
    time.count = 0;

    power.avg *= 2;
    power.max *= 2;

    BATvoltage = (3.6 / 4095.0) * (1510.0 / 510.0) * analogRead(PIN_BAT);

    /*String str = "RPM:";
      str += String(cadence.avg);
      str += " PWR:";
      str += String(power.avg);
      str += " BAT:";
      str += String(BATvoltage, 2);*/

    String str = String(cadence.avg);
    str += "RPM ";
    str += String(power.avg);
    str += "W ";
    str += String(power.max);
    str += "W ";
    str += String(BATvoltage, 2);
    str += "V,";

    SerialBLE.write(str.c_str());
    //Serial.println(str);
  }
}
