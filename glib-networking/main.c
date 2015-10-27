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

int main (int argc, char* arg[])
{
  uint64_t last_time = g_get_monotonic_time();

  app.data = (tceu_org*) &CEU_DATA;
  app.init = &ceu_app_init;

  app.init (&app);

  while(app.isAlive) 
  {
    struct timespec ts_now;
    uint64_t dt;
    dt = g_get_monotonic_time () - last_time;
    last_time += dt;

#ifdef CEU_WCLOCKS
    ceu_sys_go(&app, CEU_IN__WCLOCK, &dt);
#endif
#ifdef CEU_ASYNCS
    ceu_sys_go(&app, CEU_IN__ASYNC, NULL);
#endif
  }
  printf ("All done\n");
  return app.ret;
}
