#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "actuators/pump.h"

LOG_MODULE_REGISTER(pump, LOG_LEVEL_INF);

#define ZEPHYR_USER DT_PATH(zephyr_user)

static const struct gpio_dt_spec in1  = GPIO_DT_SPEC_GET(ZEPHYR_USER, pump_in1_gpios);
static const struct gpio_dt_spec in2  = GPIO_DT_SPEC_GET(ZEPHYR_USER, pump_in2_gpios);
static const struct gpio_dt_spec pwm  = GPIO_DT_SPEC_GET(ZEPHYR_USER, pump_pwm_gpios);
static const struct gpio_dt_spec stby = GPIO_DT_SPEC_GET(ZEPHYR_USER, pump_stby_gpios);

static const struct gpio_dt_spec *const pins[] = { &in1, &in2, &pwm, &stby };
static bool ready;

int pump_init(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(pins); i++) {
		if (!gpio_is_ready_dt(pins[i])) {
			LOG_ERR("Pumpe-GPIO ikke klar");
			return -ENODEV;
		}
		/* Alle pinner starter lave = pumpen er av */
		int err = gpio_pin_configure_dt(pins[i], GPIO_OUTPUT_INACTIVE);
		if (err) {
			return err;
		}
	}
	ready = true;
	return 0;
}

void pump_set(bool on)
{
	if (!ready) {
		return;
	}

	if (on) {
		/* Går pumpen feil vei: bytt verdiene for in1 og in2 */
		gpio_pin_set_dt(&stby, 1);
		gpio_pin_set_dt(&in1, 1);
		gpio_pin_set_dt(&in2, 0);
		gpio_pin_set_dt(&pwm, 1);
	} else {
		gpio_pin_set_dt(&pwm, 0);
		gpio_pin_set_dt(&in1, 0);
		gpio_pin_set_dt(&in2, 0);
		gpio_pin_set_dt(&stby, 0);
	}
	LOG_INF("Pumpe %s", on ? "PÅ" : "AV");
}
