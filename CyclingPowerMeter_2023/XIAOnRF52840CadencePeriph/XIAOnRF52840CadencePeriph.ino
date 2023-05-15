// https://tech.144lab.com/entry/arduino-ble1

#include <bluefruit.h>
#include <LSM6DS3.h>
#include <Wire.h>

#define coefficient 0.1666

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery

LSM6DS3 myIMU(I2C_MODE, 0x6A);
float aX, aY, aZ, gX, gY, gZ;
int cadence;

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

void setup() {
  Serial.begin(9600);
  while (!Serial);
  while (myIMU.begin() != 0) {
    Serial.println("Device error");
    delay(1000);
  }

  initBluefruit();
  Serial.println("initBluefruit");
  // Configure and Start BLE Uart Service
  bleuart.begin();
  Serial.println("initBlueUart");
  // Set up and start advertising
  startAdvertised();
  Serial.println("iniAdv");
}

void loop() {

  gZ = myIMU.readFloatGyroZ();
  cadence = abs(gZ * coefficient);
  Serial.println(cadence);

  String str = "RPM:";
  str += String(cadence);
  str += " PWR:";
  str += String(255);
  bleuart.print(str);
  delay(200);
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
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

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
