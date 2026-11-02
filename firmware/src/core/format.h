#ifndef FORMAT_H
#define FORMAT_H

#include <stdio.h>
#include <stddef.h>

/* Skriver en verdi i tideler som desimaltall, f.eks. 225 -> "22.5" */
static inline void fmt_tenths(char *buf, size_t len, int value)
{
	int a = value < 0 ? -value : value;

	snprintf(buf, len, "%s%d.%d", value < 0 ? "-" : "", a / 10, a % 10);
}

#endif
