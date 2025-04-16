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

#include "config.h"
#define _GNU_SOURCE

#include "util-strings.h"

/**
 * Return the next word in a string pointed to by state before the first
 * separator character. Call repeatedly to tokenize a whole string.
 *
 * @param state Current state
 * @param len String length of the word returned
 * @param separators List of separator characters
 *
 * @return The first word in *state, NOT null-terminated
 */
static const char *
next_word(const char **state, size_t *len, const char *separators)
{
	const char *next = *state;
	size_t l;

	if (!*next)
		return NULL;

	next += strspn(next, separators);
	if (!*next) {
		*state = next;
		return NULL;
	}

	l = strcspn(next, separators);
	*state = next + l;
	*len = l;

	return next;
}

/**
 * Return a null-terminated string array with the tokens in the input
 * string, e.g. "one two\tthree" with a separator list of " \t" will return
 * an array [ "one", "two", "three", NULL ] and num elements 3.
 *
 * Use strv_free() to free the array.
 *
 * Another example:
 *   result = strv_from_string("+1-2++3--4++-+5-+-", "+-", &nelem)
 *   result == [ "1", "2", "3", "4", "5", NULL ] and nelem == 5
 *
 * @param in Input string
 * @param separators List of separator characters
 * @param num_elements Number of elements found in the input string
 *
 * @return A null-terminated string array or NULL on errors
 */
char **
strv_from_string(const char *in, const char *separators, size_t *num_elements)
{
	assert(in != NULL);

	const char *s = in;
	size_t l, nelems = 0;
	while (next_word(&s, &l, separators) != NULL)
		nelems++;

	if (nelems == 0) {
		*num_elements = 0;
		return NULL;
	}

	size_t strv_len = nelems + 1; /* NULL-terminated */
	char **strv = zalloc(strv_len * sizeof *strv);

	size_t idx = 0;
	const char *word;
	s = in;
	while ((word = next_word(&s, &l, separators)) != NULL) {
		char *copy = strndup(word, l);
		if (!copy) {
			strv_free(strv);
			*num_elements = 0;
			return NULL;
		}

		strv[idx++] = copy;
	}

	*num_elements = nelems;

	return strv;
}
