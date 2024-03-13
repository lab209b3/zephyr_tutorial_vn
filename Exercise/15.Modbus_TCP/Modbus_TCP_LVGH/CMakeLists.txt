cmake_minimum_required(VERSION 3.22)
set(BOARD stm32f746g_disco)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
find_package(Zephyr)
project(MODBUS)

target_sources(app PRIVATE src/main.c)

