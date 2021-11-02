/**
 * Example detect tags and show their unique ID 
 * Authors: 
 *        Salvador Mendoza - @Netxing - salmg.net
 *        For Electronic Cats - electroniccats.com
 * 
 *  March 2020
 * 
 * This code is beerware; if you see me (or any other collaborator 
 * member) at the local, and you've found our code helpful, 
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 */

#include "Electroniccats_PN7150.h"
#define PN7150_IRQ   (15)
#define PN7150_VEN   (14)
#define PN7150_ADDR  (0x28)

Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR); // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28

unsigned char STATUSOK[] = {0x90, 0x00}, Cmd[256], CmdSize;

uint8_t mode = 2;                                                  // modes: 1 = Reader/ Writer, 2 = Emulation

void setup(){
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Detect NFC readers with PN7150");
  Serial.println("Initializing..."); 

  if (nfc.connectNCI()) { //Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }
  
  if (nfc.ConfigureSettings()) {
    Serial.println("The Configure Settings is failed!");
    while (1);
  }
  
  if(nfc.ConfigMode(mode)){ //Set up the configuration mode
    Serial.println("The Configure Mode is failed!!");
    while (1);
  }
  nfc.StartDiscovery(mode); //NCI Discovery mode
  Serial.println("Waiting for an Reader Card ...");
}

void loop(){
  if(nfc.CardModeReceive(Cmd, &CmdSize) == 0) { //Data in buffer?
      if ((CmdSize >= 2) && (Cmd[0] == 0x00)) { //Expect at least two bytes
          switch (Cmd[1]) {
              case 0xA4: //Something tries to select a file, meaning that it is a reader
                  Serial.println("Reader detected!");
                  break;
  
              case 0xB0: //SFI
                  break;
  
              case 0xD0: //...
                  break;
  
              default:
                  break;
          }
          nfc.CardModeSend(STATUSOK, sizeof(STATUSOK));
      }
  }
}
