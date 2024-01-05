#ifndef PeripheralBLE_h
#define PeripheralBLE_h
#include "Arduino.h"
#include <bluefruit.h>

class PeripheralBLE : public BLEUart {
  public:
    void begin(char *name);
    static void connect_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
    bool isOpen();
  private:
};

static bool isopen = 0;

#endif
