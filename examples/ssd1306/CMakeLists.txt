# Example project for pal

add_executable(ssd1306 ssd1306.cpp)

pico_set_program_name(ssd1306 "SSD1306 Example")
pico_set_program_version(ssd1306 "0.1")

pico_enable_stdio_uart(ssd1306 1)
pico_enable_stdio_usb(ssd1306 0)

# Add the libraries we'll need
target_link_libraries(ssd1306
        pico_stdlib
        hardware_i2c
        pal-ssd1306
        )

pico_add_extra_outputs(ssd1306)
