#ifndef XWIN_H
#define XWIN_H

#include <graph/graph.h>
#include <x/xcntl.h>
#include <x/xevent.h>
#include <x/x.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct st_xwin {
	x_t* x;
	int fd;
	void* data;
	void* g_buf;
	int   g_shmid;
	xinfo_t xinfo;
	xinfo_t xinfo_prev; //for backup the state before fullscreen/min/max.

	void (*on_close)(struct st_xwin* xwin);
	void (*on_min)(struct st_xwin* xwin);
	void (*on_resize)(struct st_xwin* xwin);
	void (*on_focus)(struct st_xwin* xwin);
	void (*on_unfocus)(struct st_xwin* xwin);
	void (*on_repaint)(struct st_xwin* xwin, graph_t* g);
	void (*on_event)(struct st_xwin* xwin, xevent_t* ev);
} xwin_t;

xwin_t*  xwin_open(x_t* xp, int x, int y, int w, int h, const char* title, int style);
int      xwin_update_info(xwin_t* x, xinfo_t* xinfo);
void     xwin_close(xwin_t* x);
int      xwin_set_visible(xwin_t* x, bool visible);
void     xwin_repaint(xwin_t* x, bool sync);
void     xwin_repaint_req(xwin_t* x);
int      xwin_resize(xwin_t* x, int dw, int dh);
int      xwin_resize_to(xwin_t* x, int w, int h);
int      xwin_move(xwin_t* xwin, int dx, int dy);
int      xwin_move_to(xwin_t* xwin, int x, int y);
int      xwin_set_display(xwin_t* xwin, uint32_t display_index);
int      xwin_call_xim(xwin_t* xwin);
int      xwin_event_handle(xwin_t* xwin, xevent_t* ev);

#ifdef __cplusplus
}
#endif

#endif
