/*
 * Test to read a LiPo battery connected to VBAT on the
 * underside of XIAO BLE boards.
 * 
 * Note that the ADC input from the VBAT voltage divider is
 * NOT pin P0_31 as shown on the schematic!
 * 
 * March, 2022
 * davekw7x
 */

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);
  pinMode(VBAT_ENABLE, OUTPUT);
}

const double vRef = 3.3; // Assumes 3.3V regulator output is ADC reference voltage
const unsigned int numReadings = 1024; // 10-bit ADC readings 0-1023, so the factor is 1024

void loop()
{
  digitalWrite(VBAT_ENABLE, LOW);
  unsigned int adcCount = analogRead(PIN_VBAT);
  double adcVoltage = (adcCount * vRef) / numReadings;
  double vBat = adcVoltage*1510.0/510.0; // Voltage divider from Vbat to ADC
  Serial.println(vBat);
  delay(1000);
}
