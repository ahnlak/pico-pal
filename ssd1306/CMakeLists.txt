# Each element of pico-pal is defined as a distinct INTERFACE library,
# so that we minimise the amount of code linked in to the final executable.

# Library name
set(PAL_LIB_NAME pal-ssd1306)

# Everything else is semi-automagic
add_library(${PAL_LIB_NAME} INTERFACE)
target_sources(${PAL_LIB_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/${PAL_LIB_NAME}.cpp)
target_include_directories(${PAL_LIB_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR})
