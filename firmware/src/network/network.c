#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "network/network.h"
#include "network/wifi.h"
#include "network/mqtt_client.h"
#include "core/app_state.h"
#include "core/events.h"
#include "core/format.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(network, LOG_LEVEL_INF);

#define NETWORK_STACK_SIZE 6144
#define NETWORK_PRIORITY   7
#define RETRY_DELAY        K_SECONDS(5)

static void set_status(bool wifi, bool mqtt)
{
	struct app_state *s = app_state_lock();
	s->wifi_ok = wifi;
	s->mqtt_ok = mqtt;
	app_state_unlock();
}

static void publish_sensors(void)
{
	struct app_state s;
	char json[384], temp[16] = "null", hum[16] = "null";

	app_state_snapshot(&s);
	if (s.aht_ok) {
		fmt_tenths(temp, sizeof(temp), s.temp_c10);
		fmt_tenths(hum, sizeof(hum), s.hum_pct10);
	}

#define B(x) ((x) ? "true" : "false")
	snprintf(json, sizeof(json),
		 "{\"soil\":%d,\"soil_raw\":%d,\"soil_ok\":%s,"
		 "\"temp\":%s,\"hum\":%s,\"aht_ok\":%s,"
		 "\"water_ok\":%s,\"water_sensor_ok\":%s,\"pump\":%s,"
		 "\"auto\":%s,\"threshold\":%d,\"interval_h\":%d,\"seconds\":%d,"
		 "\"water_count\":%u,\"uptime_s\":%u}",
		 s.soil_pct, s.soil_raw, B(s.soil_ok),
		 temp, hum, B(s.aht_ok),
		 B(s.water_ok), B(s.water_sensor_ok), B(s.pump_on),
		 B(s.auto_enabled), s.threshold_pct, s.schedule_hours, s.water_seconds,
		 s.water_count, (unsigned int)(k_uptime_get() / 1000));
#undef B

	mqtt_app_publish(MQTT_TOPIC_SENSOR, json);
}

static void publish_events(void)
{
	char ev[APP_EVENT_MAX_LEN];

	while (app_event_get(ev, sizeof(ev)) == 0) {
		mqtt_app_publish(MQTT_TOPIC_EVENT, ev);
	}
}

static void network_thread(void *a, void *b, void *c)
{
	int64_t last_publish = 0;
	bool pump_was_on = false;

	wifi_init();

	while (1) {
		if (!wifi_is_connected()) {
			set_status(false, false);
			if (wifi_connect() != 0) {
				k_sleep(RETRY_DELAY);
				continue;
			}
		}

		if (!mqtt_app_is_connected()) {
			set_status(true, false);
			if (mqtt_app_connect() != 0) {
				k_sleep(RETRY_DELAY);
				continue;
			}
			set_status(true, true);
			last_publish = 0;   /* Send målinger med en gang */
		}

		mqtt_app_process(200);
		publish_events();

		/* Send målinger jevnlig, og med en gang når pumpen endrer tilstand */
		struct app_state s;
		app_state_snapshot(&s);
		int64_t now = k_uptime_get();
		if (now - last_publish >= MQTT_PUBLISH_PERIOD_S * 1000 || s.pump_on != pump_was_on) {
			publish_sensors();
			last_publish = now;
			pump_was_on = s.pump_on;
		}
	}
}

K_THREAD_DEFINE(network_tid, NETWORK_STACK_SIZE, network_thread, NULL, NULL, NULL,
		NETWORK_PRIORITY, 0, SYS_FOREVER_MS);

void network_start(void)
{
	k_thread_start(network_tid);
}
