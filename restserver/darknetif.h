
#ifndef DARKNETIF_H
#define DARKNETIF_H

#include "httpjson.h"

int darknetif_init(void);
int darknetif_process(struct httpjson_job *job, struct httpjson_job_reply *reply);

#endif
