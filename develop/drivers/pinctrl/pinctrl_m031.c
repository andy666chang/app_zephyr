
#define DT_DRV_COMPAT nuvoton_m031_pinctrl

#include <stdint.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/dt-bindings/pinctrl/m03x-pinctrl.h>
#include <NuMicro.h>

#define MODE_PIN_SHIFT(pin)	((pin) * 2)
#define MODE_MASK(pin)		(3 << MODE_PIN_SHIFT(pin))
#define DINOFF_PIN_SHIFT(pin)	((pin) + 16)
#define DINOFF_MASK(pin)	(1 << DINOFF_PIN_SHIFT(pin))

#define PORT_PIN_MASK		0xFFFF

#define REG_MFP(port, pin) (*(volatile uint32_t *)((uint32_t)DT_INST_REG_ADDR_BY_NAME(0, mfp) + \
						   ((port) * 8) + \
						   ((pin) > 7 ? 4 : 0)))

#define REG_MFOS(port) (*(volatile uint32_t *)((uint32_t)DT_INST_REG_ADDR_BY_NAME(0, mfos) + \
					       ((port) * 4)))

#define MFP_CTL(pin, mfp) ((mfp) << (((pin) % 8) * 4))

/** Utility macro that expands to the GPIO port address if it exists */
#define M031_PORT_ADDR_OR_NONE(nodelabel)			\
	IF_ENABLED(DT_NODE_EXISTS(DT_NODELABEL(nodelabel)),	\
		   (DT_REG_ADDR(DT_NODELABEL(nodelabel)),))

/** Port addresses */
static const uint32_t gpio_port_addrs[] = {
	M031_PORT_ADDR_OR_NONE(gpioa)
	M031_PORT_ADDR_OR_NONE(gpiob)
	M031_PORT_ADDR_OR_NONE(gpioc)
	M031_PORT_ADDR_OR_NONE(gpiod)
	M031_PORT_ADDR_OR_NONE(gpioe)
	M031_PORT_ADDR_OR_NONE(gpiof)
	M031_PORT_ADDR_OR_NONE(gpiog)
	M031_PORT_ADDR_OR_NONE(gpioh)
};

static int gpio_configure(const pinctrl_soc_pin_t *pin)
{
	uint8_t port_idx, pin_idx;
	GPIO_T *port;

	port_idx = M03X_PORT(pin->pinmux);
	if (port_idx >= ARRAY_SIZE(gpio_port_addrs)) {
		return -EINVAL;
	}

	pin_idx = M03X_PIN(pin->pinmux);

	port = (GPIO_T *)gpio_port_addrs[port_idx];


	port->MODE = (port->MODE & ~MODE_MASK(pin_idx)) |
		     ((pin->open_drain ? 2 : 0) << MODE_PIN_SHIFT(pin_idx));
	port->DBEN = (port->DBEN & ~BIT(pin_idx)) |
		     ((pin->input_debounce ? 1 : 0) << pin_idx);
	port->DINOFF = (port->DINOFF & ~DINOFF_MASK(pin_idx)) |
		       ((pin->input_disable ? 1 : 0) << DINOFF_PIN_SHIFT(pin_idx));

	return 0;
}

int pinctrl_configure_pins(const pinctrl_soc_pin_t *pins, uint8_t pin_cnt,
			   uintptr_t reg)
{
	int ret = 0;

	for (uint8_t i = 0U; i < pin_cnt; i++) {
		uint32_t port = M03X_PORT(pins[i].pinmux);
		uint32_t pin = M03X_PIN(pins[i].pinmux);
		uint32_t mfp = M03X_MFP(pins[i].pinmux);

		REG_MFP(port, pin) = (REG_MFP(port, pin) & ~MFP_CTL(pin, 0xf)) |
				     MFP_CTL(pin, mfp);

		if (pins[i].open_drain != 0) {
			REG_MFOS(port) |= BIT(pin);
		} else {
			REG_MFOS(port) &= ~BIT(pin);
		}

		ret = gpio_configure(&pins[i]);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}
