/*
 * Tolker kommandoer (JSON) fra appen via MQTT.
 *   {"cmd":"water","seconds":5}
 *   {"cmd":"stop"}
 *   {"cmd":"config","auto":true,"threshold":30,"interval_h":5,"seconds":3}
 */
#ifndef COMMANDS_H
#define COMMANDS_H

#include <stddef.h>

void commands_handle(char *json, size_t len);

#endif
