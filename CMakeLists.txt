cmake_minimum_required(VERSION 3.13)

#include build functions from SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)

#
project(pcp C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#
set(PICO_BOARD pico_w)

add_compile_options(
	-Wall
#	-Werror
)

pico_sdk_init()

add_executable(pcp
	app.c
	dhcpserver/dhcpserver.c
)

#
#pico_generate_pio_header(${PROJECT_NAME}
#    ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio
#)

pico_enable_stdio_usb(pcp 1)
pico_enable_stdio_uart(pcp 1)

pico_add_extra_outputs(pcp)

target_include_directories(pcp PRIVATE 
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/dhcpserver
	)

target_link_libraries(pcp
  	pico_stdlib 
	pico_util
	# pico_cyw43_arch_lwip_poll
	pico_cyw43_arch_lwip_threadsafe_background
	pico_lwip_http
)


