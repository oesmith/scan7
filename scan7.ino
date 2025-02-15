#include <Adafruit_SSD1327.h>
#include "mbe.h"

#include "Mx437_Amstrad_PC4pt7b.h"

#define OLED_PIN_CS (8)
#define OLED_PIN_DC (10)
#define OLED_PIN_RST (19)

Adafruit_SSD1327 OLED(128, 128, &SPI, OLED_PIN_DC, OLED_PIN_RST, OLED_PIN_CS);

void showMsg(const char* msg) {
  OLED.clearDisplay();
  OLED.setCursor(0, 6);
  OLED.print(msg);
  OLED.display();
}

void setup() {
  // Pull all the SPI CS pins high.
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);

  OLED.begin();
  showMsg("Initialising...");

  if (mbe_init() != MBE_OK) {
    showMsg("CAN BUS FAIL");
    return;
  }
  showMsg("CAN BUS OK");
  
  delay(3000);

  char ver[256];
  mbe_error err = mbe_version(ver, 256);
  if (err != MBE_OK) {
    OLED.clearDisplay();
    OLED.setCursor(0, 6);
    OLED.println("Version failed");
    OLED.println(err);
    OLED.display();
    return;
  }
  showMsg(ver);
}

void loop() {
  /* if (CAN.checkReceive() == CAN_MSGAVAIL) { */
  /*   uint8_t len; */
  /*   uint8_t buf[8]; */
  /*   CAN.readMsgBuf(&len, buf); */
  /*   for (uint8_t i = 0; i < len; i++) { */
  /*     OLED.printf("%02x", buf[i]); */
  /*   } */
  /*   OLED.printf("\n"); */
  /*   OLED.display(); */
  /* } */
  delay(100);
}
