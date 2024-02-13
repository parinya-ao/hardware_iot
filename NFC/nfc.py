from machine import Pin, I2C, UART
import pn532
import time

# pin
i2c = I2C(scl=Pin(5), sda=Pin(4))
pn532 = pn532.PN532_I2C(i2c) #pin from libary pn532
nfc = pn532.PN532(pn532) # pin nfc
connected = False

### setup function
pin_2 = Pin(2, Pin.OUT) # create pin
uart = UART(0, baudrate=115200) #ส่งลงช่อง 115200 baud ใน arudino
uart.init(115200, bits=8, parity=None, stop=1) #

i2c = I2C(scl=Pin(48), sda=Pin(47)) #set i2c

###

### loop function
uid = bytearray(7) # uid เลขฐาน 16

while (1):
    pint_2.value(1) #เปิดหลอดไฟ
    
    while not connected:
        connected = connect()
    
    success, uidLength = nfc.read_passive_target_id(PN532_MIFARE_ISO14443A, uid)

    if success:
        print("Card Detected")
        print("Size of UID: {} bytes".format(uidLength))
        print("UID:", " ".join(["{:02X}".format(byte) for byte in uid[:uidLength]]))
        print("")

        time.sleep(1)
        connected = connect()
    else:
        # PN532 probably timed out waiting for a card
        # print("Timed out waiting for a card")
        pass
###

### function connect
def connect():
    nfc.begin()
    
    versiondata = nfc.get_firmware_version() #check version firmware
    if not versiondata: # หาไม่เจอ
        print("PN53x card not found! check port!!!!!!!")
        return False

    print("Found chip PN5{:02X}".format((versiondata >> 24) & 0xFF))
    print("Firmware version: {}.{}".format((versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF))
    
    nfc.set_passive_activation_retries(0xFF)
    nfc.SAMConfig()

    print("Waiting for card (ISO14443A Mifare)...")
    print("")

    return True