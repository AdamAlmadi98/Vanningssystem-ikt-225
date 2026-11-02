#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/data/json.h>
#include <zephyr/logging/log.h>

#include "control/commands.h"
#include "control/watering.h"
#include "core/app_state.h"
#include "config/app_config.h"

LOG_MODULE_REGISTER(commands, LOG_LEVEL_INF);

struct cmd_msg {
	char   *cmd;
	int32_t seconds;
	int32_t threshold;
	int32_t interval_h;
	bool    auto_on;
};

/* Rekkefølgen her bestemmer bit-nummeret i returverdien fra json_obj_parse */
enum { F_CMD, F_SECONDS, F_THRESHOLD, F_INTERVAL, F_AUTO };

static const struct json_obj_descr cmd_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct cmd_msg, cmd, JSON_TOK_STRING),
	JSON_OBJ_DESCR_PRIM(struct cmd_msg, seconds, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct cmd_msg, threshold, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM(struct cmd_msg, interval_h, JSON_TOK_NUMBER),
	JSON_OBJ_DESCR_PRIM_NAMED(struct cmd_msg, "auto", auto_on, JSON_TOK_TRUE),
};

void commands_handle(char *json, size_t len)
{
	struct cmd_msg m = { 0 };
	int fields = json_obj_parse(json, len, cmd_descr, ARRAY_SIZE(cmd_descr), &m);

	if (fields < 0 || !(fields & BIT(F_CMD))) {
		LOG_WRN("Ugyldig kommando");
		return;
	}
	LOG_INF("Kommando: %s", m.cmd);

	if (strcmp(m.cmd, "water") == 0) {
		watering_request_manual((fields & BIT(F_SECONDS)) ? m.seconds : 0);
	} else if (strcmp(m.cmd, "stop") == 0) {
		watering_request_stop();
	} else if (strcmp(m.cmd, "config") == 0) {
		struct app_state *s = app_state_lock();
		if (fields & BIT(F_SECONDS)) {
			s->water_seconds = CLAMP(m.seconds, 1, PUMP_MAX_SECONDS);
		}
		if (fields & BIT(F_THRESHOLD)) {
			s->threshold_pct = CLAMP(m.threshold, 0, 100);
		}
		if (fields & BIT(F_INTERVAL)) {
			s->schedule_hours = CLAMP(m.interval_h, 0, 72);
		}
		if (fields & BIT(F_AUTO)) {
			s->auto_enabled = m.auto_on;
		}
		app_state_unlock();
	} else {
		LOG_WRN("Ukjent kommando: %s", m.cmd);
	}
}
