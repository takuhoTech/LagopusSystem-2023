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

  analogReference(AR_INTERNAL_2_4); // VREF = 2.4V
  analogReadResolution(10);         // 10bit A/D
}
//#define AIRMETER 0
//#define POWERMETER 1 //The numbers defined here are the order addPrphName executed. this will make it useful!

#define CADENCE_HEIGHT 10
#define POWER_HEIGHT 175
#define UNIT_OFFSET 51

#define POWER_MAX 300
#define POWER_TGT 240
#define POWER_MIN 180

#define CADENCE_MAX 100
#define CADENCE_TGT 90
#define CADENCE_MIN 80

void DrawGauge(int x, int y0, int y1, uint32_t color)
{
  tft.fillRect(0, min(y0, y1), x, abs(y0 - y1), color);
  tft.fillRect(240 - x, min(y0, y1), x, abs(y0 - y1), color);
}

void EraseGauge(int x, int y0, int y1, uint32_t color)
{
  tft.fillRect(x, min(y0, y1), 240 - (2 * x), abs(y0 - y1), color);
}

void loop()
{
  typedef struct
  {
    double current = 0;
    //double last = 0;
  } BAT;
  BAT AirMeterBat;
  BAT PowerMeterBat;
  BAT DisplayBat;

  typedef struct
  {
    double current = 0;
    //double last = 0;
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
  uint16_t BLUE = 0x1c7f;
  uint16_t PowerColor = 0x0000;
  uint16_t CadenceColor = 0x0000;
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
      //String msg = "Ready";
      //SerialBLE.write(msg.c_str());
      digitalWrite(LED_BLUE, HIGH);
      String tmp = SerialBLE.readStringUntil(',');
      StringSplitter *packet = new StringSplitter(tmp, ' ', 5);
      cadence.current = packet->getItemAtIndex(0).toInt();
      power.current = packet->getItemAtIndex(1).toInt();
      PowerMeterBat.current = packet->getItemAtIndex(2).toInt() / 100.0;
      AirSpeed.current = packet->getItemAtIndex(3).toInt() / 100.0;
      AirMeterBat.current = packet->getItemAtIndex(4).toInt() / 100.0;
      delete packet;
    }
    else
    {
      digitalWrite(LED_BLUE, LOW);
    }

    tft.setTextSize(8);
    if (cadence.last >= 100) {
      tft.setCursor(60, CADENCE_HEIGHT);
    }
    else if (cadence.last >= 10) {
      tft.setCursor(80, CADENCE_HEIGHT);
    }
    else
    {
      tft.setCursor(100, CADENCE_HEIGHT);
    }
    tft.setTextColor(BLACK);
    tft.print(cadence.last);
    if (cadence.current >= 100) {
      tft.setCursor(60, CADENCE_HEIGHT);
    }
    else if (cadence.current >= 10) {
      tft.setCursor(80, CADENCE_HEIGHT);
    }
    else
    {
      tft.setCursor(100, CADENCE_HEIGHT);
    }
    if (cadence.current >= CADENCE_MAX) {
      //tft.setTextColor(RED, BLACK, false);
      tft.setTextColor(RED);
      CadenceColor = RED;
    }
    else if (cadence.current < CADENCE_MIN) {
      //tft.setTextColor(BLUE, BLACK, false);
      tft.setTextColor(BLUE);
      CadenceColor = BLUE;
    }
    else {
      //tft.setTextColor(WHITE, BLACK, false);
      tft.setTextColor(WHITE);
      CadenceColor = WHITE;
    }
    tft.print(cadence.current);
    tft.setCursor(110, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("RPM");

    DisplayBat.current = (analogRead(A0) / 1023.0) * 2.4 * 2;

    tft.setCursor(174, CADENCE_HEIGHT + UNIT_OFFSET - 16);
    tft.setTextColor(WHITE, BLACK, true);
    tft.setTextSize(1);
    tft.print("Dis");
    tft.setCursor(174 + 18, CADENCE_HEIGHT + UNIT_OFFSET - 16);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(DisplayBat.current, 2);

    tft.setCursor(174, CADENCE_HEIGHT + UNIT_OFFSET - 8);
    tft.setTextColor(WHITE, BLACK, true);
    tft.setTextSize(1);
    tft.print("Air");
    tft.setCursor(174 + 18, CADENCE_HEIGHT + UNIT_OFFSET - 8);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(AirMeterBat.current, 2);
    //tft.setCursor(172 + 53, CADENCE_HEIGHT + UNIT_OFFSET-9);
    //tft.print("V");

    tft.setCursor(174, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextColor(WHITE, BLACK, true);
    tft.setTextSize(1);
    tft.print("Pwr");
    tft.setCursor(174 + 18, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(PowerMeterBat.current, 2);
    //tft.setCursor(172 + 53, CADENCE_HEIGHT + UNIT_OFFSET);
    //tft.print("V");

    tft.setCursor(65, CADENCE_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("MPS");
    tft.setCursor(17, 54);
    tft.setTextSize(2);
    tft.setTextColor(WHITE, BLACK, true);
    tft.print(AirSpeed.current, 2);


    tft.setTextSize(8);
    if (power.last >= 100) {
      tft.setCursor(60, POWER_HEIGHT);
    }
    else if (power.last >= 10) {
      tft.setCursor(80, POWER_HEIGHT);
    }
    else
    {
      tft.setCursor(100, POWER_HEIGHT);
    }
    tft.setTextColor(BLACK);
    tft.print(power.last);
    if (power.current >= 100) {
      tft.setCursor(60, POWER_HEIGHT);
    }
    else if (power.current >= 10) {
      tft.setCursor(80, POWER_HEIGHT);
    }
    else
    {
      tft.setCursor(100, POWER_HEIGHT);
    }
    if (power.current >= POWER_MAX) {
      //tft.setTextColor(RED, BLACK, true);
      tft.setTextColor(RED);
      PowerColor = RED;
    }
    else if (power.current < POWER_MIN) {
      //tft.setTextColor(BLUE, BLACK, true);
      tft.setTextColor(BLUE);
      PowerColor = BLUE;
    }
    else {
      //tft.setTextColor(WHITE, BLACK, true);
      tft.setTextColor(WHITE);
      PowerColor = WHITE;
    }
    tft.print(power.current);

    tft.setCursor(108, POWER_HEIGHT + UNIT_OFFSET);
    tft.setTextSize(1);
    tft.print("Watt");

    if (cadence.current < CADENCE_TGT)
    {
      EraseGauge(map(cadence.current, 0, CADENCE_TGT, 0, 120), 120 - 50, 120 - 1, BLACK);
      DrawGauge(map(cadence.current, 0, CADENCE_TGT, 0, 120), 120 - 50, 120 - 1, CadenceColor);
      //tft.fillRect(0, 120, 120, 20, BLUE);
    }
    else
    {
      DrawGauge(120, 120 - 50, 120 - 1, CadenceColor);
    }

    if (power.current < POWER_TGT)
    {
      EraseGauge(map(power.current, 0, POWER_TGT, 0, 120), 120 + 1, 120 + 50, BLACK);
      DrawGauge(map(power.current, 0, POWER_TGT, 0, 120), 120 + 1, 120 + 50, PowerColor);
      //tft.fillRect(0, 120, 120, 20, BLUE);
    }
    else
    {
      DrawGauge(120, 120 + 1, 120 + 50, PowerColor);
    }

    cadence.last = cadence.current;
    power.last = power.current;
    delay(200);
  }
}
