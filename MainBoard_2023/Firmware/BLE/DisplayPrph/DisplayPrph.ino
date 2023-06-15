#include <Adafruit_TinyUSB.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include "NotoSansBold15.h"
#include <Adafruit_GFX.h>
#include "StringSplitter.h"
//#include "CentralBLE.h"
//CentralBLE SerialBLE;
#include "PeripheralBLE.h"
PeripheralBLE SerialBLE;

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
TFT_eSprite spr = TFT_eSprite(&tft); // Sprite
#define TFT_GREY 0xBDF7
#define panel_height 240
#define panel_length 240
#define Radius 120
#define AA_FONT_SMALL NotoSansBold15
#define FSS9 &FreeSans9pt7b
#define FMO9 &FreeMono9pt7b
#define FSSB9 &FreeSansBold9pt7b

void setup(void)
{
  //Serial.begin(115200);
  //while (!Serial); yield();
  SerialBLE.begin("Display");
  //pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  //digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
}
//#define AIRMETER 0
//#define POWERMETER 1 //The numbers defined here are the order addPrphName executed. this will make it useful!

#define CADENCE_HEIGHT 10
#define POWER_HEIGHT 175
#define UNIT_OFFSET 51

void loop()
{
  typedef struct
  {
    double current = 0;
    double last = 0;
  } BAT;
  BAT AirMeterBat;
  BAT PowerMeterBat;

  typedef struct
  {
    double current = 0;
    double last = 0;
  } Flow;
  Flow AirSpeed;

  typedef struct
  {
    int current = 0;
    int last = 0;
  } Cadence;
  Cadence cadence;
  typedef struct
  {
    int current = 0;
    int last = 0;
  } Power;
  Power power;

  uint16_t WHITE = 0xFFFF;
  uint16_t BLACK = 0x0000;
  uint16_t RED = 0xF800;
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(0x000000);

  //tft.drawLine(45, 120, 195, 120, WHITE);
  tft.drawLine(0, 120, 240, 120, WHITE);
  /*tft.setCursor(45, 104);
    tft.setTextColor(WHITE,BLACK,true);
    tft.setTextSize(2, 2, 0);
    tft.print("90");*/

  /*tft.setCursor(92, 40 + 47);
    tft.setTextSize(3);
    tft.print("RPM");
    tft.setCursor(110, 135 + 63);
    tft.print("W");*/

  delay(100);

  while (1) {
    if (SerialBLE.isOpen()) {
      digitalWrite(LED_BLUE, HIGH);
      String tmp = SerialBLE.readStringUntil(',');
      StringSplitter *packet = new StringSplitter(tmp, ' ', 5);
      cadence.current = packet->getItemAtIndex(0).toInt();
      power.current = packet->getItemAtIndex(1).toInt();
      PowerMeterBat.current = packet->getItemAtIndex(2).toInt() / 100.0;
      AirSpeed.current = packet->getItemAtIndex(3).toInt() / 100.0;
      AirMeterBat.current = packet->getItemAtIndex(4).toInt() / 100.0;
    }
    else
    {
      digitalWrite(LED_BLUE, LOW);
    }
    tft.setCursor(174, CADENCE_HEIGHT + UNIT_OFFSET - 9);
    tft.setTextColor(WHITE, BLACK, true);
    tft.setTextSize(1);
    tft.print("Air:");
    tft.setCursor(174 + 23, CADENCE_HEIGHT + UNIT_OFFSET - 9);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(AirMeterBat.current, 2);
    //tft.setCursor(172 + 53, CADENCE_HEIGHT + UNIT_OFFSET-9);
    //tft.print("V");

    tft.setCursor(174, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextColor(WHITE, BLACK, true);
    tft.setTextSize(1);
    tft.print("Pwr:");
    tft.setCursor(174 + 23, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(PowerMeterBat.current, 2);
    //tft.setCursor(172 + 53, CADENCE_HEIGHT + UNIT_OFFSET);
    //tft.print("V");

    tft.setCursor(17, 54);
    tft.setTextSize(2);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(AirSpeed.current, 2);
    tft.setCursor(65, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("MPS");

    tft.setTextSize(8);
    if (cadence.current >= 100) {
      tft.setCursor(50, CADENCE_HEIGHT);
    }
    else if (cadence.current >= 10) {
      tft.setCursor(74, CADENCE_HEIGHT);
    }
    else
    {
      tft.setCursor(100, CADENCE_HEIGHT);
    }
    if (cadence.current < 100) {
      tft.setTextColor(WHITE, BLACK, true);
    }
    else {
      tft.setTextColor(RED, BLACK, true);
    }
    tft.print(cadence.current);

    tft.setCursor(109, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("RPM");

    tft.setTextSize(8);
    if (power.current >= 100) {
      tft.setCursor(50, POWER_HEIGHT);
    }
    else if (power.current >= 10) {
      tft.setCursor(74, POWER_HEIGHT);
    }
    else
    {
      tft.setCursor(100, POWER_HEIGHT);
    }
    if (power.current < 300) {
      tft.setTextColor(WHITE, BLACK, true);
    }
    else {
      tft.setTextColor(RED, BLACK, true);
    }
    tft.print(power.current);

    tft.setCursor(115, POWER_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("W");

    delay(100);
  }
}
