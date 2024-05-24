

zephyr_library()

if(CONFIG_CPU_CORTEX_M4)
  zephyr_compile_definitions( -DCORE_CM4 )
elseif(CONFIG_CPU_CORTEX_M7)
  zephyr_compile_definitions( -DCORE_CM7 )
endif()

if(CONFIG_M031_TD)
  zephyr_compile_definitions(M031_TD)
elseif(CONFIG_M031_LE)
  zephyr_compile_definitions(M031_LE)
endif()

if(CONFIG_SOC_SERIES_M03X)
  set(SERIES_NAME_L M031BSP)
  set(STD_PERIPH_DRV_DIR StdDriver)
else()
  message(ERROR "Invalid SERIES")
endif()

# zephyr_include_directories(common)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)

zephyr_include_directories(
  ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/inc
  ${SERIES_NAME_L}/Library/Device/Nuvoton/M031/Include
)

zephyr_library_sources(${SERIES_NAME_L}/Library/Device/Nuvoton/M031/Source/system_M031Series.c)
# zephyr_library_sources(${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/${SERIES_NAME_L}_rcm.c)
# zephyr_library_sources(${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/${SERIES_NAME_L}_misc.c)


zephyr_library_sources(${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/sys.c)
zephyr_library_sources(${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/clk.c)

zephyr_library_sources_ifdef(CONFIG_M031_ADC     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/adc.c)
zephyr_library_sources_ifdef(CONFIG_M031_CRC     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/crc.c)
zephyr_library_sources_ifdef(CONFIG_M031_EINT    ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/eint.c)
zephyr_library_sources_ifdef(CONFIG_M031_FMC     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/fmc.c)
zephyr_library_sources_ifdef(CONFIG_M031_GPIO    ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/gpio.c)
zephyr_library_sources_ifdef(CONFIG_M031_I2C     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/i2c.c)
zephyr_library_sources_ifdef(CONFIG_M031_SPI     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/spi.c)
zephyr_library_sources_ifdef(CONFIG_M031_TMR     ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/timer.c)
zephyr_library_sources_ifdef(CONFIG_M031_USART   ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/uart.c)
zephyr_library_sources_ifdef(CONFIG_M031_WWDT    ${SERIES_NAME_L}/Library/${STD_PERIPH_DRV_DIR}/src/wwdt.c)

# acmp.c
# bpwm.c
# ebi.c
# pdma.c
# pwm.c
# qspi.c
# retarget.c
# rtc.c
# usbd.c
# usci_i2c.c
# usci_spi.c
# usci_uart.c
# wdt.c
