#define PIN_OLED_CS 8
#define PIN_OLED_RST 19
#define PIN_OLED_DC 10

void oled_init(void);
void oled_display(void);
void oled_clear(void);
void oled_print_char(int row, int col, char ch, char colour);
void oled_print_line(int row, char* string, char colour);
