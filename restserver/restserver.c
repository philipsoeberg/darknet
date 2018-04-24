
#include "sys_includes.h"
#include "httpjson.h"
#include "global.h"
#include "debug.h"
#include "config.h"

#include "darknetif.h"


volatile enum app_is_alive app_is_alive;

// Unix signal handler.
static void catch_signal(int sig) {
  if (sig == SIGTRAP) { return; } //<-- Ignore it. GDB will catch this, but we do not need it for normal runtime (it's thrown by die_assert() while staying in context)
  if (sig == SIGTERM) { INF("Caught %s. Forcing application termination.\n", strsignal(sig)); exit(EXIT_FAILURE); }
  if (!app_is_alive) {
    WRN("Caught %s for the second time.. Forcing SIGTERM\n", strsignal(sig));
    kill(0, SIGTERM);
    return;
  }
  INF("Caught %s. Graceful shutdown of application..\n", strsignal(sig));
  app_is_alive=APP_TERMINATE;
}

int main(int argc, char *argv[])
{
  int i;
  int result;

  INF("DARKNET RESTful Daemon v" VERSION " startup\n");
  if (config_cmd_args_parse(argc, argv)) exit(0);

  if (config.verbose) {
    WRN("Verbose level %d activated. Expect performance penalty.\n", config.verbose);
  }

  // Register signals
  signal(SIGTERM, catch_signal);  //<-- Catch Termination Signal (eg kill 9 <pid>)
  signal(SIGINT, catch_signal);   //<-- Catch keyboard interrupt (eg CTRL-C)
  signal(SIGTRAP, catch_signal);  //<-- Catch SIGTRAP but ingore them. Used to trap to GDB if gdb is attached
  
  //Set app to INIT
  app_is_alive = APP_INIT;

  /* Dump running config */
  config_dump_running_config();

  //Create threads.
  INF("Create HTTP+JSON thread\n");
  if (httpjson_start()) DIE("Failed creating HTTP+JSON server\n");

  //init darknetif
  darknetif_init();

  //Execute all threads
  app_is_alive = APP_RUN;

  INF("App is up and running\n");

  while(app_is_alive) {
    //Do nothing..
    sleep(1);
  }

  INF("App is shutting down\n");

  //Stop threads
  INF("Shutdown HTTP+JSON thread\n");
  if (httpjson_stop()) DIE("Failed stopping HTTP+JSON server\n");

  INF("App terminated successfully\n");

  return 0;
}
