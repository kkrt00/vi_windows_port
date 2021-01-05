#include "libbb_custom.h"
// from last_char_is.c
/* Find out if the last character of a string matches the one given */
char* FAST_FUNC last_char_is(const char *s, int c)
{
	if (!s || !s[0])
		return NULL;
	while (s[1])
		s++;
	return (*s == (char)c) ? (char *) s : NULL;
}

// from bb_bswap_64.c
#if !(ULONG_MAX > 0xffffffff)
uint64_t FAST_FUNC bb_bswap_64(uint64_t x)
{
	return bswap_64(x);
}
#endif

// from strndup.c

#include <string.h>
#include <stdlib.h>

char *
strndup (char const *s, size_t n)
{
  size_t len = strnlen (s, n);
  char *new = malloc (len + 1);

  if (new == NULL)
    return NULL;

  new[len] = '\0';
  return memcpy (new, s, len);
}
