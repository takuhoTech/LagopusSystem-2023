void setup() {
  Serial.begin(115200);
  Serial.println("start");
  Serial1.begin(115200);
  Serial1.write(0x55); Serial1.write(0x07); //reset
}

void loop() {
  Serial1.write(0x55); Serial1.write(0x40); Serial1.write(0x4E); // Ain0 input single
  //Serial1.write(0x55); Serial1.write(0x42); Serial1.write(0x78); // Ain0 input single
  Serial1.write(0x55); Serial1.write(0x08); // one shot
  Serial1.write(0x55); Serial1.write(0x10); // Read
  long temp0 = Serial1.read() ; long temp1 = Serial1.read() ; long temp2 = Serial1.read() ;
  long temp = temp2 << 16 | temp1 << 8 | temp0 ;
  //float volts = temp * 2.048 / 8388608.0 ;
  Serial.print(temp0);
  Serial.print("  ");
  Serial.print(temp1);
  Serial.print("  ");
  Serial.print(temp2);
  Serial.print("  ");
  Serial.println(temp);
  delay(100);
}
