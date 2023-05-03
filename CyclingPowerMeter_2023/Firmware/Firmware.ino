//#include <Adafruit_TinyUSB.h>

#define DRDY D1
long val;
long temp[3];
int RREGwait;

void setup() {
  Serial.begin(115200); while (!Serial); Serial.println("Serial USB [ OK ]");
  Serial1.begin(1000000); while (!Serial1); Serial.println("Serial ADC [ OK ]");
  pinMode(LEDG, OUTPUT); digitalWrite(LEDG, HIGH);
  pinMode(DRDY, INPUT);
  Serial1.write(0x55); Serial1.write(0x07);//ADS122U04 RESET
  delay(1000);
  //================Register Setting=============================================================================
  RREGwait = 1200; //RREG wait time usec 2*Tbaud(10usec)

  //Reg00(01001110=0x4E)[7-4:AINp=AIN1 AINn=AIN2|3-1:GAIN128|0:PGA enable]
  Serial1.write(0x55); Serial1.write(0x40); Serial1.write(0x4E);
  delayMicroseconds(1000);

  //Reg01(01111000=0x78)[7-5:DataRate350sps|4:Turbo mode|3:Continuous conversion mode|2-1:Internal 2.048V Ref|0:Temp disable]
  Serial1.write(0x55); Serial1.write(0x42); Serial1.write(0x78);
  delayMicroseconds(1000);

  //Reg02(00000000=0x00)[7:DRDY|6:DCNT disable|5-4:CRC disable|3:BCS Off|2-0:IDAC off]
  Serial1.write(0x55); Serial1.write(0x44); Serial1.write((byte)0);
  delayMicroseconds(1000);

  //Reg03(00000001=0x01)[7-5:default|4-2:default|1:Reserved|0:Automatic data read mode]
  Serial1.write(0x55); Serial1.write(0x46); Serial1.write(0x01);
  delayMicroseconds(1000);

  //Reg04(01111000=0x78)[7:Reserved|6:GPIO2output|5:GPIO1input|4:GPIO0input|3:GPIO2SEL DRDY|2:GPIO2 LogicLow|1:GPIO1 LogicLow|0:GPIO0 LogicLow]
  Serial1.write(0x55); Serial1.write(0x48); Serial1.write(0x48);
  delayMicroseconds(1000);

  Serial1.write(0x55); Serial1.write(0x08);//Start/Sync
  //delayMicroseconds(100);//************************このWAIT重要************************************
  delay(1000);
  Serial.println("ADC Config [ OK ]");
}

void loop() {
  //delayMicroseconds(500);//5msec Period
  //Serial1.write(0x55);
  //Serial1.write(0x10);
  while (digitalRead(DRDY) == 1);
  delayMicroseconds(10);
  temp[0] = Serial1.read();
  //delayMicroseconds(10);
  temp[1] = Serial1.read();
  //delayMicroseconds(10);
  temp[2] = Serial1.read();
  //delayMicroseconds(10);
  val = temp[2] << 16 | temp[1] << 8 | temp[0] ;
  Serial.print(temp[2]);
  Serial.print("  ");
  Serial.print(temp[1]);
  Serial.print("  ");
  Serial.print(temp[0]);
  Serial.print("  ");
  Serial.println(val);
}
