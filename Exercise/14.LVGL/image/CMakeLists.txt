# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(BOARD stm32f746g_disco)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl)


target_sources(app PRIVATE main.c)
target_sources(app PRIVATE zephyr_logo.c)