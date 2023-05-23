#include "PeripheralBLE.h"
#include <bluefruit.h>

BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart;
BLEBas  blebas;  // battery

void PeripheralBLE::begin(char *name)
{
  Bluefruit.autoConnLed(false); //true:BLE LED is enabled on CONNECT  false:Control LED manually via PIN 19
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX); //Config the peripheral connection with maximum bandwidth more SRAM required by SoftDevice
  Bluefruit.begin(); //Note: All config***() function must be called before begin()

  Bluefruit.setTxPower(8); //default is 4dBm. Check bluefruit.h for supported values.
  Bluefruit.setName(name);
  //Bluefruit.setName(getMcuUniqueID()); //useful with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  bledfu.begin(); //To be consistent OTA DFU should be added first if it exists
  bledis.setManufacturer("Adafruit Industries"); //Configure Device Information Service
  bledis.setModel("Bluefruit Feather52");
  bledis.begin(); //Start Device Information Service
  BLEUart::begin(); //Configure and Start BLE Uart Service
  blebas.begin(); //Start BLE Battery Service
  blebas.write(100);

  //Set up and start advertising
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE); //Advertising packet
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleuart); //Include bleuart 128-bit uuid

  Bluefruit.ScanResponse.addName(); //Secondary Scan Response packet (optional) Since there is no room for 'Name' in Advertising packet

  //For recommended advertising interval https://developer.apple.com/library/content/qa/qa1931/_index.html
  Bluefruit.Advertising.restartOnDisconnect(true); //Enable auto advertising if disconnected
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms Interval: fast mode = 20 ms, slow mode = 152.5 ms
  Bluefruit.Advertising.setFastTimeout(30);      // Timeout for fast mode is 30 seconds
  Bluefruit.Advertising.start(0);                // Start(timeout) with timeout = 0 will advertise forever (until connected)
}

void PeripheralBLE::connect_callback(uint16_t conn_handle) //callback invoked when central connects
{
  BLEConnection* connection = Bluefruit.Connection(conn_handle); //Get the reference to current connection
  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));
  isopen = 1;
  //Serial.print("Connected to ");
  //Serial.println(central_name);
}

void PeripheralBLE::disconnect_callback(uint16_t conn_handle, uint8_t reason) //Callback invoked when a connection is dropped
{
  (void) conn_handle; //conn_handle connection where this event happens
  (void) reason; //reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
  isopen = 0;
  //Serial.println();
  //Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}

bool PeripheralBLE::isOpen()
{
  return isopen;
}
