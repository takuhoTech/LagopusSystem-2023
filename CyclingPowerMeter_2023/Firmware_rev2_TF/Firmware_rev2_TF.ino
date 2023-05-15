#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>
#include "LSM6DS3.h"
#include "Wire.h"

#define PIN_WAKE  7
#define PIN_POWER A5

LSM6DS3 myIMU(I2C_MODE, 0x6A); //I2C device address 0x6A

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery
const uint8_t PrimaryServiceUUID[] = {
  // ccdd0000-0011-2233-4455-66778899aabb
  0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44,
  0x33, 0x22, 0x11, 0x00, 0x00, 0x00, 0xdd, 0xcc,
};
BLEService PrimaryService(PrimaryServiceUUID);
const uint8_t BtnCharacteristicUUID[] = {
  // ccdd0001-0011-2233-4455-66778899aabb
  0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44,
  0x33, 0x22, 0x11, 0x00, 0x01, 0x00, 0xdd, 0xcc,
};
BLECharacteristic BtnCharacteristic(BtnCharacteristicUUID);

void initBluefruit() {
  /*
     自動LED制御のON/OFF
     trueの場合、ターゲットのLED1の制御をBLEドライバーに奪われる
  */
  Bluefruit.autoConnLed(true);

  /*
     周辺機器との接続を最大帯域で設定する
     SoftDeviceに必要なSRAMが増える
     注意：すべてのconfig***()関数は、begin()の前に呼び出す必要があります。
  */
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();

  // 送信出力電力を設定
  Bluefruit.setTxPower(4);
  // device名の設定
  Bluefruit.setName("tedeyannen,tede");
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

}

void startAdvertised() {
  // アドバタイズパケットのフラグ
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  // 電力とデバイス名とプライマリサービスUUIDをアドバタイズ情報に載せる
  Bluefruit.Advertising.addTxPower();
  Bluefruit.ScanResponse.addName();
  Bluefruit.Advertising.addService(PrimaryService);
  // 接続を切ったらアドバタイズを再開する設定
  Bluefruit.Advertising.restartOnDisconnect(true);
  // アドバタイズはファストモードー＞スローモードー＞ストップという状態遷移
  // ファストモードの時間を指定する（３０秒）
  Bluefruit.Advertising.setFastTimeout(30);  // number of seconds in fast
  // 何秒間アドバタイズするか（ゼロの場合はノンストップ）
  Bluefruit.Advertising.start(0);
}

void setup()
{
  initBluefruit();
  bleuart.begin();
  startAdvertised();
  //Bluefruit.begin(); // Sleep functions need the softdevice to be active.
  pinMode(PIN_WAKE,  INPUT_PULLUP_SENSE);    // this pin (PIN_WAKE) is pulled up and wakes up the feather when externally connected to ground.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  delay(500);
  digitalWrite(LED_GREEN, HIGH);

  Serial.begin(115200);
  //while (!Serial);
  while (myIMU.begin() != 0);
}

void loop()
{
  double angular_velocity_degree = 0, angular_velocity_radian = 0, degree = 0;
  typedef struct
  {
    double last = 0;
    double current = 0;
    double delta = 0;
    long count = 0;
  } Time;
  Time time;
  typedef struct
  {
    double raw = 0;
    long sum = 0;
    int avg = 0;
  } Cadence;
  Cadence cadence;
  typedef struct
  {
    const double SLOPE = 5.56209;
    const int OFFSET = 77;
    int strain = 0;
    int raw = 0;
    long sum = 0;
    int avg = 0;
  } Power;
  Power power;

  while (true)
  {
    do {
      if (digitalRead(PIN_WAKE) == LOW) {
        digitalWrite(LED_RED, LOW);
        delay(500);
        digitalWrite(LED_RED, HIGH);
        delay(250);
        sd_power_system_off();
      }
      angular_velocity_degree = myIMU.readFloatGyroZ(); //単位は度
      angular_velocity_radian = angular_velocity_degree * (PI / 180.0);
      cadence.raw = angular_velocity_radian * (30.0 / PI);
      cadence.sum += cadence.raw;

      time.current = micros();
      time.delta = time.current - time.last;
      time.last = time.current;
      degree += angular_velocity_degree * (time.delta / 1000000);
      time.count++;

      power.strain = analogRead(PIN_POWER) - power.OFFSET;
      if (power.strain < 0) {
        power.strain = 0;
      }
      power.raw = (power.strain / power.SLOPE) * angular_velocity_radian;
      if (power.raw < 0) {
        power.raw = 0;
      }
      power.sum += power.raw;

      //Serial.print(power.raw); Serial.print("  "); Serial.print(cadence.raw); Serial.print("  "); Serial.println(degree);
      delay(10);
    } while (degree < 360);
    degree = 0;
    cadence.avg = cadence.sum / time.count;
    power.avg = power.sum / time.count;
    cadence.sum = 0;
    power.sum = 0;
    time.count = 0;

    String str = "RPM:";
    str += String(cadence.avg);
    str += " PWR:";
    str += String(power.avg);
    bleuart.print(str);
    //Serial.println(str);
  }
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  //Serial.print("Connected to ");
  //Serial.println(central_name);
}

/**
   Callback invoked when a connection is dropped
   @param conn_handle connection where this event happens
   @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
*/
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  //Serial.println();
  //Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
