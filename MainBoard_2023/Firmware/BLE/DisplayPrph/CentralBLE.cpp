#include "CentralBLE.h"
#include <bluefruit.h>
#include <queue>

namespace std {
void __throw_length_error(char const*) {}
void __throw_bad_alloc() {}
}

void CentralBLE::addPrphName(char *name)
{
  for (int i = 0; i < MAX_PERIPHERAL; i++)
  {
    if (Peripheral[i].Name.length() == 0)
    {
      Peripheral[i].Name = name;
      return;
    }
  }
}

void CentralBLE::begin(char *name)
{
  for (int i = 0; i < MAX_PERIPHERAL; i++)
  {
    Peripheral[i].ID = 255;
  }
  Bluefruit.autoConnLed(false);
  //blinkTimer.begin(100, blink_timer_callback); //Initialize blinkTimer for 100 ms and start it
  //blinkTimer.start();
  Bluefruit.begin(0, MAX_PERIPHERAL); //dafault:Peripheral=0,Central=4
  Bluefruit.setName(name); //Set Name
  for (uint8_t idx = 0; idx < BLE_MAX_CONNECTION; idx++) //Init peripheral pool
  {
    prphs[idx].conn_handle = BLE_CONN_HANDLE_INVALID; //Invalid all connection handle
    prphs[idx].bleuart.begin(); //All of BLE Central Uart Serivce
    prphs[idx].bleuart.setRxCallback(bleuart_rx_callback);
  }
  Bluefruit.Central.setConnectCallback(connect_callback); //Callbacks for Central
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);
  /* Start Central Scanning
     - Enable auto scan if disconnected
     - Interval = 100 ms, window = 80 ms
     - Filter only accept bleuart service in advertising
     - Don't use active scan (used to retrieve the optional scan response adv packet)
     - Start(0) = will scan forever since no timeout is given
  */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE);
  Bluefruit.Scanner.useActiveScan(false);       // Don't request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds
}

void CentralBLE::scan_callback(ble_gap_evt_adv_report_t* report) //Callback invoked when scanner picks up an advertising packet
{
  // Since we configure the scanner with filterUuid()
  // Scan callback only invoked for device with bleuart service advertised
  // Connect to the device with bleuart service in advertising packet
  Bluefruit.Central.connect(report); //@param report Structural advertising data
}

void CentralBLE::connect_callback(uint16_t conn_handle) //Callback invoked when an connection is established
{
  int id  = findConnHandle(BLE_CONN_HANDLE_INVALID); //Find an available ID to use
  if ( id < 0 ) return; //Eeek: Exceeded the number of connections !!!
  prph_info_t* peer = &prphs[id]; //アドレス渡し
  peer->conn_handle = conn_handle;

  Bluefruit.Connection(conn_handle)->getPeerName(peer->name, sizeof(peer->name) - 1);

  String tmp = String(peer->name);

  for (int i = 0; i < MAX_PERIPHERAL; i++)
  {
    if (tmp == Peripheral[i].Name) //Air //tmp.indexOf(PeripheralName[0]) != -1
    {
      Peripheral[i].ID = id;
    }
  }
  //Serial.print("Connected to ");
  //Serial.println(peer->name);
  //Serial.print("Discovering BLE UART service ... ");
  if ( peer->bleuart.discover(conn_handle) )
  {
    //Serial.println("Found it");
    peer->bleuart.enableTXD();
    //Serial.println("Continue scanning...");
    Bluefruit.Scanner.start(0);
  }
  else
  {
    //Serial.println("Found...NOTHING!");
    Bluefruit.disconnect(conn_handle); //disconnect since we couldn't find bleuart service
  }
  connection_num++;
}

void CentralBLE::disconnect_callback(uint16_t conn_handle, uint8_t reason) //Callback invoked when a connection is dropped
{
  (void) conn_handle;
  (void) reason; //reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
  connection_num--;
  int id  = findConnHandle(conn_handle); //Mark the ID as invalid
  if ( id < 0 ) return; //Non-existant connection, something went wrong, DBG !!!

  prphs[id].conn_handle = BLE_CONN_HANDLE_INVALID; //Mark conn handle as invalid

  for (int i = 0; i < MAX_PERIPHERAL; i++)
  {
    if (Peripheral[i].ID == id)
    {
      Peripheral[i].ID = 255;
    }
  }
  //Serial.print(prphs[id].name);
  //Serial.println(" disconnected!");
}

void CentralBLE::bleuart_rx_callback(BLEClientUart& uart_svc) //Callback invoked when BLE UART data is received
{
  // uart_svc is prphs[conn_handle].bleuart
  uint16_t conn_handle = uart_svc.connHandle();
  int id = findConnHandle(conn_handle);
  prph_info_t* peer = &prphs[id];
  //Serial.printf("[From %s]: ", peer->name); //Print sender's name
  while (uart_svc.available()) //uart_svc Reference object to the service where the data arrived.
  {
    char buf[20 + 1] = { 0 }; //default MTU with an extra byte for string terminator
    if ( uart_svc.read(buf, sizeof(buf) - 1) )
    {
      //Serial.println(buf);
      for (int i = 0; i <= 20; i++)
      {
        Peripheral[id].FIFO.push(buf[i]);
      }
    }
  }
}

int CentralBLE::findConnHandle(uint16_t conn_handle) //Find the connection handle in the peripheral array
{
  for (int id = 0; id < BLE_MAX_CONNECTION; id++)
  {
    if (conn_handle == prphs[id].conn_handle)
    {
      return id; //return array index if found, otherwise -1
    }
  }
  return -1;
}
/**Software Timer callback is invoked via a built-in FreeRTOS thread with
   minimal stack size. Therefore it should be as simple as possible. If
   a periodically heavy task is needed, please use Scheduler.startLoop() to
   create a dedicated task for it.
   More information http://www.freertos.org/RTOS-software-timer.html
*/
void CentralBLE::blink_timer_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;
  // Period of sequence is 10 times (1 second).
  // RED LED will toggle first 2*n times (on/off) and remain off for the rest of period
  // Where n = number of connection
  static uint8_t count = 0;
  if ( count < 2 * connection_num ) digitalToggle(LED_RED);
  if ( count % 2 && digitalRead(LED_RED)) digitalWrite(LED_RED, LOW); // issue #98
  count++;
  if (count >= 10) count = 0;
}

void CentralBLE::print(int ID, char *message)
{
  if ( Bluefruit.Central.connected() ) //First check if we are connected to any peripherals
  {
    prph_info_t* peer = &prphs[ID];
    if ( peer->bleuart.discovered() )
    {
      peer->bleuart.print(message);
    }
  }
}
int CentralBLE::available(int ID)
{
  return Peripheral[ID].FIFO.size();
}
char CentralBLE::read(int ID)
{
  char tmp = Peripheral[ID].FIFO.front();
  Peripheral[ID].FIFO.pop();
  return tmp;
  //return FIFO[ID].dequeue();
}

String CentralBLE::readStringUntil(int ID, char terminator)
{
  String str = "";
  while (1) {
    while (Peripheral[ID].FIFO.size() > 0)
    {
      char tmp = Peripheral[ID].FIFO.front();
      Peripheral[ID].FIFO.pop();
      if (tmp == terminator)
      {
        return str;
      }
      str += String(tmp);
      //delay(50);
    }
    delay(1);
  }
  return "error";
}

bool CentralBLE::isOpen(int device)
{
  if (Peripheral[device].ID != 255)
  {
    return 1;
  }
  return 0;
}

int getPeripheralID(int device)
{
  return Peripheral[device].ID;
}
