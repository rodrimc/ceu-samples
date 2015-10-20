#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define CEU_OUT_EVT (PARAM)

static char CEU_DATA[sizeof(CEU_Main)];
static tceu_app app;

void handler (MbEvent *evt)
{
  switch (evt->type)
	{
#ifdef CEU_IN_MB_APP_INIT_DONE
		case MB_APP_INIT_DONE:
		{
			g_debug ("MB_APP_INIT_DONE received.\n");
      ceu_sys_go(&app, CEU_IN_MB_APP_INIT_DONE, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_BEGIN
		case MB_BEGIN:
		{
			g_debug ("%s has started.\n", evt->state_change.media_name);
      ceu_sys_go(&app, CEU_IN_MB_BEGIN, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_PAUSE
		case MB_PAUSE:
		{
			g_debug ("%s has paused.\n", evt->state_change.media_name);
      ceu_sys_go(&app, CEU_IN_MB_PAUSE, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_END
		case MB_END:
		{
			g_debug ("%s has ended.\n", evt->state_change.media_name);
      ceu_sys_go(&app, CEU_IN_MB_END, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_REMOVED
		case MB_REMOVED:
		{
			g_debug ("%s has been removed from pipeline.\n", 
          evt->state_change.media_name);
      ceu_sys_go(&app, CEU_IN_MB_REMOVED, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_MOUSE_BUTTON_PRESS
		case MB_MOUSE_BUTTON_PRESS:
		{
			g_debug ("%d  button has been pressed on (%d, %d).\n", 
          evt->mouse_button.button, evt->mouse_button.x, evt->mouse_button.y);
      ceu_sys_go(&app, CEU_IN_MB_MOUSE_BUTTON_PRESS, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_MOUSE_BUTTON_RELEASE
		case MB_MOUSE_BUTTON_RELEASE:
		{
			g_debug ("%d button has been released on (%d, %d).\n", 
          evt->mouse_button.button, evt->mouse_button.x, evt->mouse_button.y);
      ceu_sys_go(&app, CEU_IN_MB_MOUSE_BUTTON_PRESS, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_MOUSE_MOVE
		case MB_MOUSE_MOVE:
		{
			g_debug ("mouse position: (%d, %d)\n", evt->mouse_move.x, 
          evt->mouse_move.y);
      ceu_sys_go(&app, CEU_IN_MB_MOUSE_MOVE, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_KEY_PRESS
		case MB_KEY_PRESS:
		{
			g_debug ("key pressed: %s\n", evt->keyboard.key);
      ceu_sys_go(&app, CEU_IN_MB_KEY_PRESS, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_KEY_RELEASE
		case MB_KEY_RELEASE:
		{
			g_debug ("key released: %s\n", evt->keyboard.key);
      ceu_sys_go(&app, CEU_IN_MB_KEY_RELEASE, &evt);
			break;
		}
#endif
#ifdef CEU_IN_MB_MEDIA_SELECTION
		case MB_MEDIA_SELECTION:
		{
			g_debug ("media selected: %s\n", evt->selection.media_name);
      ceu_sys_go(&app, CEU_IN_MB_MEDIA_SELECTION, &evt);
			break;
		}
#endif
    default:
			g_debug ("Unknown event received!\n");
	}
}

int main (int argc, char* arg[])
{
  struct timespec ts_old;
  uint64_t count = 0;
  
  //s32 DT;
#ifdef CEU_WCLOCKS
  //DT = CEU_WCLOCK_INACTIVE;
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
