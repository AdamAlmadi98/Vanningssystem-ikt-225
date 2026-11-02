#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>

int mqtt_app_connect(void);
bool mqtt_app_is_connected(void);
void mqtt_app_process(int timeout_ms);   /* Mottar meldinger og holder forbindelsen i live */
int mqtt_app_publish(const char *topic, const char *payload);

#endif
