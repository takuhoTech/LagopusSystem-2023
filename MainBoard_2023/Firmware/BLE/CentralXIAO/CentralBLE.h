/* Connection Handle Explanation
   The total number of connections is BLE_MAX_CONNECTION (20)
   The 'connection handle' is an integer number assigned by the SoftDevice
   (Nordic's proprietary BLE stack). Each connection will receive it's own
   numeric 'handle' starting from 0 to BLE_MAX_CONNECTION-1, depending on the order
   of connection(s).

   - E.g If our Central board connects to a mobile phone first (running as a peripheral),
   then afterwards connects to another Bluefruit board running in peripheral mode, then
   the connection handle of mobile phone is 0, and the handle for the Bluefruit
   board is 1, and so on.

   LED PATTERNS
   LED_RED   - Blinks pattern changes based on the number of connections.
   LED_BLUE  - Blinks constantly when scanning*/
#ifndef CentralBLE_h
#define CentralBLE_h
#include "Arduino.h"
#include <bluefruit.h>
#include <queue>

/* Peripheral info array (one per peripheral device)
   There are 'BLE_MAX_CONNECTION' central connections, but the
   the connection handle can be numerically larger (for example if
   the peripheral role is also used, such as connecting to a mobile
   device). As such, we need to convert connection handles <-> the array
   index where appropriate to prevent out of array accesses.

   Note: One can simply declares the array with BLE_MAX_CONNECTION and use connection
   handle as index directly with the expense of SRAM.
*/

class CentralBLE
{
  public:
    void addPrphName(char *name);
    void begin(char *name);
    static void scan_callback(ble_gap_evt_adv_report_t* report);
    static void connect_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
    static void bleuart_rx_callback(BLEClientUart& uart_svc);
    static int findConnHandle(uint16_t conn_handle);
    static void blink_timer_callback(TimerHandle_t xTimerID);

    void print(int ID, char *message);
    int available(int ID);
    char read(int ID);
    String readStringUntil(int ID, char terminator);

    bool isOpen(int device);
};

typedef struct //Struct containing peripheral info
{
  char name[16 + 1];
  uint16_t conn_handle;
  BLEClientUart bleuart; //Each prph need its own bleuart client service
} prph_info_t;
static prph_info_t prphs[BLE_MAX_CONNECTION];
static SoftwareTimer blinkTimer; //Software Timer for blinking the RED LED
static uint8_t connection_num = 0; //for blink pattern

typedef struct
{
  std::queue<char> FIFO; //bleuart_rx_callbackの中でFIFO[受信したID]に受信内容を書き込む　FIFO[0]がAirMeterとは限らない
  uint8_t ID; //connect_callbackの中で、Peripheral[0].IDにAirMeterのIDを、Peripheral[1].IDにPowerMeterのIDを入れる
  String Name;
} prph_info;
#define MAX_PERIPHERAL 2 //this can be increased
static prph_info Peripheral[MAX_PERIPHERAL];

//enum DEVICE {AirMeter, PowerMeter}; //AirMeter=0,PowerMeter=1
int getPeripheralID(int device);

#endif
