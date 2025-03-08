
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_FLASH_M031 flash_m031.c)
