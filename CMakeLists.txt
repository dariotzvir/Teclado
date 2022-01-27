cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(Teclado C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(Teclado Teclado.cpp)

target_sources(Teclado PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Teclado.cpp
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
)
target_include_directories(Teclado PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(Teclado PUBLIC pico_stdlib tinyusb_device tinyusb_board)
pico_add_extra_outputs(Teclado)

set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")

