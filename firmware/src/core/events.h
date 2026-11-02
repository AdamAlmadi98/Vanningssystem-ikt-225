/*
 * Kø for hendelser (vanning, alarmer) som skal sendes til serveren.
 * Hendelsene er ferdige JSON-strenger.
 */
#ifndef EVENTS_H
#define EVENTS_H

#include <stddef.h>

#define APP_EVENT_MAX_LEN 128

/* printf-stil: app_event_post("{\"type\":\"low_water\"}"); */
void app_event_post(const char *fmt, ...);

/* Henter neste hendelse. Returnerer 0 hvis en hendelse ble hentet. */
int app_event_get(char *buf, size_t len);

#endif
