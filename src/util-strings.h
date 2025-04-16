/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013-2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#ifdef HAVE_XLOCALE_H
#include <xlocale.h>
#endif

#include "util-macros.h"

static inline void *
zalloc(size_t size)
{
	void *p;

	/* We never need to alloc anything more than 1,5 MB so we can assume
	 * if we ever get above that something's going wrong */
	if (size > 1536 * 1024)
		assert(!"bug: internal malloc size limit exceeded");

	p = calloc(1, size);
	if (!p)
		abort();

	return p;
}

static inline bool
safe_atoi_base(const char *str, int *val, int base)
{
	char *endptr;
	long v;

	assert(base == 10 || base == 16 || base == 8);

	errno = 0;
	v = strtol(str, &endptr, base);
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;

	if (v > INT_MAX || v < INT_MIN)
		return false;

	*val = v;
	return true;
}

static inline bool
safe_atoi(const char *str, int *val)
{
	return safe_atoi_base(str, val, 10);
}

static inline bool
safe_atou_base(const char *str, unsigned int *val, int base)
{
	char *endptr;
	unsigned long v;

	assert(base == 10 || base == 16 || base == 8);

	errno = 0;
	v = strtoul(str, &endptr, base);
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;

	if ((long)v < 0)
		return false;

	*val = v;
	return true;
}

static inline bool
safe_atou(const char *str, unsigned int *val)
{
	return safe_atou_base(str, val, 10);
}

static inline bool
safe_atod(const char *str, double *val)
{
	char *endptr;
	double v;
#ifdef HAVE_LOCALE_H
	locale_t c_locale;
#endif
	size_t slen = strlen(str);

	/* We don't have a use-case where we want to accept hex for a double
	 * or any of the other values strtod can parse */
	for (size_t i = 0; i < slen; i++) {
		char c = str[i];

		if (isdigit(c))
		       continue;
		switch(c) {
		case '+':
		case '-':
		case '.':
			break;
		default:
			return false;
		}
	}

#ifdef HAVE_LOCALE_H
	/* Create a "C" locale to force strtod to use '.' as separator */
	c_locale = newlocale(LC_NUMERIC_MASK, "C", (locale_t)0);
	if (c_locale == (locale_t)0)
		return false;

	errno = 0;
	v = strtod_l(str, &endptr, c_locale);
	freelocale(c_locale);
#else
	/* No locale support in provided libc, assume it already uses '.' */
	errno = 0;
	v = strtod(str, &endptr);
#endif
	if (errno > 0)
		return false;
	if (str == endptr)
		return false;
	if (*str != '\0' && *endptr != '\0')
		return false;
	if (v != 0.0 && !isnormal(v))
		return false;

	*val = v;
	return true;
}

char **strv_from_string(const char *in, const char *separator, size_t *num_elements);

static inline void
strv_free(char **strv) {
	char **s = strv;

	if (!strv)
		return;

	while (*s != NULL) {
		free(*s);
		*s = (char*)0x1; /* detect use-after-free */
		s++;
	}

	free (strv);
}

/**
 * parse a string containing a list of doubles into a double array.
 *
 * @param in string to parse
 * @param separator string used to separate double in list e.g. ","
 * @param result double array
 * @param length length of double array
 * @return true when parsed successfully otherwise false
 */
static inline double *
double_array_from_string(const char *in,
			 const char *separator,
			 size_t *length)
{
	double *result = NULL;
	*length = 0;

	size_t nelem;
	char **strv = strv_from_string(in, separator, &nelem);
	if(!strv)
		return result;

	double *numv = zalloc(sizeof(double) * nelem);
	for (size_t idx = 0; idx < nelem; idx++) {
		double val;
		if (!safe_atod(strv[idx], &val))
			goto out;

		numv[idx] = val;
	}

	result = numv;
	numv = NULL;
	*length = nelem;

out:
	strv_free(strv);
	free(numv);
	return result;
}
