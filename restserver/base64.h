
#ifndef BASE64_H
#define BASE64_H

#include <sys/types.h>

int
my_b64_ntop(u_char const *src, size_t srclength, char *target, size_t targsize);

int
my_b64_pton(char const *src, u_char *target, size_t targsize);

#endif
