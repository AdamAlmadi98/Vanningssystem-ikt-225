/* AHT20 temperatur- og luftfuktighetssensor (I2C, adresse 0x38) */
#ifndef AHT20_H
#define AHT20_H

int aht20_init(void);

/* Verdier i tideler: temp_c10 = 225 betyr 22,5 °C */
int aht20_read(int *temp_c10, int *hum_pct10);

#endif
