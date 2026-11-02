/* FIT0800 vannpumpe styrt via TB6612FNG (kanal A) */
#ifndef PUMP_H
#define PUMP_H

#include <stdbool.h>

int pump_init(void);
void pump_set(bool on);

#endif
