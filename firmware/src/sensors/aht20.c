/*
 * Enkel driver for AHT20 etter databladet:
 *   1. Init:  send 0xBE 0x08 0x00 hvis sensoren ikke er kalibrert
 *   2. Mål:   send 0xAC 0x33 0x00, vent 80 ms, les 7 byte
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include "sensors/aht20.h"

#define AHT20_ADDR            0x38
#define AHT20_STATUS_BUSY     0x80
#define AHT20_STATUS_CAL      0x08

static const struct device *i2c = DEVICE_DT_GET(DT_ALIAS(app_i2c));
static bool initialized;

static uint8_t crc8(const uint8_t *data, size_t len)
{
	uint8_t crc = 0xFF;

	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (int b = 0; b < 8; b++) {
			crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
		}
	}
	return crc;
}

int aht20_init(void)
{
	uint8_t status;

	if (!device_is_ready(i2c)) {
		return -ENODEV;
	}
	k_msleep(40);

	int err = i2c_read(i2c, &status, 1, AHT20_ADDR);
	if (err) {
		return err;
	}
	if (!(status & AHT20_STATUS_CAL)) {
		const uint8_t cmd[] = { 0xBE, 0x08, 0x00 };
		err = i2c_write(i2c, cmd, sizeof(cmd), AHT20_ADDR);
		if (err) {
			return err;
		}
		k_msleep(10);
	}
	initialized = true;
	return 0;
}

int aht20_read(int *temp_c10, int *hum_pct10)
{
	const uint8_t cmd[] = { 0xAC, 0x33, 0x00 };
	uint8_t d[7];
	int err;

	if (!initialized) {
		err = aht20_init();
		if (err) {
			return err;
		}
	}

	err = i2c_write(i2c, cmd, sizeof(cmd), AHT20_ADDR);
	if (err) {
		initialized = false;
		return err;
	}
	k_msleep(80);

	err = i2c_read(i2c, d, sizeof(d), AHT20_ADDR);
	if (err) {
		initialized = false;
		return err;
	}
	if (d[0] & AHT20_STATUS_BUSY) {
		return -EBUSY;
	}
	if (crc8(d, 6) != d[6]) {
		return -EIO;
	}

	uint32_t hum_raw = ((uint32_t)d[1] << 12) | ((uint32_t)d[2] << 4) | (d[3] >> 4);
	uint32_t temp_raw = (((uint32_t)d[3] & 0x0F) << 16) | ((uint32_t)d[4] << 8) | d[5];

	*hum_pct10 = (int)((hum_raw * 1000ULL) >> 20);
	*temp_c10 = (int)((temp_raw * 2000ULL) >> 20) - 500;
	return 0;
}
