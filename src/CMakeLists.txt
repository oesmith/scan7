add_executable(scan7 main.c oled.c font.c can.c)

pico_enable_stdio_usb(scan7 1)
pico_enable_stdio_uart(scan7 0)
pico_add_extra_outputs(scan7)

target_link_libraries(scan7
  pico_stdlib
  hardware_spi
)
