void setup() {
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  Serial.begin(9600);
  Serial1.begin(115200);
  //while (!Serial);
  while (!Serial1);
  delay(500);
  Serial1.println("");
  delay(2000);
  Serial1.println("nxplayer");
  delay(500);
}

void loop() {
  if (Serial.available() > 0)
  {
    char c = Serial.read();
    Serial1.write(c);
  }
  if (Serial1.available() > 0) {
    char c = Serial1.read();
    Serial.write(c);
  }

  if (!digitalRead(21))
  {
    Serial1.println("stop");
    delay(500);
    Serial1.println("play 1.wav");
    delay(500);
  }
  else  if (!digitalRead(22))
  {
    Serial1.println("stop");
    delay(500);
    Serial1.println("play 2.wav");
    delay(500);
  }
}
