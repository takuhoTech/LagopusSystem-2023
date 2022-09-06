//TODO:ピトー管analogreadの値をそのまま電圧として使っていた問題を修正済

#define Pin_SDP816 28
#define Pin_US 26
#define Pin_RPM 27
#define Pin_PLAYER_BUSY 2
//再生中は0(=LOW)になる

#include <Wire.h>

#include <LSM6.h>
LSM6 imu;

#include "DFRobotDFPlayerMini.h"
DFRobotDFPlayerMini myDFPlayer;

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#define LPS25H_ADDRESS 0x5d /* SA0 -> VCC */
//#define LPS25H_ADDRESS  0x5c /* SA0 -> GND */
#define LPS25H_WHO_AM_I 0x0f
#define LPS25H_CTRL_REG1 0x20
#define LPS25H_PRESS_OUT_XL 0x28
#define LPS25H_PRESS_OUT_L 0x29
#define LPS25H_PRESS_OUT_H 0x2a
#define LPS25H_TEMP_OUT_L 0x2b
#define LPS25H_TEMP_OUT_H 0x2c

typedef struct
{
  double delta_pressure;
  double delta_pressure_prev;
  double delta_pressure_raw;
  double delta_pressure_raw_cal;
  double calibrated;
  const double Air_density = 1.166;
  double Pitot_tube_constant = 1.0;
  double delta_pressure_nofilter;
  double Airspeed;
  double Airspeed_prev;
  double Airspeed_raw;
  double Airspeed_array[100];
  int Airspeed_array_size;
  int i, k, j;
  double tmp;
} airspeed_sensor;
airspeed_sensor AS;

typedef struct
{
  const double polar_circumference = 40009000.0;
  const double equator_circumference = 40075000.0;
  double Latitude_home;
  double Longitude_home;
  long distance_from_home;
  long distance_from_home_prev;
  double Latitude;
  double Longitude;
  double GroundSpeed;
  double GroundSpeed_prev;
  double Altitude;
  double AltitudeMSL;
  int Heading;
  byte SIV;
  float pDOP;
} location;
location GNSS;

typedef struct
{
  int voice_order[13] = {3, 8, 6, 4, 5, 11, 12, 13, 15, 14, 16, 10, 9};
  int dist_target[8] = {200, 500, 600, 695, 1000, 2000, 5000, 10000};
  int dist_voice_order[8] = {2, 4, 6, 7, 8, 9, 10, 11};
  long time_voice_std;
  long time_voice;
  long time_voice_airspeed;
  long time_voice_speed;
  long time_voice_altitude;
  long time_voice_cadence_bad;
  long time_voice_cadence_good;
  int p, q;
  String request_buffer;
  int request_Folder;
  int request_File;
  byte volume = 30;
} lagopus;
lagopus LS;

typedef struct
{
  volatile long last_time = 0;
  volatile long duration = 0;
  volatile double cadence;
  volatile double cadence_prev;
  int flag;
} cadence;
cadence RPM;

typedef struct
{
  double resolution = 0.00061;
  double acc_X;
  double acc_Y;
  double acc_Z;
  double acc_X_prev;
  double acc_Y_prev;
  double acc_Z_prev;
  double degree_roll = 0;
  double degree_pitch = 0;
  double degree_roll_home = 0;
  double degree_pitch_home = 0;
} LSM6DS33;
LSM6DS33 IMU;

typedef struct
{
  double pressure;
  double pressure_prev;
  double pressure_home;
  double delta_altitude;
  //double delta_altitude_prev;
  double altitude = 10.0;
} altitude_pressure;
altitude_pressure AL_P;
typedef struct
{
  double altitude = 10.0;
  double altitude_prev;
  double distance_max = 10.7;
} altitude_distance;
altitude_distance AL_D;

const double altitude_home = 10.0;
double altitude = 10.0;

const double pi = 3.14159265359;
const double G_acc = 9.80665;

int count = 0;

void Logging(void);
void Lagopus_Play_value(double);
bool GNSS_update(void);
void GNSS_boot(void);
void Cadence_duration(void);
bool Lagopus_notPlaying(void);
void Lagopus_Play(int, int);


double Degree_to_Radian(double degree)
{
  return (degree * (pi / 180.0));
}

double Radian_to_Degree(double radian)
{
  return (radian * (180.0 / pi));
}

long distance(double lat1, double lng1, double lat2, double lng2)
{
  long dx = GNSS.equator_circumference * cos(Degree_to_Radian((lat1 + lat2) / 2.0)) * (abs(lng2 - lng1) / 360.0);
  long dy = GNSS.polar_circumference * (abs(lat2 - lat1) / 360.0);
  long dist = sqrt(sq(dx) + sq(dy));
  return (dist);
}

double sign(double input)
{
  if (input > 0)
  {
    return 1.0;
  }
  else if (input < 0)
  {
    return -1.0;
  }
  else
  {
    return 0.0;
  }
}

int whoAmI()
{
  Wire1.beginTransmission(LPS25H_ADDRESS);
  Wire1.write(LPS25H_WHO_AM_I);
  Wire1.endTransmission();

  Wire1.requestFrom(LPS25H_ADDRESS, 1);
  while (Wire1.available() < 1)
  {
    ;
  }

  return Wire1.read();
}

void setCtrlReg1()
{
  Wire1.beginTransmission(LPS25H_ADDRESS);
  Wire1.write(LPS25H_CTRL_REG1);
  Wire1.write(0xC0); // 25Hzにまであげる
  // Wire1.write(0x90); // default: 1Hz
  Wire1.endTransmission();
}

float getPressure()
{
  long pData = 0;

  for (int i = 0; i < 3; i++)
  {
    Wire1.beginTransmission(LPS25H_ADDRESS);
    Wire1.write(LPS25H_PRESS_OUT_XL + i);
    Wire1.endTransmission();

    Wire1.requestFrom(LPS25H_ADDRESS, 1);
    while (Wire1.available() < 1)
    {
      ;
    }

    pData |= Wire1.read() << (8 * i);
  }

  return pData / 4096.0;
}

float getTemperature()
{
  short tData = 0;

  for (int i = 0; i < 2; i++)
  {
    Wire1.beginTransmission(LPS25H_ADDRESS);
    Wire1.write(LPS25H_TEMP_OUT_L + i);
    Wire1.endTransmission();

    Wire1.requestFrom(LPS25H_ADDRESS, 1);
    while (Wire1.available() < 1)
    {
      ;
    }

    tData |= Wire1.read() << (8 * i);
  }

  return 42.5 + tData / 480.0;
}

void setup()
{
  pinMode(Pin_RPM, INPUT);

  Wire1.setSDA(14);
  Wire1.setSCL(15);
  Wire1.begin();
  // Serial.print("WHO_AM_I = 0x");
  // Serial.println(whoAmI(), HEX);
  setCtrlReg1();

  while (!imu.init())
  {
  }
  imu.enableDefault();

  attachInterrupt(Pin_RPM, Cadence_duration, RISING);
}

void setup1()
{
  pinMode(Pin_PLAYER_BUSY, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(9600);

  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(9600);

  Serial2.setTX(8);
  Serial2.setRX(9);
  Serial2.begin(9600);

  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin();
  // Wire.setClock(400000);

  while (!myDFPlayer.begin(Serial1))
  {
  }
  delay(5000);
  myDFPlayer.volume(LS.volume); // Set volume value. From 0 to 30
  delay(500);
  Lagopus_Play(2, 1);
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(2, 2);
  GNSS_boot();
  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  delay(2000);
  for (count = 0; count < 10; count++)
  {
    AL_P.pressure_home += getPressure();
    delay(100);
  }
  AL_P.pressure_home /= 10.0;

  AS.Airspeed_array_size = sizeof(AS.Airspeed_array) / sizeof(double);
  AS.delta_pressure_raw_cal = 3.3 - ((analogRead(Pin_SDP816) / 1023.0) * 3.3) - (3.3 / 2);

  for (count = 0; count < 10; count++)
  {
    imu.read();
    IMU.acc_X = IMU.acc_X_prev = imu.a.x * IMU.resolution;
    IMU.acc_Y = IMU.acc_Y_prev = imu.a.y * IMU.resolution;
    IMU.acc_Z = IMU.acc_Z_prev = imu.a.z * IMU.resolution;
    IMU.degree_roll_home  += Radian_to_Degree(atan2(IMU.acc_Y, -IMU.acc_Z));
    IMU.degree_pitch_home += Radian_to_Degree(atan(-IMU.acc_X / sqrt(IMU.acc_Y * IMU.acc_Y + IMU.acc_Z * IMU.acc_Z)));
    delay(100);
  }
  IMU.degree_roll_home /= 10.0;
  IMU.degree_pitch_home /= 10.0;

  while (true)
  {
    for (AS.k = 0; AS.k < AS.Airspeed_array_size; AS.k++)
    {
      AS.delta_pressure_raw = 3.3 - ((analogRead(Pin_SDP816) / 1023.0) * 3.3) - AS.delta_pressure_raw_cal;
      AS.delta_pressure = (sign((AS.delta_pressure_raw / 3.3) - 0.5) * pow(((AS.delta_pressure_raw / (3.3 * 0.4)) - 1.25), 2.0)) * 133.0;
      if (AS.delta_pressure < 0)
      {
        AS.delta_pressure = 0.0;
      }
      AS.Airspeed_raw = (AS.Pitot_tube_constant * sqrt((2.0 * AS.delta_pressure) / AS.Air_density));
      AS.Airspeed_array[AS.k] = AS.Airspeed_raw;
      AL_P.pressure = getPressure();
      AL_P.delta_altitude = (AL_P.pressure_home - AL_P.pressure) * 9.09;
      //AL_P.delta_altitude = (AL_P.delta_altitude * 0.01) + (AL_P.delta_altitude_prev * 0.99);
      //AL_P.delta_altitude_prev = AL_P.delta_altitude;
      AL_P.altitude = altitude_home + AL_P.delta_altitude;
      imu.read();
      IMU.acc_X = imu.a.x * IMU.resolution;
      IMU.acc_Y = imu.a.y * IMU.resolution;
      IMU.acc_Z = imu.a.z * IMU.resolution;
      IMU.acc_X = (IMU.acc_X * 0.01) + (IMU.acc_X_prev * 0.99);
      IMU.acc_X_prev = IMU.acc_X;
      IMU.acc_Y = (IMU.acc_Y * 0.01) + (IMU.acc_Y_prev * 0.99);
      IMU.acc_Y_prev = IMU.acc_Y;
      IMU.acc_Z = (IMU.acc_Z * 0.01) + (IMU.acc_Z_prev * 0.99);
      IMU.acc_Z_prev = IMU.acc_Z;
      IMU.degree_roll  = Radian_to_Degree(atan2(IMU.acc_Y, -IMU.acc_Z)) - IMU.degree_roll_home;
      IMU.degree_pitch = Radian_to_Degree(atan(-IMU.acc_X / sqrt(IMU.acc_Y * IMU.acc_Y + IMU.acc_Z * IMU.acc_Z))) - IMU.degree_pitch_home;
      delay(10);
    }
    for (AS.i = 0; AS.i < AS.Airspeed_array_size; AS.i++)
    {
      for (AS.j = AS.i + 1; AS.j < AS.Airspeed_array_size; AS.j++)
      {
        if (AS.Airspeed_array[AS.i] > AS.Airspeed_array[AS.j])
        {
          AS.tmp = AS.Airspeed_array[AS.i];
          AS.Airspeed_array[AS.i] = AS.Airspeed_array[AS.j];
          AS.Airspeed_array[AS.j] = AS.tmp;
        }
      }
    }
    AS.Airspeed = AS.Airspeed_array[94];
    AS.Airspeed = (AS.Airspeed * 0.5) + (AS.Airspeed_prev * 0.5);
    AS.Airspeed_prev = AS.Airspeed;
    AL_D.altitude = constrain((analogRead(Pin_US) * (3.3 / 5) * 2) / 100.0, 0, AL_D.distance_max);
    if (AL_D.altitude < 0.3)
    {
      altitude = 0.3;
    }
    else if (AL_D.altitude < 6.0)
    {
      altitude = AL_D.altitude;
    }
    else
    {
      altitude = (AL_D.altitude / AL_D.distance_max) * AL_P.altitude + (1.0 - (AL_D.altitude / AL_D.distance_max)) * AL_D.altitude;
    }

    if (RPM.duration != 0)
    {
      RPM.cadence = 60 / (RPM.duration / 1000.0);
    }
    //RPM.cadence = (RPM.cadence * 0.5) + (RPM.cadence_prev * 0.5);
    //RPM.cadence_prev = RPM.cadence;
    if ((RPM.cadence < 75) && (millis() - RPM.last_time < 2000))//故障により、更新されない値が連続で使われることの防止
    {
      if (RPM.flag > 0)
      {
        RPM.flag = 0;
      }
      RPM.flag--;
    }
    else if ((RPM.cadence > 85) && (RPM.cadence < 95) && (millis() - RPM.last_time < 2000))//故障により、更新されない値が連続で使われることの防止
    {
      if (RPM.flag < 0)
      {
        RPM.flag = 0;
      }
      RPM.flag++;
    }
    else
    {
      RPM.flag = 0;
    }
  }
}

void loop1()
{
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(2, 3);
  for (count = 0; count < 8; count++)
  {
    GNSS_update();
    GNSS.Latitude_home = GNSS.Latitude;
    GNSS.Longitude_home = GNSS.Longitude;
    delay(1000);
  }
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(2, 4);
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(2, 5);
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(2, 6);
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(3, 1);

  while (true)
  {
    do
    {
      GNSS_update();
      if ((Serial.available() > 0) && Lagopus_notPlaying())
      {
        LS.request_buffer = Serial.readStringUntil('.');
        LS.request_File = int(LS.request_buffer.toInt() % 100);
        LS.request_Folder = int((LS.request_buffer.toInt() - LS.request_File) / 100);
        Lagopus_Play(LS.request_Folder, LS.request_File);
      }
      else if ((AS.Airspeed >= 1) && (millis() - LS.time_voice > 25000) && Lagopus_notPlaying())
      {
        Lagopus_Play(8, 1);
        delay(800);
        Lagopus_Play_value(AS.Airspeed);
      }
      Logging();
      delay(100);
    } while (((GNSS.GroundSpeed) < 3) || (GNSS.distance_from_home < 15));
    Serial2.println("START");
    Lagopus_Play(3, 2);
    while (true)
    {
      GNSS_update();
      if ((millis() - LS.time_voice_std > 30000) && Lagopus_notPlaying() && (LS.p < 13))
      {
        Lagopus_Play(3, LS.voice_order[LS.p]);
        LS.time_voice_std = millis();
        LS.p++;
      }
      else if ((GNSS.distance_from_home > LS.dist_target[LS.q]) && Lagopus_notPlaying() && (LS.q < 8))
      {
        Lagopus_Play(6, LS.dist_voice_order[LS.q]);
        LS.q++;
      }
      else if ((AS.Airspeed < 4.5) && (millis() - LS.time_voice_airspeed > 50000) && Lagopus_notPlaying())
      {
        Lagopus_Play(7, 3);
        LS.time_voice_airspeed = millis();
      }
      else if ((GNSS.GroundSpeed < 4.5) && (millis() - LS.time_voice_speed > 50000) && Lagopus_notPlaying())
      {
        Lagopus_Play(7, 2);
        LS.time_voice_speed = millis();
      }
      else if ((altitude < 1.5) && (millis() - LS.time_voice_altitude > 50000) && Lagopus_notPlaying())
      {
        Lagopus_Play(7, 4);
        LS.time_voice_altitude = millis();
      }
      else if ((RPM.flag < -8) && (millis() - LS.time_voice_cadence_bad > 50000) && Lagopus_notPlaying())
      {
        Lagopus_Play(7, 1);
        LS.time_voice_cadence_bad = millis();
      }
      else if ((RPM.flag > 8) && (millis() - LS.time_voice_cadence_good > 50000) && Lagopus_notPlaying())
      {
        Lagopus_Play(5, random(1, 5));
        LS.time_voice_cadence_good = millis();
      }
      Logging();
      delay(100);
    }
    //Serial2.println("STOP");
  }
}

void Logging()
{
  Serial2.print(RPM.cadence, 3);
  Serial2.print("  ");
  Serial2.print(AL_P.pressure, 3);
  Serial2.print("  ");
  Serial2.print(altitude, 3);
  Serial2.print("  ");
  Serial2.print(AS.Airspeed, 3);
  Serial2.print("  ");
  Serial2.print(AS.Airspeed_raw, 3);
  Serial2.print("  ");
  Serial2.print(GNSS.GroundSpeed, 3);
  Serial2.print("  ");
  Serial2.print(GNSS.distance_from_home);
  Serial2.print("  ");
  Serial2.print(GNSS.Altitude, 3);
  Serial2.print("  ");
  Serial2.print(GNSS.AltitudeMSL, 3);
  Serial2.print("  ");
  Serial2.print(GNSS.Heading);
  Serial2.print("  ");
  Serial2.print(GNSS.Latitude, 7);
  Serial2.print("  ");
  Serial2.print(GNSS.Longitude, 7);
  Serial2.print("  ");
  Serial2.print(GNSS.SIV);
  Serial2.print("  ");
  Serial2.print(GNSS.pDOP, 3);
  Serial2.print("  ");
  Serial2.print(IMU.degree_roll, 2);
  Serial2.print("  ");
  Serial2.println(IMU.degree_pitch, 2);
  /*Serial.print("rpm:");
    Serial.print(RPM.cadence, 3);
    Serial.print(" ");
    Serial.print("p:");
    Serial.print(AL_P.pressure, 3);
    Serial.print(" ");
    Serial.print("alt:");
    Serial.print(altitude, 3);
    Serial.print(" ");
    Serial.print("AS:");
    Serial.print(AS.Airspeed, 3);
    Serial.print(" ");
    Serial.print("AS_raw:");
    Serial.print(AS.Airspeed_raw, 3);
    Serial.print(" ");
    Serial.print("GS:");
    Serial.print(GNSS.GroundSpeed, 3);
    Serial.print(" ");
    Serial.print("dist:");
    Serial.print(GNSS.distance_from_home);
    Serial.print(" ");
    Serial.print("Alt:");
    Serial.print(GNSS.Altitude, 3);
    Serial.print(" ");
    Serial.print("AltMSL:");
    Serial.print(GNSS.AltitudeMSL, 3);
    Serial.print(" ");
    Serial.print("Heading:");
    Serial.print(GNSS.Heading);
    Serial.print(" ");
    Serial.print("Lat:");
    Serial.print(GNSS.Latitude, 3);
    Serial.print(" ");
    Serial.print("Lng:");
    Serial.print(GNSS.Longitude, 3);
    Serial.print(" ");
    Serial.print("SIV:");
    Serial.print(GNSS.SIV);
    Serial.print(" ");
    Serial.print("pDOP:");
    Serial.println(GNSS.pDOP, 3);*/
  Serial.print(RPM.cadence, 3);
  Serial.print(" ");
  Serial.print(AL_P.pressure, 3);
  Serial.print(" ");
  Serial.print(altitude, 3);
  Serial.print(" ");
  Serial.print(AS.Airspeed, 3);
  Serial.print(" ");
  Serial.print(AS.Airspeed_raw, 3);
  Serial.print(" ");
  Serial.print(GNSS.GroundSpeed, 3);
  Serial.print(" ");
  Serial.print(GNSS.distance_from_home);
  Serial.print(" ");
  Serial.print(GNSS.Altitude, 3);
  Serial.print(" ");
  Serial.print(GNSS.AltitudeMSL, 3);
  Serial.print(" ");
  Serial.print(GNSS.Heading);
  Serial.print(" ");
  Serial.print(GNSS.Latitude, 3);
  Serial.print(" ");
  Serial.print(GNSS.Longitude, 3);
  Serial.print(" ");
  Serial.print(GNSS.SIV);
  Serial.print(" ");
  Serial.print(GNSS.pDOP, 3);
  Serial.print(" ");
  Serial.print(IMU.degree_roll, 2);
  Serial.print(" ");
  Serial.println(IMU.degree_pitch, 2);
}

bool Lagopus_notPlaying()
{
  return ((digitalRead(Pin_PLAYER_BUSY) == 1) && (millis() - LS.time_voice > 500));
}

void Lagopus_Play(int Folder, int Num)
{
  myDFPlayer.playFolder(Folder, Num);
  LS.time_voice = millis();
}

void Lagopus_Play_value(double input)
{
  Lagopus_Play(9, int(input) + 1);
  delay(800);
  Lagopus_Play(10, int(10.0 * input + 0.5) - 10 * int(input) + 1);
}

void Cadence_duration()
{
  if (millis() - RPM.last_time > 500)
  {
    RPM.duration = millis() - RPM.last_time;
    RPM.last_time = millis();
  }
}

bool GNSS_update()
{
  GNSS.SIV = myGNSS.getSIV();
  GNSS.pDOP = myGNSS.getPDOP() / 100.0;
  if ((GNSS.SIV < 6) || (GNSS.pDOP > 5))
  {
    return 0;
  }
  GNSS.Latitude = myGNSS.getLatitude() / 10000000.0;
  GNSS.Longitude = myGNSS.getLongitude() / 10000000.0;
  GNSS.GroundSpeed = myGNSS.getGroundSpeed() / 1000.0;
  GNSS.GroundSpeed = (GNSS.GroundSpeed * 0.4) + (GNSS.GroundSpeed_prev * 0.6);
  GNSS.GroundSpeed_prev = GNSS.GroundSpeed;
  GNSS.Altitude = myGNSS.getAltitude() / 1000.0;
  GNSS.AltitudeMSL = myGNSS.getAltitudeMSL() / 1000.0;
  GNSS.Heading = myGNSS.getHeading() / 100000.0;
  GNSS.distance_from_home = distance(GNSS.Latitude_home, GNSS.Longitude_home, GNSS.Latitude, GNSS.Longitude);
  return 1;
}

void GNSS_boot()
{
  while (myGNSS.begin() == false) // Attempt to re-connect
  {
  }
  myGNSS.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
  // myGNSS.setNavigationFrequency(4); //Set output to 5 times a second
  myGNSS.setAutoPVT(true);
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); // Save (only) the communications port settings to flash and BBR
  delay(1000);
  do
  {
    GNSS_update();
    if ((Serial.available() > 0) && Lagopus_notPlaying())
    {
      LS.request_buffer = Serial.readStringUntil('.');
      LS.request_File = int(LS.request_buffer.toInt() % 100);
      LS.request_Folder = int((LS.request_buffer.toInt() - LS.request_File) / 100);
      Lagopus_Play(LS.request_Folder, LS.request_File);
    }
    else if ((AS.Airspeed >= 1) && (millis() - LS.time_voice > 25000) && Lagopus_notPlaying())
    {
      Lagopus_Play(8, 1);
      delay(800);
      Lagopus_Play_value(AS.Airspeed);
    }
    Logging();
    delay(100);
  } while ((GNSS.SIV < 8) || (GNSS.pDOP > 5));
}
