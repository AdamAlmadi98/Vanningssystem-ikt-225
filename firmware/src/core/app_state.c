#include <string.h>
#include <zephyr/kernel.h>

#include "core/app_state.h"
#include "config/app_config.h"

static K_MUTEX_DEFINE(state_mutex);
static struct app_state state;

void app_state_init(void)
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	memset(&state, 0, sizeof(state));
	state.soil_pct = -1;
	state.auto_enabled = DEFAULT_AUTO_ENABLED;
	state.threshold_pct = DEFAULT_THRESHOLD_PCT;
	state.schedule_hours = DEFAULT_SCHEDULE_HOURS;
	state.water_seconds = PUMP_DEFAULT_SECONDS;
	k_mutex_unlock(&state_mutex);
}

struct app_state *app_state_lock(void)
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	return &state;
}

void app_state_unlock(void)
{
	k_mutex_unlock(&state_mutex);
}

void app_state_snapshot(struct app_state *out)
{
	k_mutex_lock(&state_mutex, K_FOREVER);
	*out = state;
	k_mutex_unlock(&state_mutex);
}
