#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define RED_GPIO       42
#define YELLOW_GPIO    41
#define GREEN_GPIO     40

#define TOPIC_LIGHT    TOPIC_PREFIX "/light"
#define TOPIC_LED_RED  TOPIC_PREFIX "/led/red"
#define TOPIC_LOCKING_VALUE  TOPIC_PREFIX "/locking"
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
volatile bool connected = false;

WiFiClient wifiClient;
PubSubClient mqtt(MQTT_BROKER, 1883, wifiClient);
uint32_t last_publish;


void connect_wifi() {
  printf("WiFi MAC address is %s\n", WiFi.macAddress().c_str());
  printf("Connecting to WiFi %s.\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    printf(".");
    fflush(stdout);
    delay(500);
  }
  printf("\nWiFi connected.\n");
}


void connect_mqtt() {
  printf("Connecting to MQTT broker at %s.\n", MQTT_BROKER);
  if (!mqtt.connect("", MQTT_USER, MQTT_PASS)) {
    printf("Failed to connect to MQTT broker.\n");
    for (;;) {} // wait here forever
  }
  mqtt.setCallback(mqtt_callback);
  mqtt.subscribe(TOPIC_LED_RED);
  printf("MQTT broker connected.\n");
}


void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_LED_RED) == 0) {
    payload[length] = 0; // null-terminate the payload to treat it as a string
    int value = atoi((char*)payload); 
    if (value == 0) {
      digitalWrite(RED_GPIO, LOW);
    }
    else if (value == 1) {
      digitalWrite(RED_GPIO, HIGH);
    }
    else {
      printf("Invalid payload received.\n");
    }
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
bool check_id(uint8_t now[]){
  uint8_t id_list[2][4]={{35 ,226 ,2 ,19},{179,90,89,252}};
  bool ans = 0 ;
  for (uint8_t i = 0 ; i<2;i++)
  {
    for (uint8_t j = 0 ; j<4;i++){
      if (now[j] != id_list[i][j]){
        break;
      }
      ans = 1;
      
    }
    if (ans){
      return true ;
    }

  }
  return false ;
}

void setup() {
  pinMode(RED_GPIO, OUTPUT);
  pinMode(GREEN_GPIO, OUTPUT);
  digitalWrite(RED_GPIO, 0);
  connect_wifi();
  connect_mqtt();
  last_publish = 0;
  //NFC part
  Serial.begin(115200);
  Wire.begin(48, 47);
  Serial.println("*** Testing Module PN532 NFC RFID ***");
  char all_id[20] = "0x23 0xE2 0x2 0x13";
  char now_id[20] ;
}

void loop() {
  // check for incoming subscribed topics
  mqtt.loop();


  // publish light value periodically (without using delay)
  uint32_t now = millis();
  // if (now - last_publish >= 2000) {
  //   // int level = 100 - (analogRead(LDR_GPIO)*100/4095);
  //   String payload(level);
  //   printf("Publishing light value: %d\n", level);
  //   mqtt.publish(TOPIC_LIGHT, payload.c_str());
  //   last_publish = now;

  boolean success;
  // Buffer to store the UID
  uint8_t uid[] = { 0, 0, 0, 0};
  // UID size (4 or 7 bytes depending on card type)
  uint8_t uidLength;

  while (!connected) {
    connected = connect();
  }

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  Serial.println(success);
  if (success)
  {
    Serial.println("Card Detected");
    Serial.print("Size of UID: "); Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID: ");
    Serial.println("");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      Serial.print(" "); Serial.print(uid[i]);
    }
    if (check_id(uid)){
      digitalWrite(GREEN_GPIO,HIGH);
      digitalWrite(RED_GPIO,LOW);  
      Serial.println("");
      Serial.println("Unlock");
      String payload("0");
      mqtt.publish(TOPIC_LIGHT, payload.c_str());
      // mqtt.publish(TOPIC_LOCKING_VALUE, "0"); 
      last_publish = now; 
    }
    else{
      digitalWrite(RED_GPIO,HIGH);
      digitalWrite(GREEN_GPIO,LOW);
      Serial.println("");
      Serial.println("Lock");
      String payload("1");
      mqtt.publish(TOPIC_LIGHT, payload.c_str());
      // mqtt.publish(TOPIC_LOCKING_VALUE, "1");
      last_publish = now;
    }
    Serial.println("");
    // Serial.println("");
    
    delay(1000);
    connected = connect();
  }
  else
  {
    // digitalWrite(RED_GPIO,LOW);
    // PN532 probably timed out waiting for a card
    // Serial.println("Timed out waiting for a card");
  }
}
