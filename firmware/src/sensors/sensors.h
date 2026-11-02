/*
 * Samler alle sensorene. Hovedløkken trenger bare å kalle
 * sensors_init() én gang og sensors_update() jevnlig.
 */
#ifndef SENSORS_H
#define SENSORS_H

void sensors_init(void);
void sensors_update(void);   /* Leser alle sensorer og oppdaterer app_state */

#endif
