/*
 * Felles tilstand for hele systemet. Beskyttes av en mutex fordi
 * hovedløkken og nettverkstråden bruker den samtidig.
 *
 * Bruk:
 *     struct app_state *s = app_state_lock();
 *     s->pump_on = true;
 *     app_state_unlock();
 */
#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>

struct app_state {
	/* Målinger */
	int  soil_pct;          /* 0–100 %, -1 ved feil */
	int  soil_raw;          /* Råverdi fra ADC (nyttig ved kalibrering) */
	bool soil_ok;
	int  temp_c10;          /* Temperatur i tideler °C (225 = 22,5 °C) */
	int  hum_pct10;         /* Luftfuktighet i tideler % */
	bool aht_ok;
	bool water_ok;          /* true = nok vann i tanken */
	bool water_sensor_ok;

	/* Utganger og tilkobling */
	bool pump_on;
	bool wifi_ok;
	bool mqtt_ok;

	/* Innstillinger (kan endres fra appen) */
	bool auto_enabled;
	int  threshold_pct;
	int  schedule_hours;
	int  water_seconds;

	/* Forespørsler fra appen */
	int  manual_request_s;  /* > 0 = vann nå i så mange sekunder */
	bool stop_request;

	/* Statistikk */
	int64_t  last_water_ms;
	uint32_t water_count;
};

void app_state_init(void);
struct app_state *app_state_lock(void);
void app_state_unlock(void);
void app_state_snapshot(struct app_state *out);

#endif
