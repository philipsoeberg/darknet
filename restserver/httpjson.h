
#ifndef HTTPJSON_H
#define HTTPJSON_H

#include "config.h"

struct httpjson_job {
  char *image; //base64 of image
  int return_original_image;
  int return_marked_image;
};

struct httpjson_job_reply {
  int error;            /* Fault code. Must be zero for OK */
  char *description;      /* Fault text, or OK if no error */
  
  char org_image[CONFIG_POST_DATA_SIZE_MAX+1]; //base64 or null of original image
  char mark_image[CONFIG_POST_DATA_SIZE_MAX+1]; //base64 or null of marked image
  
  char predict_time[10];

  char is_person; //true/false if "person" was detected
  int is_person_confidence;  //level of confidence

};

/* Server start/stop */
int httpjson_start(void);
int httpjson_stop(void);

#endif
