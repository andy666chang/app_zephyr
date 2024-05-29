

zephyr_include_directories(include)

if(CONFIG_SOC_SERIES_M03X)
add_subdirectory(drivers)
add_subdirectory(hal)
endif()
