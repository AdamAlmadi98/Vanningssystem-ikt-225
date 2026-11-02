#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "sensors/water_level.h"

static const struct gpio_dt_spec level = GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), water_level_gpios);
static bool ready;

int water_level_init(void)
{
	if (!gpio_is_ready_dt(&level)) {
		return -ENODEV;
	}
	int err = gpio_pin_configure_dt(&level, GPIO_INPUT);
	if (err) {
		return err;
	}
	ready = true;
	return 0;
}

int water_level_read(void)
{
	if (!ready) {
		return -ENODEV;
	}
	return gpio_pin_get_dt(&level);
}
