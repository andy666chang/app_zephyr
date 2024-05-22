
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_CLOCK_CTRL_M03X clock_m031.c)
