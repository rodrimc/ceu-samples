#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <mb.h>

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

#include "_ceu_app.c"

#ifndef CEU_IN_MB_BEGIN
  #define CEU_IN_MB_BEGIN 0xFFFFFFC
#endif

#ifndef CEU_IN_MB_PAUSE
  #define CEU_IN_MB_PAUSE 0xFFFFFD
#endif

#ifndef CEU_IN_MB_END
  #define CEU_IN_MB_END 0xFFFFFE
#endif

#ifndef CEU_IN_MB_REMOVED
  #define CEU_IN_MB_REMOVED 0xFFFFFF
#endif

#define CEU_OUT_EVT (PARAM)

static char CEU_DATA[sizeof(CEU_Main)];
static tceu_app app;

void handler (MbMediaEvent *evt)
{
  switch (evt->evt)
	{
		case MB_BEGIN:
		{
			g_print ("%s has started.\n", evt->media->name);
      ceu_sys_go(&app, CEU_IN_MB_BEGIN, &evt);
			break;
		}
		case MB_PAUSE:
		{
			g_print ("%s has paused.\n", evt->media->name);
      ceu_sys_go(&app, CEU_IN_MB_PAUSE, &evt);
			break;
		}
		case MB_END:
		{
			g_print ("%s has ended.\n", evt->media->name);
      ceu_sys_go(&app, CEU_IN_MB_END, &evt);
			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);
      ceu_sys_go(&app, CEU_IN_MB_REMOVED, &evt);
			break;
		}
		default:
			g_printerr ("Unknown event received!\n");
	}
}

int main (int argc, char* arg[])
{
  struct timespec ts_old;
  uint64_t count = 0;
  
  //s32 DT;
#ifdef CEU_WCLOCKS
  DT = CEU_WCLOCK_INACTIVE;
#endif

  app.data = (tceu_org*) &CEU_DATA;
  app.init = &ceu_app_init;

  app.init (&app);

  mb_register_handler(handler);

  clock_gettime(CLOCK_REALTIME, &ts_old);

  while(app.isAlive) 
  {
    struct timespec ts_now;
    s32 dt;
    do
    {
      clock_gettime(CLOCK_REALTIME, &ts_now);
      dt = (ts_now.tv_sec  - ts_old.tv_sec)*1000000 +
        (ts_now.tv_nsec - ts_old.tv_nsec)/1000;
    }
    while (dt == 0);
    
    ts_old = ts_now;
    count ++;
    
#ifdef CEU_WCLOCKS
    ceu_sys_go(&app, CEU_IN__WCLOCK, &dt);
#endif
#ifdef CEU_ASYNCS
    ceu_sys_go(&app, CEU_IN__ASYNC, NULL);
#endif
  }
  mb_clean_up();
  printf ("All done\n");
  return app.ret;
}
