
/*
 * HTTP+JSON REST server.
*/

#include "sys_includes.h"
#include <microhttpd.h>
#include <jansson.h>
#include "global.h"
#include "debug.h"
#include "config.h"

#include "darknetif.h"
#include "httpjson.h"

static int
handle_image_post(const char *body, char **response)
{
  int ret=-1;
  json_t *jsonin=NULL;
  json_t *jsonout=NULL;
  json_t *jsonvar;
  json_error_t jerr;

  jsonin = json_loads(body, 0, &jerr);
  ASSERT_DO(jsonin, return -1);

  if (config.verbose >= 3) {
    char *tmpstr;
    tmpstr = json_dumps(jsonin, JSON_PRESERVE_ORDER | JSON_INDENT(4));
    DBG3("JSON REASSEMBLED: %s\n", tmpstr);
    free(tmpstr);
  }

  static struct httpjson_job job;
  memset(&job, 0, sizeof(job));
  static struct httpjson_job_reply reply;
  memset(&reply, 0, sizeof(reply));

  jsonvar = json_object_get(jsonin, "image");
  ASSERT_DO(jsonvar, goto json_parse_end);
  job.image = (char*)json_string_value(jsonvar);

  jsonvar = json_object_get(jsonin, "return_original_image");
  ASSERT_DO(jsonvar, goto json_parse_end);
  job.return_original_image = json_boolean_value(jsonvar);

  jsonvar = json_object_get(jsonin, "return_marked_image");
  ASSERT_DO(jsonvar, goto json_parse_end);
  job.return_marked_image = json_boolean_value(jsonvar);

  darknetif_process(&job, &reply);

  jsonout = json_object();
  ASSERT_DO(jsonout, goto json_parse_end);
  json_object_set_new(jsonout, "error", json_integer(reply.error));
  json_object_set_new(jsonout, "description", json_string(reply.description));
  json_object_set_new(jsonout, "org_image", json_string(reply.org_image));
  json_object_set_new(jsonout, "mark_image", json_string(reply.mark_image));
  json_object_set_new(jsonout, "predict_time", json_real(reply.predict_time));
  json_object_set_new(jsonout, "is_person", json_integer(reply.is_person));
  json_object_set_new(jsonout, "is_person_confidence", json_integer(reply.is_person_confidence));

  *response = json_dumps(jsonout, 0);
  if (config.verbose >= 3) {
    DBG3("JSON RESPONSE: %s\n", *response);
  }
  
  ret = 0;
json_parse_end:
  if (jsonin) json_decref(jsonin);
  if (jsonout) json_decref(jsonout);
  return ret;
}


#define PARSE_REQUEST_METHOD_GET (1)
#define PARSE_REQUEST_METHOD_POST (2)
static int
parse_request(const char *url, int method, const char *body, char **response)
{
  DBG3("url [%s] method [%d] body [%s]\n", url, method, body);
  if (method == PARSE_REQUEST_METHOD_POST) {
    if (handle_image_post(body, response)) {
      return 500;
    } else {
      return 200;
    }
  }
  DBG3("Nothing parsed in parse_request\n");
  return 500;
}

static int
queue_response(struct MHD_Connection *connection, char *txt, enum MHD_ResponseMemoryMode mode, int httpcode)
{
  int ret;
  struct MHD_Response *response;
  //MHD_RESPMEM_MUST_COPY , MHD_RESPMEM_PERSISTENT , MHD_RESPMEM_MUST_FREE
  response = MHD_create_response_from_buffer (strlen (txt), (void *) txt, mode);
  if (!response) {
    return MHD_NO;
  }
  ret = MHD_queue_response (connection, httpcode, response);
  MHD_destroy_response (response);
  return ret;
}

struct con_type_post {
  struct MHD_Connection *connection;
  char *post_data;
  char *response;
  int post_data_size;
};

static char *html_error_internal_error = "<html><body>INTERNAL ERROR</body></html>"; //MHD_RESPMEM_PERSISTENT
static char *html_error_method_not_allowed = "<html><body>Method not allowed. Only GET and POST is processed.</body></html>";

int answer_to_connection (void *cls, struct MHD_Connection *connection,
                          const char *url,
                          const char *method, const char *version,
                          const char *upload_data,
                          size_t *upload_data_size, void **con_cls)
{
  DBG3("HTTPJSON: url %s, method %s, version %s, upload_data [%s], upload_data_size %d\n", url, method, version, upload_data, (int)*upload_data_size);

  struct con_type_post *con_type_post;
  int http_code;

  //TODO: I don't check the media type.. It would require to use the MHD post processor to extract key=value,
  // but I simply only talk application/json back no matter what media is requested, and if I can't parse
  // input, I'll simply return an error (in application/json format)

  //Process GET requests;
  if (0 == strcmp(method, "GET")) {
    char *response=NULL;
    http_code = parse_request(url, PARSE_REQUEST_METHOD_GET, NULL, &response);
    if (response) {
      DBG2("HTTP: Response %s\n", response);
      queue_response(connection, response, MHD_RESPMEM_MUST_FREE, http_code);
    } else {
      WRN("HTTP: No Reponse generated.. Returning code 500\n");
      queue_response(connection, html_error_internal_error, MHD_RESPMEM_PERSISTENT, 500);
    }
    return MHD_YES;
  }

  //Process POST requests
  if (0 != strcmp(method, "POST")) {
    WRN("Invalid method [%s]. Only GET and POST are supported\n", method);
    queue_response(connection, html_error_method_not_allowed, MHD_RESPMEM_PERSISTENT, 405);
    return MHD_YES;
  }

  if (*con_cls == NULL) {
    /* first POST-part, which is headers only */
    INF("Receiving new job\n");
    con_type_post = calloc(1, sizeof(struct con_type_post));
    ASSERT_DO(NULL != con_type_post, return MHD_NO);
    con_type_post->post_data = calloc(1, CONFIG_POST_DATA_SIZE_MAX);
    ASSERT_DO(NULL != con_type_post->post_data, { free(con_type_post); return MHD_NO; });    
    con_type_post->connection = connection;
    *con_cls = con_type_post;
    return MHD_YES;
  }

  //con_cls is con_type_post
  con_type_post = *con_cls;
  ASSERT(NULL != con_type_post);

  if (*upload_data_size == 0) {
    /* last POST-part, parse POST data, send response, clear con_cls */
    INF("New job received. Processing it\n");
    DBG3("Post DATA: [%s]\n", con_type_post->post_data);
    http_code = parse_request(url, PARSE_REQUEST_METHOD_POST, con_type_post->post_data, &con_type_post->response);
    if (con_type_post->response) {
      DBG3("HTTP: Response %s\n", con_type_post->response);
      queue_response(con_type_post->connection, con_type_post->response, MHD_RESPMEM_MUST_FREE, http_code);
      INF("Done sending results. Clearing connection and awaiting new job\n");
    } else {
      WRN("HTTP: No Reponse generated.. Returning code 500\n");
      queue_response(con_type_post->connection, html_error_internal_error, MHD_RESPMEM_PERSISTENT, 500);
    }
    if (con_type_post->post_data) free(con_type_post->post_data);
    free(con_type_post);
    *con_cls = NULL;
    return MHD_YES;
  }

  /* copy POST data, then return to fetch next batch of POST data */
  if ((con_type_post->post_data_size + *upload_data_size) >= CONFIG_POST_DATA_SIZE_MAX) {
    ERR("POST data size exceeds %d bytes. Dropping connection\n", CONFIG_POST_DATA_SIZE_MAX);
    queue_response(con_type_post->connection, "POST exceeds data limit", MHD_RESPMEM_PERSISTENT, 413);
    if (con_type_post->post_data) free(con_type_post->post_data);
    free(con_type_post);
    *con_cls = NULL;
    return MHD_YES;
  }
  memcpy(&con_type_post->post_data[con_type_post->post_data_size], upload_data, *upload_data_size);
  con_type_post->post_data_size += *upload_data_size;
  *upload_data_size = 0; //signal to MHD that all data has been consumed
  return MHD_YES;
}

struct MHD_Daemon *http_daemon;

int httpjson_start(void)
{
  INF("Starting HTTP+JSON REST server on port %d\n", config.port);

  http_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, config.port, NULL, NULL,
                                 &answer_to_connection, NULL,
                                 //MHD_OPTION_SOCK_ADDR, (struct sockaddr)addr, //to listen on IP and PORT
                                 //MHD_OPTION_CONNECTION_LIMIT, (unsigned int)FD_SETSIZE - 4, //default is FD_SETSIZE - 4
                                 MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)300,  //default is zero (no timeout)
                                 MHD_OPTION_END);

/*
  http_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTPJSON_PORT, NULL, NULL,
                                 &answer_to_connection, NULL,
                                 MHD_OPTION_NOTIFY_COMPLETED, request_completed,
                                 NULL, MHD_OPTION_END);
*/
  ASSERT_DO(NULL != http_daemon, return -1);
  return 0;
}

int httpjson_stop(void)
{
  INF("Stopping HTTP+JSON server\n");
  MHD_stop_daemon (http_daemon);
  return 0;
}
