#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "control/watering.h"
#include "actuators/pump.h"
#include "core/app_state.h"
#include "core/events.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(watering, LOG_LEVEL_INF);

#define AUTO_COOLDOWN_MS ((int64_t)AUTO_COOLDOWN_S * 1000)

static int64_t pump_start_ms;
static int64_t pump_end_ms;
static const char *pump_reason;
static int64_t last_auto_ms;
static int64_t last_schedule_ms;

/* Brukes for å sende varsler bare når tilstanden endrer seg */
static bool prev_water_ok = true;
static bool prev_needs_water;

/* Returnerer årsaken til at pumpen IKKE får gå, eller NULL hvis alt er OK */
static const char *safety_problem(const struct app_state *s)
{
	if (!s->water_sensor_ok) {
		return "water_sensor_fault";
	}
	if (!s->water_ok) {
		return "low_water";
	}
	return NULL;
}

static void start_pump(struct app_state *s, int seconds, const char *reason, int64_t now)
{
	seconds = CLAMP(seconds, 1, PUMP_MAX_SECONDS);
	pump_set(true);
	s->pump_on = true;
	pump_start_ms = now;
	pump_end_ms = now + (int64_t)seconds * 1000;
	pump_reason = reason;
	LOG_INF("Starter vanning: %d s (%s)", seconds, reason);
}

static void stop_pump(struct app_state *s, int64_t now, const char *event_type, const char *stopped_by)
{
	pump_set(false);
	s->pump_on = false;
	s->last_water_ms = now;
	s->water_count++;
	app_event_post("{\"type\":\"%s\",\"reason\":\"%s\",\"stopped\":\"%s\",\"ms\":%d}",
		       event_type, pump_reason, stopped_by, (int)(now - pump_start_ms));
}

static void check_alerts(struct app_state *s)
{
	if (prev_water_ok && !s->water_ok) {
		app_event_post("{\"type\":\"low_water\"}");
	}
	prev_water_ok = s->water_ok;

	bool needs_water = s->soil_ok && s->soil_pct < s->threshold_pct;
	if (needs_water && !prev_needs_water) {
		app_event_post("{\"type\":\"needs_water\",\"soil\":%d}", s->soil_pct);
	}
	prev_needs_water = needs_water;
}

void watering_init(void)
{
	pump_set(false);
	last_auto_ms = -AUTO_COOLDOWN_MS;  /* Tillat automatisk vanning med en gang */
	last_schedule_ms = k_uptime_get();
	app_event_post("{\"type\":\"boot\"}");
}

void watering_update(void)
{
	int64_t now = k_uptime_get();
	struct app_state *s = app_state_lock();
	const char *problem = safety_problem(s);

	if (s->pump_on) {
		/* Pumpen går: sjekk om den skal stoppes */
		if (problem) {
			stop_pump(s, now, "safety_stop", problem);
		} else if (s->stop_request) {
			stop_pump(s, now, "watered", "user");
		} else if (now >= pump_end_ms) {
			stop_pump(s, now, "watered", "done");
		}
	} else {
		/* Pumpen står: sjekk om den skal starte */
		const char *reason = NULL;
		int seconds = s->water_seconds;

		if (s->manual_request_s > 0) {
			if (problem) {
				app_event_post("{\"type\":\"rejected\",\"reason\":\"%s\"}", problem);
			} else {
				reason = "manual";
				seconds = s->manual_request_s;
			}
		} else if (!problem && s->soil_ok) {
			int64_t schedule_ms = (int64_t)s->schedule_hours * 3600 * 1000;

			if (s->auto_enabled && s->soil_pct < s->threshold_pct &&
			    now - last_auto_ms >= AUTO_COOLDOWN_MS) {
				reason = "auto";
				last_auto_ms = now;
			} else if (s->schedule_hours > 0 && now - last_schedule_ms >= schedule_ms) {
				last_schedule_ms = now;
				if (s->soil_pct < SCHEDULE_SKIP_ABOVE_PCT) {
					reason = "schedule";
				} else {
					app_event_post("{\"type\":\"schedule_skipped\",\"soil\":%d}", s->soil_pct);
				}
			}
		}

		if (reason) {
			start_pump(s, seconds, reason, now);
		}
	}

	s->manual_request_s = 0;
	s->stop_request = false;

	check_alerts(s);
	app_state_unlock();
}

void watering_request_manual(int seconds)
{
	struct app_state *s = app_state_lock();
	s->manual_request_s = seconds > 0 ? seconds : s->water_seconds;
	app_state_unlock();
}

void watering_request_stop(void)
{
	struct app_state *s = app_state_lock();
	s->stop_request = true;
	app_state_unlock();
}
