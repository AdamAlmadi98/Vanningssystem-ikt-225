#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sensors/sensors.h"
#include "sensors/soil_sensor.h"
#include "sensors/water_level.h"
#include "sensors/aht20.h"
#include "core/app_state.h"

LOG_MODULE_REGISTER(sensors, LOG_LEVEL_INF);

void sensors_init(void)
{
	if (soil_sensor_init() != 0) {
		LOG_ERR("Jordfuktighetssensor kunne ikke startes");
	}
	if (water_level_init() != 0) {
		LOG_ERR("Vannivåsensor kunne ikke startes");
	}
	if (aht20_init() != 0) {
		LOG_WRN("AHT20 ikke funnet (prøver igjen senere)");
	}
}

void sensors_update(void)
{
	int soil_pct = -1, soil_raw = 0;
	int temp_c10 = 0, hum_pct10 = 0;

	bool soil_ok = soil_sensor_read(&soil_pct, &soil_raw) == 0;
	int level = water_level_read();
	bool aht_ok = aht20_read(&temp_c10, &hum_pct10) == 0;

	struct app_state *s = app_state_lock();

	s->soil_ok = soil_ok;
	s->soil_pct = soil_ok ? soil_pct : -1;
	s->soil_raw = soil_raw;

	s->water_sensor_ok = level >= 0;
	s->water_ok = level == 1;

	s->aht_ok = aht_ok;
	if (aht_ok) {
		s->temp_c10 = temp_c10;
		s->hum_pct10 = hum_pct10;
	}

	app_state_unlock();
}
