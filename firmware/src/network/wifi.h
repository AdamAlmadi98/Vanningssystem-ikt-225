#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

void wifi_init(void);
int wifi_connect(void);       /* Blokkerer til vi har IP-adresse (maks 30 s) */
bool wifi_is_connected(void);

#endif
