#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>  

#define TIME_FREQ 1 //frequency in which the update_ceu_time callback 
                    //is called (ms)


#define ceu_out_assert(v) ceu_sys_assert(v)
void ceu_sys_assert (int v) {
    assert(v);
}

#define ceu_out_log(m,s) ceu_sys_log(m,s)
void ceu_sys_log (int mode, long s) {
    switch (mode) {
        case 0:
            printf("%s", (char*)s);
            break;
        case 1:
            printf("%lX", s);
            break;
        case 2:
            printf("%ld", s);
            break;
    }
}

typedef struct _InConnection
{
  GSocketService *service;
  GSocketConnection *connection;
  GObject *source_object;
  gpointer user_data;
} InConnection;

typedef struct _InputMessage
{
  GSocketConnection *connection;
  gsize buff_len;
  gssize read_count;
  char *buff;
  int to;
} InputMessage;

void
async_read(GInputStream *input_stream, gsize buff_len, int handler_id);

void
connect_socket_service_incoming_signal(GSocketService *service);

#define CEU_OUT_EVT (PARAM)

#include "_ceu_app.c"

static char CEU_DATA[sizeof(CEU_Main)];
static tceu_app app;
static GMainLoop *main_loop;
static uint64_t old_dt;

gboolean
incoming_callback  (GSocketService *service,
                    GSocketConnection *connection,
                    GObject *source_object,
                    gpointer user_data)

{
#ifdef CEU_IN_INCOMING
  InConnection *conn = (InConnection *) malloc (sizeof (InConnection));
  conn->service = service;
  conn->connection = connection;
  conn->source_object = source_object;
  conn->user_data = user_data;

  ceu_sys_go(&app, CEU_IN_INCOMING, &conn);
  free (conn);
#endif

  return FALSE;
}

void 
read_done (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
#ifdef CEU_IN_READ_DONE
  GInputStream *stream = G_INPUT_STREAM (source_object);
  GError *error = NULL;
  InputMessage *message = user_data;
  message->read_count = g_input_stream_read_finish (stream, res, &error);
  
  ceu_sys_go(&app, CEU_IN_READ_DONE, &message);
  free (message->buff);
  free (message);
#endif
}

void
async_read (GInputStream *input_stream, gsize buff_len, int handler_id)
{
  InputMessage *input = g_new0 (InputMessage, 1);
  input->buff_len = buff_len;
  input->buff = (char *) malloc (input->buff_len *  sizeof (char));
  input->to = handler_id;
  
  g_input_stream_read_async (input_stream, input->buff, input->buff_len, 
      G_PRIORITY_DEFAULT, NULL, read_done, input);
}

void
connect_socket_service_incoming_signal(GSocketService *service)
{
  g_signal_connect (service, "incoming", 
      G_CALLBACK (incoming_callback), NULL);
}


gboolean
update_ceu_time (gpointer data)
{
  if (app.isAlive)
  {
    uint64_t dt = g_get_monotonic_time () - old_dt;
#ifdef CEU_WCLOCKS
    ceu_sys_go(&app, CEU_IN__WCLOCK, &dt);
#endif
#ifdef CEU_ASYNCS
    ceu_sys_go(&app, CEU_IN__ASYNC, NULL);
#endif
    old_dt += dt;

    return G_SOURCE_CONTINUE;
  }
  else
  {
    g_main_loop_quit (main_loop);
    return G_SOURCE_REMOVE;
  }
}

int main (int argc, char* arg[])
{
  uint64_t last_time = g_get_monotonic_time();
  main_loop = g_main_loop_new (NULL, FALSE);

  app.data = (tceu_org*) &CEU_DATA;
  app.init = &ceu_app_init;

  app.init (&app);

  old_dt = g_get_monotonic_time ();

  g_timeout_add (TIME_FREQ, update_ceu_time, NULL);

  g_main_loop_run (main_loop);
  
  printf ("All done\n");
  return app.ret;
}
