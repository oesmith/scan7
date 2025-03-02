#include <Adafruit_SSD1327.h>
#include "font.h"
#include "mbe.h"
#include "poll.h"

#define OLED_PIN_CS (8)
#define OLED_PIN_DC (10)
#define OLED_PIN_RST (19)
#define BUTTON_PIN (24)
#define VERSION F("v0.1.0")

Adafruit_SSD1327 OLED(128, 128, &SPI, OLED_PIN_DC, OLED_PIN_RST, OLED_PIN_CS);

void setup() {
  // Pull all the SPI CS pins high.
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  digitalWrite(8, HIGH);
  digitalWrite(9, HIGH);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  OLED.begin();
  OLED.setFont(&Mx437_Amstrad_PC4pt7b);
  OLED.clearDisplay();
  OLED.drawRect(0, 0, 128, 128, 0x8888);
  OLED.drawRect(2, 2, 124, 124, 0x8888);
  OLED.setCursor(8, 20);
  OLED.print("SCAN 7");
  OLED.setCursor(8, 40);
  OLED.print("olly" "@olly" ".xyz");

  OLED.setCursor(8, 60);
  OLED.print(VERSION);
  OLED.setCursor(8, 70);
  OLED.print(__DATE__);

  OLED.display();

  OLED.setCursor(8, 110);

  delay(1000);


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
}

#define UPDATE_INTERVAL_MILLIS 100
#define DEBOUNCE_MILLIS 50
#define NUM_PAGES 4

static long last_update_millis = 0;
static long last_debounce_millis = 0;
static int page = 0;
static int button_state = HIGH;
static int last_button_state = HIGH;

void loop() {
  read_page_button();
  update();
}

void read_page_button() {
  int pin_state = digitalRead(BUTTON_PIN);
  int t = millis();

  if (pin_state != last_button_state) {
    last_debounce_millis = t;
  }

  if ((t - last_debounce_millis) > DEBOUNCE_MILLIS) {
    if (pin_state != button_state) {
      button_state = pin_state;

      if (button_state == HIGH) {
        page = (page + 1) % NUM_PAGES;
      }
    }
  }

  last_button_state = pin_state;
}

void update() {
  long t = millis();
  if (t - last_update_millis < UPDATE_INTERVAL_MILLIS) {
    return;
  }
  last_update_millis = t;

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

  switch (page) {
    case 0:
      draw_page_1(status);
      break;
    case 1:
      draw_page_2(status);
      break;
    case 2:
      draw_page_3(status);
      break;
    case 3:
      draw_page_4(status);
      break;
  }

  OLED.display();
}

void draw_page_1(status_t status) {
  OLED.setCursor(0, 0);
  OLED.print("MAIN        *...");

  OLED.drawRect(0, 10, 62, 26, 0x8888);
  OLED.setCursor(4, 14);
  OLED.print("RPM");
  OLED.setCursor(4, 24);
  OLED.printf("%d", status.engine_rpm);

  OLED.drawRect(66, 10, 62, 26, 0x8888);
  OLED.setCursor(70, 14);
  OLED.print("BAT");
  OLED.setCursor(70, 24);
  OLED.printf("%.1fv", status.battery_volts);

  OLED.drawRect(0, 40, 62, 26, 0x8888);
  OLED.setCursor(4, 44);
  OLED.print("WATER");
  OLED.setCursor(4, 54);
  OLED.printf("%.1fc", status.coolant_temp_c);

  OLED.drawRect(66, 40, 62, 26, 0x8888);
  OLED.setCursor(70, 44);
  OLED.print("INTAKE");
  OLED.setCursor(70, 54);
  OLED.printf("%.1fc", status.air_temp_c);

  OLED.drawRect(0, 70, 62, 26, 0x8888);
  OLED.setCursor(4, 74);
  OLED.print("T.IDLE");
  OLED.setCursor(4, 84);
  OLED.printf("%d", status.target_idle_rpm);

  OLED.drawRect(66, 70, 62, 26, 0x8888);
  OLED.setCursor(70, 74);
  OLED.print("BARO");
  OLED.setCursor(70, 84);
  OLED.printf("%.1fb", status.baro_pressure_bar);

  OLED.drawRect(0, 100, 128, 26, 0x8888);
  OLED.setCursor(4, 104);
  OLED.print("TPS");
  OLED.setCursor(4, 114);
  OLED.printf("%.1f", status.throttle_site);
  OLED.setCursor(70, 114);
  OLED.printf("%.1fv", status.throttle_angle_volts);

}

void draw_page_2(status_t status) {
  OLED.setCursor(0, 0);
  OLED.print("LAMBDA      .*..");

  OLED.drawRect(0, 10, 128, 26, 0x8888);
  OLED.setCursor(4, 14);
  OLED.printf("SENSOR");
  OLED.setCursor(4, 24);
  OLED.printf("%.1f",status.current_lambda);
  OLED.setCursor(70, 24);
  OLED.printf("%.1fv", status.lambda_volts);

  OLED.drawRect(0, 40, 62, 26, 0x8888);
  OLED.setCursor(4, 44);
  OLED.print("TARGET");
  OLED.setCursor(4, 54);
  OLED.printf("%.1f", status.target_lambda);

  OLED.drawRect(66, 40, 62, 26, 0x8888);
  OLED.setCursor(70, 44);
  OLED.print("TRIM");
  OLED.setCursor(70, 54);
  OLED.printf("%.1f%%", status.lambda_trim_percent);

  OLED.drawRect(0, 70, 62, 26, 0x8888);
  OLED.setCursor(4, 74);
  OLED.print("STATUS");
  OLED.setCursor(4, 84);
  OLED.printf("%04x", status.lambda_status_flags);
}

void draw_page_3(status_t status) {
  OLED.setCursor(0, 0);
  OLED.print("CODES       ..*.");

  OLED.drawRect(0, 10, 62, 26, 0x8888);
  OLED.setCursor(4, 14);
  OLED.print("FAULTA");
  OLED.setCursor(4, 24);
  OLED.printf("%04x", status.current_faults_a_flags);

  OLED.drawRect(66, 10, 62, 26, 0x8888);
  OLED.setCursor(70, 14);
  OLED.print("FAULTB");
  OLED.setCursor(70, 24);
  OLED.printf("%04x", status.current_faults_b_flags);

  OLED.drawRect(0, 40, 62, 26, 0x8888);
  OLED.setCursor(4, 44);
  OLED.print("FAULTC");
  OLED.setCursor(4, 54);
  OLED.printf("%04x", status.current_faults_c_flags);

  OLED.drawRect(66, 40, 62, 26, 0x8888);
  OLED.setCursor(70, 44);
  OLED.print("FAULTD");
  OLED.setCursor(70, 54);
  OLED.printf("%04x", status.current_faults_d_flags);

  OLED.drawRect(0, 70, 62, 26, 0x8888);
  OLED.setCursor(4, 74);
  OLED.print("E.SYNC");
  OLED.setCursor(4, 84);
  OLED.printf("%02x", status.engine_synch_status_flags);

  OLED.drawRect(66, 70, 62, 26, 0x8888);
  OLED.setCursor(70, 74);
  OLED.print("IDLE");
  OLED.setCursor(70, 84);
  OLED.printf("%02x", status.idlespeed_status_flags);
}

void draw_page_4(status_t status) {
  OLED.setCursor(0, 0);
  OLED.print("EXTRA       ...*");

  OLED.drawRect(0, 10, 62, 26, 0x8888);
  OLED.setCursor(4, 14);
  OLED.print("S.CUT");
  OLED.setCursor(4, 24);
  OLED.printf("%d", status.soft_cut_rpm);

  OLED.drawRect(66, 10, 62, 26, 0x8888);
  OLED.setCursor(70, 14);
  OLED.print("H.CUT");
  OLED.setCursor(70, 24);
  OLED.printf("%d", status.hard_cut_rpm);

  OLED.drawRect(0, 40, 62, 26, 0x8888);
  OLED.setCursor(4, 44);
  OLED.print("WARMUP");
  OLED.setCursor(4, 54);
  OLED.printf("%.0fs", status.warm_up_timer_s);
}
