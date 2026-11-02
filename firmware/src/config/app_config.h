/*
 * Alle innstillinger for vanningssystemet samlet på ett sted.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if __has_include("config/secrets.h")
#include "config/secrets.h"
#else
#error "Kopier src/config/secrets.example.h til src/config/secrets.h og fyll inn Wi-Fi og MQTT."
#endif

/* ---------- Hovedløkke ---------- */
#define CONTROL_PERIOD_MS        1000   /* Hvor ofte sensorer leses og logikken kjøres */

/* ---------- Jordfuktighet (kalibrer disse!) ---------- */
#define SOIL_RAW_DRY             3000   /* Råverdi med sensoren i luft */
#define SOIL_RAW_WET             1300   /* Råverdi med sensoren i et glass vann */
#define SOIL_RAW_MIN_VALID       100    /* Under dette = sensorfeil (frakoblet) */
#define SOIL_RAW_MAX_VALID       4000   /* Over dette = sensorfeil */
#define SOIL_SAMPLES             8      /* Antall målinger som gjennomsnittes */

/* ---------- Vanning ---------- */
#define DEFAULT_AUTO_ENABLED     true
#define DEFAULT_THRESHOLD_PCT    30     /* Vann når jorden er tørrere enn dette */
#define DEFAULT_SCHEDULE_HOURS   0      /* 0 = tidsplan av */
#define PUMP_DEFAULT_SECONDS     3
#define PUMP_MAX_SECONDS         15     /* Sikkerhet: pumpen går aldri lenger enn dette */
#define AUTO_COOLDOWN_S          (10 * 60) /* Vent før ny automatisk vanning (vannet må trekke inn) */
#define SCHEDULE_SKIP_ABOVE_PCT  80     /* Tidsplan hopper over vanning hvis jorden er våtere */

/* ---------- MQTT ---------- */
#define MQTT_BROKER_PORT         1883
#define MQTT_CLIENT_ID           "vanning-esp32"
#define MQTT_TOPIC_SENSOR        "vanning/sensor"
#define MQTT_TOPIC_EVENT         "vanning/event"
#define MQTT_TOPIC_CMD           "vanning/cmd"
#define MQTT_PUBLISH_PERIOD_S    10

#endif
