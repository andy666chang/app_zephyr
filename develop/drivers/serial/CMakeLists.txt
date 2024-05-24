
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_UART_M031 uart_m031.c)
