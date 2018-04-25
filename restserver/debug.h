
#ifndef ASSIST_H
#define ASSIST_H

#include "global.h"
#include "config.h"

//Basic printf() like output macros. Always prefix threadname and a timestamp in a formatted fasion.
#define INFORM(d,s,...) { printf(d ":%16.16s:%4d: " s, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stdout); }
#define TRAP(...) do { INFORM("TRP", ##__VA_ARGS__); raise(SIGTRAP);                             } while(0)
#define DIE(...)  do { INFORM("DIE", ##__VA_ARGS__); raise(SIGTRAP); kill(0, SIGTERM); while(app_is_alive); } while(0)
#define ERR(...)  do { INFORM("ERR", ##__VA_ARGS__); raise(SIGTRAP); /*kill(0, SIGHUP);*/        } while(0)
#define WRN(...)  do { INFORM("WRN", ##__VA_ARGS__);                                             } while(0)
#define INF(...)  do { INFORM("INF", ##__VA_ARGS__);                                             } while(0)
#define DBG1(...) do { if(unlikely(config.verbose>0)) INFORM("DB1", ##__VA_ARGS__); } while(0)  //dbg level 1. Can be enabled with full packet load
#define DBG2(...) do { if(unlikely(config.verbose>1)) INFORM("DB2", ##__VA_ARGS__); } while(0)  //dbg level 2. Can be enabled with full packet load, but will degrade on error paths
#define DBG3(...) do { if(unlikely(config.verbose>2)) INFORM("DB3", ##__VA_ARGS__); } while(0)  //dbg level 3. Will degrade in normal path

//Disable/Enable Perf(5) style counters. Used only for debugging performance
#define DISABLE_PERF { prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0); }
#define ENABLE_PERF { prctl(PR_TASK_PERF_EVENTS_ENABLE , 0, 0, 0, 0); }

//Compiler hints
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

//Assert macro. Checks equality and DIEs with errortext
#define ASSERT(x) { if (!(x)) { DIE("ASSERT failed for [" #x "] @ %s:%d\n", __func__, __LINE__);}}
#define ASSERT_DO(x, b) { if (!(x)) { ERR("ASSERT failed for [" #x "] @ %s:%d\n", __func__, __LINE__); b;}}

void hex(void *buf, int bufsz, const char *fmt, ...);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, size_t n);
int my_strcasecmp(const char *s1, const char *s2);
int my_strncasecmp(const char *s1, const char *s2, size_t n);

#endif
