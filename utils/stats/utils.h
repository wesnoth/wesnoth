/* Random util functions, mainly taken from stdrusty.h */
#ifndef _UPLOAD_ANALYSIS_UTILS_H
#define _UPLOAD_ANALYSIS_UTILS_H
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

/* Is A == B ? */
#define streq(a,b) (strcmp((a),(b)) == 0)

/* Does A start with B ? */
#define strstarts(a,b) (strncmp((a),(b),strlen(b)) == 0)

/* Does A end in B ? */
static inline bool strends(const char *a, const char *b)
{
	if (strlen(a) < strlen(b))
		return false;

	return streq(a + strlen(a) - strlen(b), b);
}

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define ___stringify(x)	#x
#define __stringify(x)		___stringify(x)

/* Convenient wrappers for malloc and realloc.  Use them. */
#define new(type) ((type *)malloc_nofail(sizeof(type)))
#define new_array(type, num) realloc_array((type *)0, (num))
#define realloc_array(ptr, num) ((__typeof__(ptr))_realloc_array((ptr), sizeof((*ptr)), (num)))

void *malloc_nofail(size_t size);
void *realloc_nofail(void *ptr, size_t size);
void *_realloc_array(void *ptr, size_t size, size_t num);

/* Avoid stupid sprintf-style */
char *aprintf(const char *fmt, ...) __attribute__((format(printf,1,2)));
char *aprintf_add(char *s, const char *fmt, ...)
	__attribute__((format(printf,2,3)));

void logpv(const char *fmt, va_list arglist);
void logp(const char *fmt, ...) __attribute__((format(printf,1,2)));
void barf(const char *fmt, ...) __attribute__((noreturn, format(printf,1,2)));
void barf_perror(const char *fmt, ...)
	__attribute__((noreturn, format(printf,1,2)));

void *grab_input(int fd, unsigned long *size);

extern int log_fd;

/* from the Linux Kernel:
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

/* Delete (by memmove) elements of an array */
#define delete_arr(p, len, off, num) \
	_delete_arr((p), (len), (off), (num), sizeof(*p))
void _delete_arr(void *p, unsigned len, unsigned off, unsigned num, size_t s);
#endif /* _UPLOAD_ANALYSIS_UTILS_H */
