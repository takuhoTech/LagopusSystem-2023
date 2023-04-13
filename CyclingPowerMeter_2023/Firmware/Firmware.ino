//#include <Adafruit_TinyUSB.h>

#define DRDY D1 //DataRdy Pin 1
//—————-Texsus ADS122U04————————–
//int DRDY = P0_03; //DataRdy Pin 1
double ft1, ft2;
int RREGwait;
int flipN;
uint8_t RR0, RR1, RR2, RR3, RR4;
uint8_t d0, d1, d2, d3, d4, d5, d6;
double data1ch, data2ch;
int DRDYflag = 0;

void setup() {
// シリアル設定
Serial.begin(115200);
while(!Serial);
Serial.println("Serial USB [ OK ]");
Serial1.begin(115200, SERIAL_8N1);
while(!Serial1);
Serial.println("Serial ADC [ OK ]");
//ピン割り込み設定
pinMode(DRDY, INPUT_PULLUP);
//attachInterrupt(digitalPinToInterrupt(2),呼び出される関数,FALLING);
attachInterrupt(DRDY, flip,FALLING);//DRDY 下がりエッジで割り込み関数flipへ飛ぶ
//=============ADS122U04レジスタ初期設定==========================================
//ADS122U04 RESET
Serial1.write(0x55);
Serial1.write(0x06);
delayMicroseconds(3000);
//================Register Setting=============================================================================
RREGwait = 600; //RREG wait time usec 2*Tbaud(10usec)

//—–Register00[7:4(MUX)3:1(GAIN)0(PGA_BYPASS)]————————————————————–
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x40);//WriteREGister 0x04 0x00(0000register00 Selected)
Serial1.write(0x0E);//7:0(0000 1110=0x0E)[7:4(0000)MUX AINp=AIN0,AINn=AIN1 3:1(111)GAIN128, 0:0(0)PGA enabled]
delayMicroseconds(1000);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x20);//Synchronization word 0x55 Read REGister00=0010 0000=0x20
delayMicroseconds(RREGwait);
if (Serial1.available()==1)
{
RR0 = Serial1.read();
delayMicroseconds(RREGwait);
Serial.println();
Serial.print("Register00=0x0E:");
Serial.println(RR0,HEX);
delayMicroseconds(RREGwait);
}

//—–Register01[7:5(DataRate)4:(OperationMode)3:(ConversionMode)2:1(VREF)0:(TemperatureSensor mode)]———
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x42);// WriteREGister 0x04 0x02(0010register01 Selected)
Serial1.write(0xAE);//7:0(1010 1110=0xAE)[7:5(101)DataRate600sps,4:(0)Normal Mode, 3:(1)Continuous conversion mode,2:1(11)VDD-VSS VRef,0:(0)Temp disabled]
delayMicroseconds(1000);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x22);//Synchronization word 0x55 Read REGister01=0010 0010=0x22
delayMicroseconds(RREGwait);
if (Serial1.available()==1)
{
RR1 = Serial1.read();
delayMicroseconds(RREGwait);
Serial.print("Register01=0xAE:");
Serial.println(RR1,HEX);
delayMicroseconds(RREGwait);

}

//—–Register02[7:DRDY)6:(DCNT)5:4(CRC)3:(BCS)2:0(IDAC)]—————————————————–
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x44);// WriteREGister 0x04 0x04(0100register02 Selected)
Serial1.write(0x08);//7:0(0000 1000=0x08)[7:(0)DRDY,6:(0)DCNT disable, 5:4(00)) inverte,3:(1)BCS On,2:0(0)IDAC off]
delayMicroseconds(1000);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x24);//Synchronization word 0x55 Read REGister02=0010 0100=0x24
delayMicroseconds(RREGwait);
if (Serial1.available()==1)
{
RR2 = Serial1.read();
delayMicroseconds(RREGwait);
Serial.print("Register02=0x08:");
Serial.println(RR2,HEX);
delayMicroseconds(RREGwait);
}

//—-Register03[7:5(I1MUX)4:2(I2MUX)1(RSERVED)0(AUTO)]—————————————————-
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x46);// WriteREGister 0x04 0x06(0110register03 Selected)
Serial1.write((byte)0);//7:0(00000000=0x00)[7:5(000)default,4:2(000)default, 1:(0)Reserved,0:(0)Manual Read
delayMicroseconds(1000);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x26);//Synchronization word 0x55 Read REGister03=0010 0110=0x26
delayMicroseconds(RREGwait);
if (Serial1.available()==1)
{
RR3 = Serial1.read();
delayMicroseconds(1000);
Serial.print("Register03=0x00:");
Serial.println(RR3,HEX);
delayMicroseconds(RREGwait);
}

delayMicroseconds(1000);
//—-Register04[7:(Reserved)6:(GPIO2DIR)5:(GPIO1DIR)4:(GPIO0DIR)3:(GPIO2SEL)2:(GPIO2DAT)1:(GPIO1DAT)0:(GPIODAT0)]—————————————————-
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x48);// WriteREGister 0x04 0x08(1000register04 Selected)
Serial1.write(0x48);//7:0(0100 1000=0x08)[6:(1)GPIO2output,5:(0)GPIO1Input, 4:(0)GPIO0Input,3:(1)GPIO2SEL DRDY,2:(0)GPIO2DAT Low,1:(0)GPIO1DAT Low,0:(0)GPIO0DAT Low
delayMicroseconds(1000);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x28);//Synchronization word 0x55 Read REGister04=0010 1000=0x28
delayMicroseconds(RREGwait);
if (Serial1.available()==1)
{
RR4 = Serial1.read();
delayMicroseconds(RREGwait);
Serial.print("Register04=0x48:");
Serial.println(RR4,HEX);
delayMicroseconds(RREGwait);
}

//Serial.println("====Start/Sync Go=============");
//Start/Sync
Serial1.write(0x55);
Serial1.write(0x08);
delayMicroseconds(100);//************************このWAIT重要************************************
Serial.println("ADC Config [ OK ]");
}

void loop() {
// put your main code here, to run repeatedly:
delayMicroseconds(500);//5msec Period
//Serial.println(Serial1.available());
Serial.println(digitalRead(DRDY));
}

//==================InterruptIn DRDY====================
void flip() {
flipN++;
if (flipN % 2 == 0)
{
//1CH—–Register00[7:4(MUX)3:1(GAIN)0(PGA_BYPASS)]————————————————————–
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x40);//WriteREGister 0x04 0x00(0000register00 Selected)
Serial1.write(0x0E);//7:0(0000 1110=0x0E)[7:4(0000)MUX AINp=AIN0,AINn=AIN1 3:1(111)GAIN128, 0:0(0)PGA enabled]
delayMicroseconds(50);

Serial1.write(0x55);
Serial1.write(0x10);
delayMicroseconds(10);
d0 = Serial1.read();
delayMicroseconds(10);
d1 = Serial1.read();
delayMicroseconds(10);
d2 = Serial1.read();
delayMicroseconds(10);
data1ch = (d0 + d1 * 256 + d2 * 65536) * 0.000393391;
ft1 = (double)micros() / 1000;
// Serial.print("%d,%8.3f\n\r",data1ch,ft);
}
else
{
//2CH—–Register00[7:4(MUX)3:1(GAIN)0(PGA_BYPASS)]————————————————————–
delayMicroseconds(50);
Serial1.write(0x55);//Synchronization word 0x55
Serial1.write(0x40);//WriteREGister 0x04 0x00(0000register00 Selected)
Serial1.write(0x6E);//7:0(0110 1110=0x6E)[7:4(0110)MUX AINp=AIN2,AINn=AIN3 3:1(111)GAIN128, 0:0(0)PGA enabled]
delayMicroseconds(50);

Serial1.write(0x55);
Serial1.write(0x10);
delayMicroseconds(10);
d3 = Serial1.read();
delayMicroseconds(10);
d4 = Serial1.read();
delayMicroseconds(10);
d5 = Serial1.read();
delayMicroseconds(10);
data2ch = (d3 + d4 * 256 + d5 * 65536) * 0.000393391;

//Serial.print("d0=%x,d1=%x,d2=%x,data=%d\n\r",d0,d1,d2,data);
ft2 = (double)micros() / 1000;
//Serial.print("%d:CH2=%d,%8.3f\n\r",flipN,data2ch,ft);
Serial.print(data1ch);
Serial.print(",");
Serial.print(data2ch);
Serial.print(",");
Serial.print(ft1);
Serial.print(",");
Serial.println(ft2);

//myled = !myled;
//Serial.print("————-DRDY Falled—————-\n\r");
}
}
