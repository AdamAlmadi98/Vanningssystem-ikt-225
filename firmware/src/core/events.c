#include <stdarg.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "core/events.h"

LOG_MODULE_REGISTER(events, LOG_LEVEL_INF);

struct app_event {
	char text[APP_EVENT_MAX_LEN];
};

K_MSGQ_DEFINE(event_q, sizeof(struct app_event), 8, 4);

void app_event_post(const char *fmt, ...)
{
	struct app_event ev;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(ev.text, sizeof(ev.text), fmt, ap);
	va_end(ap);

	LOG_INF("Hendelse: %s", ev.text);
	if (k_msgq_put(&event_q, &ev, K_NO_WAIT) != 0) {
		LOG_WRN("Hendelseskøen er full, hendelsen forkastes");
	}
}

int app_event_get(char *buf, size_t len)
{
	struct app_event ev;

	if (k_msgq_get(&event_q, &ev, K_NO_WAIT) != 0) {
		return -EAGAIN;
	}
	snprintf(buf, len, "%s", ev.text);
	return 0;
}
