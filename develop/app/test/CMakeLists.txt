# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.20.0)
# set(ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../../)

find_package(Zephyr)
project(test)

# Add your source file to the "app" target. This must come after
# find_package(Zephyr) which defines the target.
target_sources(app PRIVATE src/main.c)

