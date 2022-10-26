#include "DFRobotDFPlayerMini.h"
DFRobotDFPlayerMini myDFPlayer;


void setup()
{
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(9600);
  while (!myDFPlayer.begin(Serial1))
  {
  }
  delay(500);
  myDFPlayer.volume(8); // Set volume value. From 0 to 30
  delay(500);
  myDFPlayer.playFolder(1, 1);
}

void loop()
{

}
