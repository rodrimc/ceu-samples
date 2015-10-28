#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>  

#define TICK 100


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

void
connect_service_signal(GSocketService *service);

#include "_ceu_app.c"

#define CEU_OUT_EVT (PARAM)

static char CEU_DATA[sizeof(CEU_Main)];
static tceu_app app;
static GMainLoop *main_loop;
static uint64_t old_dt;

typedef struct
{
  GSocketService *service;
  GSocketConnection *connection;
  GObject *source_object;
  gpointer user_data;
} IncomingData;

gboolean
incoming_callback  (GSocketService *service,
                    GSocketConnection *connection,
                    GObject *source_object,
                    gpointer user_data)
{
  IncomingData data;
  data.service = service;
  data.connection = connection;
  data.source_object = source_object;
  data.user_data = user_data;
#ifdef CEU_IN_INCOMING
  ceu_sys_go(&app, CEU_IN_INCOMING, &data);
#endif
  return FALSE;
}

void
connect_service_signal(GSocketService *service)
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

  g_timeout_add (0.1, update_ceu_time, NULL);

  g_main_loop_run (main_loop);
  
  printf ("All done\n");
  return app.ret;
}
