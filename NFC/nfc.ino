// esp32-wroom-32 devkit v1 default pins
//       SCL D22
//       SDA D21

// Define the interface type
// #if 0
// #include <SPI.h>
// #include <PN532_SPI.h>
// #include "PN532.h"
// PN532_SPI pn532spi(SPI, 10);
// PN532 nfc(pn532spi);

// #elif 0
// #include <PN532_HSU.h>
// #include <PN532.h>
// PN532_HSU pn532hsu(Serial1);
// PN532 nfc(pn532hsu);

// #else
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
// #endif

volatile bool connected = false;

void setup(void)
{
  pinMode(2, OUTPUT); 
  Serial.begin(115200);
  Wire.begin(48, 47);
  Serial.println("*** Testing Module PN532 NFC RFID ***");
  // char id_now[][20] = {""}
  char all_id[20] = "0x23 0xE2 0x2 0x13";
  char now_id[20] ;
}

void loop()
{
  digitalWrite(2,HIGH);
  boolean success;
  // Buffer to store the UID
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  // UID size (4 or 7 bytes depending on card type)
  uint8_t uidLength;

  while (!connected) {
    connected = connect();
  }

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  Serial.println(success);

  // If the card is detected, print the UID
  if (success)
  {
    Serial.println("Card Detected");
    Serial.print("Size of UID: "); Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID: ");
    
    for (uint8_t i = 0; i < uidLength; i++)
    {
      Serial.print(" "); Serial.print(uid[i], HEX);
    }
    Serial.println("");
    Serial.println("");
    
    delay(1000);
    connected = connect();
  }
  else
  {
    // PN532 probably timed out waiting for a card
    // Serial.println("Timed out waiting for a card");
  }
}

bool connect() {
  
  nfc.begin();

  // Connected, show version
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata)
  {
    Serial.println("PN53x card not found!");
    return false;
  }

  //port
  Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware version: "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for card (ISO14443A Mifare)...");
  Serial.println("");

  return true;
}