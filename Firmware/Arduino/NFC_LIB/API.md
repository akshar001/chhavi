# Electroniccats_PN7150 API Reference

The `Electroniccats_PN7150` class enables Arduino library for I2C access to the PN7150 RFID/Near Field Communication chip.

### Class: `Electroniccats_PN7150`

Include and instantiate the Electroniccats_PN7150 class. Creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28

```c
#include <Electroniccats_PN7150.h>

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR);

```

- `uint8_t PN7150_IRQ`: IRQ pin for data interrupt.
- `uint8_t PN7150_VEN`: "Reset" or "Enable" pin for device.
- `uint8_t PN7150_ADDR`: Hexadecimal address for device, default `0x28` .

### Example

```c

#include "Electroniccats_PN7150.h"
#define PN7150_IRQ   (8)
#define PN7150_VEN   (7)
#define PN7150_ADDR  (0x28)

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR); // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28

void setup(){ 
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Detect NFC readers with PN7150");
  uint8_t statusNFC = setupNFC();
  if (!statusNFC) 
    Serial.println("Set up is ok");
  else
    Serial.println("Error while setting up mode, check connections!");
}

int setupNFC(){
  Serial.println("Initializing...");
  int setupOK = nfc.connectNCI();                     //Wake up the board
  if (!setupOK){
    setupOK = nfc.ConfigMode(mode);                   //Set up the configuration mode
    if (!setupOK) setupOK = nfc.StartDiscovery(mode); //NCI Discovery mode
  }
  return setupOK;
}
```
### Method: `GetFwVersion`

Get Firmware Version.

```c
uint8_t GetFwVersion();
```

### Method: `ConfigMode`

Configure Mode for device

- `1`: RW Mode.
- `2`: Emulation mode.

```c
uint8_t Electroniccats_PN7150::ConfigMode(uint8_t modeSE)
```

### Method: `StartDiscovery`

Returns .

```c
uint8_t StartDiscovery(mode);
```

### Method: `CardModeReceive`

Returns .

```c
bool CardModeReceive (unsigned char *pData unsigned char *pDataSize);	
```

### Method: `CardModeSend`

Return.

```c
bool CardModeSend (unsigned char *pData, unsigned char DataSize);
```

### Method: `ReaderActivateNext`

Return.

```c
bool ReaderActivateNext(RfIntf_t *pRfIntf);		
```

### Method: `WaitForDiscoveryNotification`

Return.

```c
bool WaitForDiscoveryNotification(RfIntf_t *pRfIntf);
```


### Method: `ProcessReaderMode`

Return.

```c
void ProcessReaderMode(RfIntf_t RfIntf, RW_Operation_t Operation);
```

### Methods: `StopDiscovery`

Return.

```c
bool StopDiscovery(void);
```