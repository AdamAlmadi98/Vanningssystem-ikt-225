/* Gravity IP65 kapasitiv jordfuktighetssensor (analog) */
#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

int soil_sensor_init(void);

/* percent: 0 = helt tørr, 100 = vann. raw: ADC-verdi (settes alltid).
 * Returnerer 0 ved OK, negativ feilkode ved sensorfeil. */
int soil_sensor_read(int *percent, int *raw);

#endif
