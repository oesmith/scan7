#include <Adafruit_SSD1327.h>
#include "mbe.h"
#include "poll.h"

#include "Mx437_Amstrad_PC4pt7b.h"

#define OLED_PIN_CS (8)
#define OLED_PIN_DC (10)
#define OLED_PIN_RST (19)

Adafruit_SSD1327 OLED(128, 128, &SPI, OLED_PIN_DC, OLED_PIN_RST, OLED_PIN_CS);

bool init_ok = false;

void setup() {
  // Pull all the SPI CS pins high.
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);

  OLED.begin();
  OLED.setFont(&Mx437_Amstrad_PC4pt7b);
  OLED.clearDisplay();
  OLED.drawRect(0, 0, 128, 128, 0xffff);
  OLED.drawRect(2, 2, 124, 124, 0xffff);
  OLED.setCursor(42, 60);
  OLED.print("SCAN 7");
  OLED.display();

  delay(1000);

  OLED.setCursor(8, 110);

  if (mbe_init() != MBE_OK) {
    OLED.println("CAN BUS FAIL");
    OLED.display();
    while(1) {
      delay(1000);
    }
    return;
  }

  char ver[32];
  mbe_error err = mbe_version(ver, 32);
  if (err != MBE_OK) {
    OLED.print(mbe_error_text(err));
    OLED.display();
    while(1) {
      delay(1000);
    }
    return;
  }

  OLED.printf("ECU %s", ver);
  OLED.display();
  delay(3000);

  init_ok = true;
}

void loop() {
  OLED.clearDisplay();
  OLED.setCursor(0, 0);

  status_t status;
  mbe_error err = poll_ecu(&status);
  if (err != MBE_OK) {
    OLED.println("Poll failed");
    OLED.println("");
    OLED.println(mbe_error_text(err));
    OLED.display();
    delay(1000);
    return;
  }

  OLED.drawRect(0, 0, 62, 26, 0xffff);
  OLED.setCursor(4, 4);
  OLED.print("RPM");
  OLED.setCursor(4, 14);
  OLED.printf("%d", status.engine_rpm);

  OLED.drawRect(66, 0, 62, 26, 0xffff);
  OLED.setCursor(70, 4);
  OLED.print("BAT");
  OLED.setCursor(70, 14);
  OLED.printf("%.1fv", status.battery_volts);

  OLED.drawRect(0, 30, 62, 26, 0xffff);
  OLED.setCursor(4, 34);
  OLED.print("WATER");
  OLED.setCursor(4, 44);
  OLED.printf("%.1fc", status.coolant_temp_c);

  OLED.drawRect(66, 30, 62, 26, 0xffff);
  OLED.setCursor(70, 34);
  OLED.print("INTAKE");
  OLED.setCursor(70, 44);
  OLED.printf("%.1fc", status.air_temp_c);

  OLED.drawRect(0, 60, 62, 36, 0xffff);
  OLED.setCursor(4, 64);
  OLED.print("TPS");
  OLED.setCursor(4, 74);
  OLED.printf("%.1f", status.throttle_site);
  OLED.setCursor(4, 84);
  OLED.printf("%.1fv", status.throttle_angle_volts);

  OLED.drawRect(66, 60, 62, 36, 0xffff);
  OLED.setCursor(70, 64);
  OLED.printf("LAMBDA");
  OLED.setCursor(70, 74);
  OLED.printf("%.1f",status.current_lambda);
  OLED.setCursor(70, 84);
  OLED.printf("%.1fv", status.lambda_volts);

  OLED.drawRect(0, 100, 128, 26, 0xffff);
  OLED.setCursor(4, 104);
  OLED.print("ERROR CODES");
  OLED.setCursor(4, 114);
  OLED.printf("%04x %04x %04x", status.current_faults_a_flags, status.current_faults_b_flags, status.current_faults_c_flags);

  OLED.display();

  delay(200);
}
