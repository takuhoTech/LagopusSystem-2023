#include "PeripheralBLE.h"
#include <Adafruit_TinyUSB.h>
#include "Wire.h"
#include <SparkFun_FS3000_Arduino_Library.h>

FS3000 flow;
PeripheralBLE SerialBLE;

#define PIN_WAKE  6
#define LED_EXT 7
#define PIN_HICHG 22 //D22 = P0.13 (BQ25100 ISET)

void setup()
{
  //Serial.begin(9600); while (!Serial) yield();
  pinMode(PIN_HICHG, OUTPUT);
  digitalWrite(PIN_HICHG, LOW); //High Charging Current : 100mA

  analogReference(AR_INTERNAL_2_4); // VREF = 2.4V
  analogReadResolution(10);         // 10bit A/D
  pinMode(VBAT_ENABLE, OUTPUT);
  digitalWrite(VBAT_ENABLE, LOW);

  SerialBLE.begin("AirMeter"); // Sleep functions need the softdevice to be active.
  pinMode(PIN_WAKE,  INPUT_PULLUP_SENSE);    // this pin (PIN_WAKE) is pulled up and wakes up the feather when externally connected to ground.
  pinMode(LED_EXT, OUTPUT);
  digitalWrite(LED_EXT, HIGH);

  Wire.begin();
  while (flow.begin() == false);
  flow.setRange(AIRFLOW_RANGE_15_MPS);

  //analogReadResolution(12); //0 - 4095
}

void loop()
{
  double BATvoltage = 0;

  typedef struct
  {
    double raw = 0;
    double sum = 0;
    double avg = 0;
  } Flow;
  Flow AirSpeed;

  typedef struct
  {
    unsigned long last = 0;
    unsigned long current = 0;
    unsigned long delta = 0;
    unsigned long count = 0;
  } Time;
  Time time;

  while (true)
  {
    do {
      if (digitalRead(PIN_WAKE) == LOW) {
        digitalWrite(LED_EXT, HIGH);
        delay(500);
        digitalWrite(LED_EXT, LOW);
        delay(250);
        sd_power_system_off();
      }

      AirSpeed.raw = flow.readMetersPerSecond();
      AirSpeed.sum += AirSpeed.raw;

      time.current = millis();
      time.delta = time.current - time.last;
      time.count++;

      if (SerialBLE.isOpen())
      {
        digitalWrite(LED_EXT, LOW);
      }
      else
      {
        digitalWrite(LED_EXT, HIGH);
      }
      delay(10);
    } while (time.delta < 1000);

    AirSpeed.avg = AirSpeed.sum / time.count;
    AirSpeed.sum = 0;
    time.count = 0;
    time.last = time.current;

    BATvoltage = (2.4 / 1023.0) * (1510.0 / 510.0) * analogRead(PIN_VBAT);

    String str = String(AirSpeed.avg, 2);
    str += "MPS ";
    str += String(BATvoltage, 2);
    str += "V,";

    SerialBLE.write(str.c_str());
    //Serial.println(AirSpeed.avg);
  }
}
