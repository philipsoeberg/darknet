
#ifndef ASSIST_H
#define ASSIST_H

#include "global.h"
#include "config.h"

//Basic printf() like output macros. Always prefix threadname and a timestamp in a formatted fasion.
#define INFORM(d,s,...) { printf(d ":%16.16s:%4d: " s, __FILE__, __LINE__, ##__VA_ARGS__); }
#define TRAP(...) do { INFORM("TRP", ##__VA_ARGS__); raise(SIGTRAP);                             } while(0)
#define DIE(...)  do { INFORM("DIE", ##__VA_ARGS__); raise(SIGTRAP); kill(0, SIGTERM); while(app_is_alive); } while(0)
#define ERR(...)  do { RPCERR(-32700, ##__VA_ARGS__); INFORM("ERR", ##__VA_ARGS__); raise(SIGTRAP); /*kill(0, SIGHUP);*/        } while(0)
#define WRN(...)  do { INFORM("WRN", ##__VA_ARGS__);                                             } while(0)
#define INF(...)  do { INFORM("INF", ##__VA_ARGS__);                                             } while(0)
#define DBG1(...) do { if(unlikely(config.verbose>0)) INFORM("DB1", ##__VA_ARGS__); } while(0)  //dbg level 1. Can be enabled with full packet load
#define DBG2(...) do { if(unlikely(config.verbose>1)) INFORM("DB2", ##__VA_ARGS__); } while(0)  //dbg level 2. Can be enabled with full packet load, but will degrade on error paths
#define DBG3(...) do { if(unlikely(config.verbose>2)) INFORM("DB3", ##__VA_ARGS__); } while(0)  //dbg level 3. Will degrade in normal path

#define RPCERR(code, message, ...)                                              \
  do {                                                                          \
    snprintf(_rpcerr_message, sizeof(_rpcerr_message), message, ##__VA_ARGS__); \
    _rpcerr_code = (code);                                                      \
  } while(0)

//Disable/Enable Perf(5) style counters. Used only for debugging performance
#define DISABLE_PERF { prctl(PR_TASK_PERF_EVENTS_DISABLE, 0, 0, 0, 0); }
#define ENABLE_PERF { prctl(PR_TASK_PERF_EVENTS_ENABLE , 0, 0, 0, 0); }

//Compiler hints
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

//Trace macro to output text errors if NTCI call fail
#define TRC(resultvar, skiptraceifreturn, calltext, call) \
  {                                                                     \
    resultvar = (call);                                                 \
    if (resultvar != skiptraceifreturn) {                               \
      char buf[200];                                                    \
      NT_ExplainError(resultvar, buf, sizeof(buf));                     \
      INFORM("TRC", "NTAPI call [%s] failed with code 0x%x\n>>>%s<<<\n", calltext, resultvar, buf); \
    }                                                                   \
  }

//Trace macro similar to TRC above but gives you greater control over the condition test
#define TRCM(resultvar, condition_fail, calltext, call) \
  {                                                                     \
    resultvar = (call);                                                 \
    if ((condition_fail)) {                                             \
      char buf[200];                                                    \
      NT_ExplainError(resultvar, buf, sizeof(buf));                     \
      INFORM("TRC", "NTAPI call [%s] failed with code 0x%x\n>>>%s<<<\n", calltext, resultvar, buf); \
    }                                                                   \
  }

//Assert macro. Checks equality and DIEs with errortext
#define ASSERT(x) { if (!(x)) { DIE("ASSERT failed for [" #x "] @ %s:%d\n", __func__, __LINE__);}}
#define ASSERT_DO(x, b) { if (!(x)) { ERR("ASSERT failed for [" #x "] @ %s:%d\n", __func__, __LINE__); b;}}

//Send NTPL message
#define SEND_NTPL(configstream, ...)                                                                   \
  do {                                                                                                 \
    int result;                                                                                        \
    NtNtplInfo_t ntplInfo;                                                                             \
    char buf[200];                                                                                     \
    snprintf(buf, sizeof(buf), __VA_ARGS__);                                                           \
    INF("Sending NTPL command [%s]\n", buf);                                                           \
    TRC(result, NT_SUCCESS, "NT_NTPL",                                                                 \
      NT_NTPL((configstream), buf, &ntplInfo, NT_NTPL_PARSER_VALIDATE_NORMAL));                        \
    if (result != NT_SUCCESS) {                                                                        \
      WRN("NTPL errorcode: 0x%x\n", ntplInfo.u.errorData.errCode);                                     \
      if (ntplInfo.u.errorData.errBuffer[0][0]) WRN("Line1: %s\n", ntplInfo.u.errorData.errBuffer[0]); \
      if (ntplInfo.u.errorData.errBuffer[1][0]) WRN("Line2: %s\n", ntplInfo.u.errorData.errBuffer[1]); \
      if (ntplInfo.u.errorData.errBuffer[2][0]) WRN("Line3: %s\n", ntplInfo.u.errorData.errBuffer[2]); \
      DIE("NTPL error\n");                                                                             \
    }                                                                                                  \
  } while(0)

void hex(void *buf, int bufsz, const char *fmt, ...);
int my_strcmp(const char *s1, const char *s2);
int my_strncmp(const char *s1, const char *s2, size_t n);
int my_strcasecmp(const char *s1, const char *s2);
int my_strncasecmp(const char *s1, const char *s2, size_t n);

extern int _rpcerr_code;
extern char _rpcerr_message[512];

#endif
