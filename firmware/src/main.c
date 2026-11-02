/*
 * Automatisk vanningssystem – ESP32-S3 TFT + Zephyr RTOS
 *
 * main() starter bare modulene og kjører hovedløkken.
 * All logikk ligger i egne mapper under src/.
 */
#include <zephyr/kernel.h>

#include "config/app_config.h"
#include "core/app_state.h"
#include "sensors/sensors.h"
#include "actuators/pump.h"
#include "control/watering.h"
#include "display/display.h"
#include "network/network.h"

int main(void)
{
	app_state_init();
	pump_init();
	sensors_init();
	watering_init();
	display_init();
	network_start();

	while (1) {
		sensors_update();
		watering_update();
		display_update();
		k_msleep(CONTROL_PERIOD_MS);
	}

	return 0;
}
