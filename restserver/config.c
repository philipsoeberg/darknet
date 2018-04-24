
#include "sys_includes.h"
#include "config.h"
#include "debug.h"

struct config config = {
    .port = 5050,
    .listen = "127.0.0.1",
    .verbose = 0,

    .datacfg = "cfg/coco.data",
    .cfgfile = "cfg/yolov2-tiny.cfg",
    .weightfile = "yolov2-tiny.weights",
    .labelfile = "data/coco.names",
    
    .thresh = .5,
    .hier_thresh = .5,
};

void config_cmd_args_print_usage(void)
{
  INF("Usage:"
"\n" "  -p, --port            server port (default: 5050)"
"\n" "  -l, --listen          listen IP (default: 127.0.0.1)"
"\n"
"\n" "      --datacfg         Darknet datacfg (default: cfg/coco.data)"
"\n" "      --cfgfile         Darknet cfgfile (default: cfg/yolov2-tiny.cfg)"
"\n" "      --weightfile      Darknet weightfile (default: yolov2-tiny.weights)"
"\n" "      --labelfile       Darknet labelfile (Default: data/coco.names)"
"\n"
"\n" "  -v  --verbose         Increase verbose. Up to 3 times."
"\n" "  -?, --help            Give this help list"
"\n"  " "
  );
}

int config_dump_running_config()
{
  INF("Running configuration:\n");
  INF(" --port       %d\n", config.port);
  INF(" --listen     %s\n", config.listen);
  INF(" --datacfg    %s\n", config.datacfg);
  INF(" --cfgfile    %s\n", config.cfgfile);
  INF(" --weightfile %s\n", config.weightfile);
  INF(" --labelfile  %s\n", config.labelfile);
  INF(" --verbose    %d\n", config.verbose);
  return 0;
}

int config_cmd_args_parse(int argc, char *argv[])
{
  int c;
  
  if (argc < 2) {
    //config_cmd_args_print_usage();
    //return -1;
    return 0;
  }

  enum {
    OPT_DATACFG,
    OPT_CFGFILE,
    OPT_WEIGHTFILE,
    OPT_LABELFILE,
  };

  while (1) {
    static struct option long_options[] = {
      {"port",         required_argument, 0, 'p'},
      {"listen",       required_argument, 0, 'l'},
      {"datacfg",      required_argument, 0, OPT_DATACFG},
      {"cfgfile",      required_argument, 0, OPT_CFGFILE},
      {"weightfile",   required_argument, 0, OPT_WEIGHTFILE},
      {"labelfile",    required_argument, 0, OPT_LABELFILE},
      {"verbose",      no_argument, 0, 'V'},
      {"help",         no_argument, 0, '?'},
      {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    char *tailptr;

    c = getopt_long ( argc, argv, "p:l:?v", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1) break;

    switch (c) {
      case 'p':
        config.port = strtol(optarg, &tailptr, 10);
        if ((optarg == tailptr) || (*tailptr != 0x0)) { ERR("Error: Invalid value for argument --port\n"); return -1; }
        if (config.port&~0xFFFF) { ERR("Error: Invalid value for argument --port\n"); return -1; }
        break;
      case 'l':
        config.listen = optarg;
        break;
      case OPT_DATACFG:
        config.datacfg = optarg;
        break;
      case OPT_CFGFILE:
        config.cfgfile = optarg;
        break;
      case OPT_WEIGHTFILE:
        config.weightfile = optarg;
        break;
      case OPT_LABELFILE:
        config.labelfile = optarg;
        break;
      case 'v':
        config.verbose++;
        break;
      case '?':
        config_cmd_args_print_usage();
        return -1;
      default:
        return 0;
    }
  }

  if (optind != argc) {
    ERR("Error: Unknown argument %s\n", argv[optind]);
    return -1;
  }
  
  return 0;
}