#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>

#include "sensors/soil_sensor.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(soil_sensor, LOG_LEVEL_INF);

static const struct adc_dt_spec adc_ch = ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), 0);
static bool ready;

int soil_sensor_init(void)
{
	if (!adc_is_ready_dt(&adc_ch)) {
		return -ENODEV;
	}
	int err = adc_channel_setup_dt(&adc_ch);
	if (err) {
		return err;
	}
	ready = true;
	return 0;
}

static int read_raw(int *raw)
{
	uint16_t sample;
	struct adc_sequence seq = {
		.buffer = &sample,
		.buffer_size = sizeof(sample),
	};

	int err = adc_sequence_init_dt(&adc_ch, &seq);
	if (err) {
		return err;
	}
	err = adc_read_dt(&adc_ch, &seq);
	if (err) {
		return err;
	}
	*raw = sample;
	return 0;
}

int soil_sensor_read(int *percent, int *raw)
{
	if (!ready) {
		return -ENODEV;
	}

	int sum = 0;
	for (int i = 0; i < SOIL_SAMPLES; i++) {
		int value;
		int err = read_raw(&value);
		if (err) {
			return err;
		}
		sum += value;
		k_usleep(500);
	}
	*raw = sum / SOIL_SAMPLES;

	if (*raw < SOIL_RAW_MIN_VALID || *raw > SOIL_RAW_MAX_VALID) {
		LOG_WRN("Ugyldig jordfuktighetsverdi: %d", *raw);
		return -EIO;
	}

	/* Kapasitiv sensor: høyere spenning = tørrere jord */
	int pct = (SOIL_RAW_DRY - *raw) * 100 / (SOIL_RAW_DRY - SOIL_RAW_WET);
	*percent = CLAMP(pct, 0, 100);
	return 0;
}
