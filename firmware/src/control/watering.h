/*
 * Vanningslogikk: automatisk vanning, tidsplan, manuell vanning,
 * varsler og sikkerhetsstopp.
 */
#ifndef WATERING_H
#define WATERING_H

void watering_init(void);
void watering_update(void);              /* Kalles én gang per CONTROL_PERIOD_MS */

void watering_request_manual(int seconds); /* 0 = bruk standard varighet */
void watering_request_stop(void);

#endif
