
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_GPIO_M031 gpio_m031.c)
