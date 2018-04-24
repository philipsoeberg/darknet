
#include "sys_includes.h"
#include "debug.h"

int _rpcerr_code;
char _rpcerr_message[512];

void hex(void *buf, int bufsz, const char *fmt, ...)
{
  unsigned char c;
  int n;
  char bytestr[4];
  char addrstr[10];
  char hexstr[ 16*3 + 5];
  char charstr[16*1 + 5];
  unsigned char *p = buf;
  memset(bytestr, 0, sizeof(bytestr));
  memset(addrstr, 0, sizeof(addrstr));
  memset(hexstr, 0, sizeof(hexstr));
  memset(charstr, 0, sizeof(charstr));
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf(" (HEX DUMP of %p length %d bytes)\n", buf, bufsz);

  for(n=1;n<=bufsz;n++) {
    if (n%16 == 1) { snprintf(addrstr, sizeof(addrstr), "%.4x", (unsigned int)(ptrdiff_t)((void*)p - (void*)buf) ); }
    c = *p;
    if (isprint(c) == 0) { c = '.'; }
    snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
    strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);
    snprintf(bytestr, sizeof(bytestr), "%c", c);
    strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);
    if(n%16 == 0) {
      printf("[%4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
      hexstr[0] = 0;
      charstr[0] = 0;
    } else if(n%8 == 0) {
      strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
      strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
    }
    p++;
  }
  if (hexstr[0] != 0) { printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr); }
}

int my_strcmp(const char *s1, const char *s2)
{
    if (!s1) return -1;
    if (!s2) return 1;
    return strcmp(s1, s2);
}
int my_strncmp(const char *s1, const char *s2, size_t n)
{
    if (!s1) return -1;
    if (!s2) return 1;
    return strncmp(s1, s2, n);
}
int my_strcasecmp(const char *s1, const char *s2)
{
    if (!s1) return -1;
    if (!s2) return 1;
    return strcasecmp(s1, s2);
}
int my_strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (!s1) return -1;
    if (!s2) return 1;
    return strncasecmp(s1, s2, n);
}
