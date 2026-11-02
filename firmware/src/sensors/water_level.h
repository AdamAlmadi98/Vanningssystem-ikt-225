/* SEN0715 vannivåsensor (digital utgang, montert på utsiden av tanken) */
#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H

int water_level_init(void);

/* 1 = vann registrert, 0 = lavt vannivå, negativ = sensorfeil */
int water_level_read(void);

#endif
