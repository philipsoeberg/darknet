
#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_POST_DATA_SIZE_MAX (1024*1024*10)

extern
struct config
{
    int port; //5050
    char *listen; //0.0.0.0
    int verbose; //0

    char *datacfg; //cfg/coco.data
    char *cfgfile; //cfg/yolov2-tiny.cfg
    char *weightfile; //yolov2-tiny.weights
    char *labelfile; //data/coco.names
    
    float thresh; //0.5
    float hier_thresh; //0.5
    

} config;

int config_cmd_args_parse(int argc, char *argv[]);
int config_dump_running_config();

#endif //CONFIG_H
