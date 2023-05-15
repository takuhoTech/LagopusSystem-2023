//TODO:ピトー管analogreadの値をそのまま電圧として使っていた問題を修正済

#define Pin_SDP816 28
#define Pin_PLAYER_BUSY 2
#define Pin_RPM 26
//再生中は0(=LOW)になる

#include "DFRobotDFPlayerMini.h"
DFRobotDFPlayerMini myDFPlayer;

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
  byte volume = 24;
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

void Cadence_duration(void);
void Lagopus_Play_value(double);
bool Lagopus_notPlaying(void);
void Lagopus_Play(int, int);

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

void setup()
{
  Serial.begin(9600);
  pinMode(Pin_RPM, INPUT);
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

  while (!myDFPlayer.begin(Serial1))
  {
  }
  delay(5000);
  myDFPlayer.volume(LS.volume); // Set volume value. From 0 to 30
  delay(500);
  Lagopus_Play(2, 1);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop()
{
  delay(2000);
  AS.Airspeed_array_size = sizeof(AS.Airspeed_array) / sizeof(double);
  AS.delta_pressure_raw_cal = 3.3 - ((analogRead(Pin_SDP816) / 1023.0) * 3.3) - (3.3 / 2);

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
    AS.Airspeed = AS.Airspeed_array[50];
    AS.Airspeed = (AS.Airspeed * 0.5) + (AS.Airspeed_prev * 0.5);
    AS.Airspeed_prev = AS.Airspeed;
    if (millis()-RPM.last_time  > 5000)
    {
      RPM.cadence = 0;
    }
    else if (RPM.duration != 0)
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
  Lagopus_Play(2, 6);
  while (Lagopus_notPlaying() == 0);
  Lagopus_Play(3, 1);

  while (true)
  {
    while (true)
    {
      Serial.println(RPM.cadence/10.0);
      if ((Serial.available() > 0) && Lagopus_notPlaying())
      {
        LS.request_buffer = Serial.readStringUntil('.');
        LS.request_File = int(LS.request_buffer.toInt() % 100);
        LS.request_Folder = int((LS.request_buffer.toInt() - LS.request_File) / 100);
        Lagopus_Play(LS.request_Folder, LS.request_File);
      }
      else if ((RPM.cadence >= 10)&&(RPM.cadence < 110) && (millis() - LS.time_voice > 2000) && Lagopus_notPlaying())
      {
        Lagopus_Play_value(RPM.cadence/10.0);
      }
      delay(100);
    }
  }
}

void Lagopus_Play_value(double input)
{
  Lagopus_Play(9, int(input) + 1);
  delay(800);
  Lagopus_Play(10, int(10.0 * input + 0.5) - 10 * int(input) + 1);
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

void Cadence_duration()
{
  if (millis() - RPM.last_time > 300)
  {
    RPM.duration = millis() - RPM.last_time;
    RPM.last_time = millis();
  }
}
