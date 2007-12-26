/* Random util functions, mainly taken from stdrusty.h */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "utils.h"

void *_realloc_array(void *ptr, size_t size, size_t num)
{
        if (num >= SIZE_MAX/size)
                return NULL;
        return realloc_nofail(ptr, size * num);
}

void *realloc_nofail(void *ptr, size_t size)
{
        ptr = realloc(ptr, size);
	if (ptr)
		return ptr;
	barf("realloc of %zu failed", size);
}

void *malloc_nofail(size_t size)
{
	void *ptr = malloc(size);
	if (ptr)
		return ptr;
	barf("malloc of %zu failed", size);
}

/* Avoid stupid sprintf-style non-value return. */
char *aprintf(const char *fmt, ...)
{
	char *ret;
	va_list arglist;

	va_start(arglist, fmt);
	vasprintf(&ret, fmt, arglist);
	va_end(arglist);
	return ret;
}

char *aprintf_add(char *s, const char *fmt, ...)
{
	int oldlen, len;
	va_list arglist;

	va_start(arglist, fmt);
	len = vsnprintf(NULL, 0, fmt, arglist);
	va_end(arglist);

	oldlen = (s ? strlen(s) : 0);
	s = realloc(s, oldlen + len + 1);

	va_start(arglist, fmt);
	vsnprintf(s + oldlen, len + 1, fmt, arglist);
	va_end(arglist);
	return s;
}

void logpv(const char *fmt, va_list arglist)
{
	char *str;
	vasprintf(&str, fmt, arglist);
	write(log_fd, str, strlen(str));
}

void logp(const char *fmt, ...)
{
	va_list arglist;

	va_start(arglist, fmt);
	logpv(fmt, arglist);
	write(log_fd, "\n", 1);
	va_end(arglist);
}

void barf(const char *fmt, ...)
{
	va_list arglist;

	write(log_fd, "FATAL: ", strlen("FATAL: "));
	va_start(arglist, fmt);
	logpv(fmt, arglist);
	write(log_fd, "\n", 1);
	va_end(arglist);
	exit(1);
}

void barf_perror(const char *fmt, ...)
{
	int err = errno;
	va_list arglist;

	fprintf(stderr, "FATAL: ");
	va_start(arglist, fmt);
	logpv(fmt, arglist);
	va_end(arglist);
	logp(": %s", strerror(err));
	exit(1);
}

/* This is all squid allows in a POST by default anyway. */
#define MAXIMUM_LEN 40000

/* This version adds one byte (for nul term) */
void *grab_input(int fd, unsigned long *size)
{
	int ret;
	void *buffer;

	buffer = malloc(MAXIMUM_LEN+1);
	*size = 0;
	while ((ret = read(fd, buffer + *size, MAXIMUM_LEN - *size)) > 0) {
		*size += ret;
		if (*size == MAXIMUM_LEN)
			barf("too much input");
	}
	if (ret < 0)
		barf_perror("read failure");

	((char *)buffer)[*size] = '\0';
	return buffer;
}

void _delete_arr(void *p, unsigned len, unsigned off, unsigned num, size_t s)
{
	assert(off + num <= len);
	memmove(p + off*s, p + (off+num)*s, (len - (off+num))*s);
}
