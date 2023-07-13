//#include <Adafruit_TinyUSB.h>
#include "StringSplitter.h"
#include "CentralBLE.h"
CentralBLE SerialBLE;

void setup(void)
{
  //Serial.begin(115200);
  //while (!Serial); yield();
  Serial1.begin(115200);
  SerialBLE.addPrphName("AirMeter");   //DEVICE_NUM = 0
  SerialBLE.addPrphName("PowerMeter"); //DEVICE_NUM = 1 //To add more than 2 peripherals, config MAX_PERIPHERAL definition in header file.
  SerialBLE.addPrphName("Display");
  SerialBLE.begin("CentralXIAO");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
}
#define AIRMETER 0
#define POWERMETER 1 //The numbers defined here are the order addPrphName executed. this will make it useful!
#define ROUNDDISPLAY 2

void loop()
{
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
  PACKET packet;

  delay(500);

  while (1) {
    if (SerialBLE.isOpen(AIRMETER)) {
      packet.AirMeterIsOpen = 1;
      digitalWrite(LED_RED, HIGH);
      String tmp = SerialBLE.readStringUntil(getPeripheralID(AIRMETER), ',');
      StringSplitter *AirData = new StringSplitter(tmp, ' ', 2);
      packet.AirSpeed = AirData->getItemAtIndex(0).toFloat();
      packet.AirMeterBat = AirData->getItemAtIndex(1).toFloat();
    }
    else
    {
      packet.AirMeterIsOpen = 0;
      digitalWrite(LED_RED, LOW);
    }
    if (SerialBLE.isOpen(POWERMETER)) {
      packet.PowerMeterIsOpen = 1;
      digitalWrite(LED_BLUE, HIGH);
      String tmp = SerialBLE.readStringUntil(getPeripheralID(POWERMETER), ',');
      StringSplitter *PowerData = new StringSplitter(tmp, ' ', 4);
      packet.Cadence = PowerData->getItemAtIndex(0).toInt();
      packet.PowerAvg = PowerData->getItemAtIndex(1).toInt();
      packet.PowerMax = PowerData->getItemAtIndex(2).toInt();
      packet.PowerMeterBat = PowerData->getItemAtIndex(3).toFloat();
    }
    else
    {
      packet.PowerMeterIsOpen = 0;
      digitalWrite(LED_BLUE, LOW);
    }
    if (Serial1.read() != -1)
    {
      Serial1.write(packet.bin, sizeof(PACKET));
    }

    if (SerialBLE.isOpen(ROUNDDISPLAY)) {
      String str = String(packet.Cadence);
      str += " ";
      str += String(packet.PowerAvg);
      str += " ";
      str += String(int(packet.PowerMeterBat * 100.0));
      str += " ";
      str += String(int(packet.AirSpeed * 100.0));
      str += " ";
      str += String(int(packet.AirMeterBat * 100.0));
      str += ",";
      SerialBLE.print(getPeripheralID(ROUNDDISPLAY), str.c_str());
    }
    delay(100);
  }
}
