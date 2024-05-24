
zephyr_library()

zephyr_library_sources_ifdef(CONFIG_PINCTRL_M031 pinctrl_m031.c)
