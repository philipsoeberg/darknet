
#ifndef GLOBAL_H
#define GLOBAL_H

#define VERSION  "1.0"

/*
 * Contains global variables (No, global variables are NOT bad. They are extremely fast compared to function execution!)
 */

/*
 * Overall application status. This variable is accessed in while(1) loops!
 * Visibility need not be instant.
 */
extern volatile enum app_is_alive { APP_TERMINATE, APP_INIT, APP_RUN } app_is_alive;

#define HTTPJSON_PORT (8500)
#define DATA_SINK_PORT (8501)
#define NUM_TX_PORTS (4)
#define NUM_IDLE_SEGMENTS_UNTIL_TX (3) // The number of segments received until the TX can start
#define MAX_SUB_STREAMS (16) //don't go above 16 here. If, you need to change the portmap compiler which use 16bit filter masks
#define RINGBUF_ORDER (27)

#define FILEREPLAY_PATH_PREFIX "/mnt/capture/replaystore/"

enum stream_format { UNKNOWN, PCAP, PCAPNG };

#endif
