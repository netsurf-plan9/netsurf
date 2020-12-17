#include <u.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include <cursor.h>
#include <plumb.h>
#include "utils/filepath.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/utils.h"
#include "utils/file.h"
#include "utils/nsoption.h"
#include "netsurf/keypress.h"
#include "netsurf/url_db.h"
#include "netsurf/browser.h"
#include "netsurf/browser_window.h"
#include "netsurf/misc.h"
#include "netsurf/plotters.h"
#include "netsurf/netsurf.h"
#include "content/fetch.h"
#include "content/backing_store.h"
#include "desktop/search.h"
#include "plan9/bitmap.h"
#include "plan9/fetch.h"
#include "plan9/layout.h"
#include "plan9/plotter.h"
#include "plan9/schedule.h"
#include "plan9/utils.h"
#include "plan9/window.h"
#include "plan9/gui.h"
#include "plan9/clipboard.h"
#include "plan9/drawui/window.h"
#include "plan9/drawui/data.h"

static void back_button_mouse_event(Mouse, void*);
static void fwd_button_mouse_event(Mouse, void*);
static void stop_button_mouse_event(Mouse, void*);
static void reload_button_mouse_event(Mouse, void*);
static void url_entry_activated(char*, void*);
static void scrollbar_mouse_event(Mouse, void*);
static void browser_mouse_event(Mouse, void*);
static void browser_keyboard_event(int, void*);

char *menu2str[] =
{
	"search",
	"cut",
	"paste",
	"snarf",
	0 
};

enum
{
	Msearch,
	Mcut,
	Mpaste,
	Msnarf,
};
Menu menu2 = { menu2str };


char *menu3str[] =
{
	"exit",
	0
};

enum
{
	Mexit,
};

Menu menu3 = { menu3str };


char **respaths;
static struct gui_window *current = NULL;

static bool nslog_stream_configure(FILE *fptr)
{
        /* set log stream to be non-buffering */
	setbuf(fptr, NULL);

	return true;
}

static char** init_resource_paths(void)
{
	char *langv[] = { "C", 0 }; /* XXX: no lang management on plan9 */
	char **pathv;
	char **respath;

	pathv = filepath_path_to_strvec(NETSURF_RESPATH ":" NETSURF_FONTPATH);
	if(pathv == NULL) {
		return NULL;
	}
	respath = filepath_generate(pathv, langv);
	filepath_free_strvec(pathv);
	return respath;
}

static nserror init_options(int argc, char *argv[])
{
	nserror ret;
	char *options;

	ret = nsoption_init(NULL, &nsoptions, &nsoptions_default);
	if(ret != NSERROR_OK) {
		return ret;
	}
	options = filepath_find(respaths, "Choices");
	nsoption_read(options, nsoptions);
	free(options);
	nsoption_commandline(&argc, argv, nsoptions);
}

static nserror init_messages(void)
{
	nserror ret;
	char *messages;

	messages = filepath_find(respaths, "Messages");
	ret = messages_add_from_file(messages);
	free(messages);
	return ret;
}

static nserror drawui_init(int argc, char *argv[])
{
	struct browser_window *bw;
	nserror ret;
	char *addr;
	nsurl *url;

	if(initdraw(nil, nil, "netsurf") < 0) {
		fprintf(stderr, "initdraw failed\n");
		exit(1);
	}
	einit(Emouse|Ekeyboard);
	data_init();

	addr = NULL;
	if (argc > 1) {
		addr = argv[1];
	} else if ((nsoption_charp(homepage_url) != NULL) && 
	    	   (nsoption_charp(homepage_url)[0] != '\0')) {
		addr = nsoption_charp(homepage_url);
	} else {
		addr = NETSURF_HOMEPAGE;
	}
	ret = nsurl_create(addr, &url);
	if(ret == NSERROR_OK){
		ret = browser_window_create(BW_CREATE_HISTORY, url, NULL, NULL, &bw);
		nsurl_unref(url);
	}
	return ret;
}

static void drawui_run(void)
{
	enum { Eplumb = 128 };
	Plumbmsg *pm;
	Event ev;
	int e, timer;

	timer = etimer(0, SCHEDULE_PERIOD);
	eresized(0);
	eplumb(Eplumb, "web");
	for(;;){
		e = event(&ev);
		switch(e){
		default:
			if(e == timer)
				schedule_run();
			break;
		case Eplumb:
			pm = ev.v;
			if (pm->ndata > 0) {
				url_entry_activated(strdup(pm->data), current);
			}
			plumbfree(pm);
			break;
		case Ekeyboard:
			dwindow_keyboard_event(current->dw, ev);
			break;
		case Emouse:
			dwindow_mouse_event(current->dw, ev);
			break;
		}
	}
}

static void drawui_exit(int status)
{
	struct browser_window *bw = current->bw;
	current->bw = NULL;
	browser_window_destroy(bw);
	netsurf_exit();
	nsoption_finalise(nsoptions, nsoptions_default);
	nslog_finalise();
	exit(status);
}

void eresized(int new)
{
	if (new && getwindow(display, Refnone) < 0) {
		fprintf(stderr, "cannot reattach to window\n");
		exit(1);
	}
	if (current != NULL) {
		gui_window_resize(current);
	}
}

struct gui_window* gui_window_create(struct browser_window *bw)
{
	struct gui_window *gw;
	Rectangle r;

	gw = calloc(1, sizeof *gw);
	if(gw==NULL)
		return NULL;
	current = gw;
	gw->bw = bw;
	gw->dw = dwindow_create(screen->r);
	dwindow_set_back_button_mouse_callback(gw->dw, back_button_mouse_event, gw);
	dwindow_set_forward_button_mouse_callback(gw->dw, fwd_button_mouse_event, gw);
	dwindow_set_stop_button_mouse_callback(gw->dw, stop_button_mouse_event, gw);
	dwindow_set_reload_button_mouse_callback(gw->dw, reload_button_mouse_event, gw);
	dwindow_set_entry_activated_callback(gw->dw, url_entry_activated, gw);
	dwindow_set_scrollbar_mouse_callback(gw->dw, scrollbar_mouse_event, gw);
	dwindow_set_browser_mouse_callback(gw->dw, browser_mouse_event, gw);
	dwindow_set_browser_keyboard_callback(gw->dw, browser_keyboard_event, gw);
	r = dwindow_get_view_rect(gw->dw);
	gw->b = allocimage(display,  Rect(0, 0, Dx(r), Dy(r)), screen->chan, 0, DNofill);
	gui_window_redraw(gw, gw->b->r);
	return gw;
}

void gui_window_destroy(struct gui_window *gw)
{
	if (gw->b != NULL)
		freeimage(gw->b);
	dwindow_destroy(gw->dw);
	free(gw);
}

void gui_window_redraw(struct gui_window *gw, Rectangle clipr)
{
	Rectangle r;
	Point p0, p1;
	struct rect clip;
	int x, y;
	struct redraw_context ctx = {
		.interactive = true,
		.background_images = true,
		.plot = plan9_plotter_table,
		.priv = gw->b,
	};

	clipr = dwindow_rect_in_view_rect(gw->dw, clipr);
	r = dwindow_get_view_rect(gw->dw);
	clip.x0 = 0;
	clip.y0 = 0;
	clip.x1 = Dx(r);
	clip.y1 = Dy(r);
	x = dwindow_get_scroll_x(gw->dw);
	y = dwindow_get_scroll_y(gw->dw);
	browser_window_redraw(gw->bw, -x, -y, &clip, &ctx);
	if(gw->caret_height > 0) {
		p0 = addpt(gw->caret, Pt(-x, -y));
		p1 = addpt(p0, Pt(0, gw->caret_height));
		line(gw->b, p0, p1, 1, 1, 0, display->black, ZP);
	}
	replclipr(screen, 0, clipr);
	draw(screen, r, gw->b, 0, ZP);
}

void gui_window_resize(struct gui_window *gw)
{
	Rectangle r;

	dwindow_resize(gw->dw, screen->r);
	r = dwindow_get_view_rect(gw->dw);
	if (gw->b != NULL)
		freeimage(gw->b);
	gw->b = allocimage(display, Rect(0, 0, Dx(r), Dy(r)), screen->chan, 0, DNofill);
	browser_window_schedule_reformat(gw->bw);
	gui_window_redraw(gw, gw->b->r);
}

static void gui_window_scroll_y(struct gui_window *gw, int x, int y, int sy)
{
	if (browser_window_scroll_at_point(gw->bw, x, y, 0, sy) == false) {
		if (dwindow_try_scroll(gw->dw, 0, sy)) {
			gui_window_redraw(gw, gw->b->r);
		}
	}
}

static void menu2hit(struct gui_window *gw, Mouse *m)
{
	static char lastbuf[1024] = {0};
	char buf[1024] = {0};
	int n, flags;

	n = emenuhit(2, m, &menu2);
	switch (n) {
	case Msearch:
		strcpy(buf, lastbuf);
		if(eenter("Search for", buf, sizeof buf, m) > 0) {
			flags = strcmp(lastbuf, buf) == 0 ? SEARCH_FLAG_FORWARDS : SEARCH_FLAG_SHOWALL;
			browser_window_search(gw->bw, gw, flags, buf);
			strcpy(lastbuf, buf);
		} else {
			browser_window_search_clear(gw->bw);
			lastbuf[0] = 0;
		}
	case Mcut:
		browser_window_key_press(gw->bw, NS_KEY_CUT_SELECTION);
		break;
	case Mpaste:
		browser_window_key_press(gw->bw, NS_KEY_PASTE);
		break;
	case Msnarf:
		browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
		break;
	}
}

static void menu3hit(struct gui_window *gw, Mouse *m)
{
	int n;

	n = emenuhit(3, m, &menu3);
	switch (n) {
	case Mexit:
		drawui_exit(0);
	}
}

void browser_mouse_event(Mouse m, void *data)
{
	static Mouse lastm;
	static int in_sel = 0;
	struct gui_window *gw = data;
	browser_mouse_state mouse = 0;;
	Rectangle r;
	int x, y, sx, sy, lx, ly;

	r = dwindow_get_view_rect(current->dw);
	sx = dwindow_get_scroll_x(current->dw);
	sy = dwindow_get_scroll_y(current->dw);
	x = m.xy.x - r.min.x + sx;
	y = m.xy.y - r.min.y + sy;
	lx = lastm.xy.x - r.min.x + sx;
	ly = lastm.xy.y - r.min.y + sy;

	if (m.buttons && in_sel && (abs(x - lastm.xy.x) > 5 || abs(y - lastm.xy.y) > 5)) {
		if (m.buttons & 1) {
			browser_window_mouse_click(gw->bw, BROWSER_MOUSE_DRAG_1, lx, ly);
			mouse |= BROWSER_MOUSE_DRAG_ON | BROWSER_MOUSE_HOLDING_1;
		} else if (m.buttons & 2) {
			browser_window_mouse_click(gw->bw, BROWSER_MOUSE_DRAG_2, lx, ly);
			mouse |= BROWSER_MOUSE_DRAG_ON | BROWSER_MOUSE_HOLDING_2;
		}
	} else {
		in_sel = 0;
	}

	if (m.buttons == 0) {
		if((m.msec - lastm.msec < 250) && lastm.buttons & 1) {
			lastm = m;
			browser_window_mouse_click(current->bw, BROWSER_MOUSE_CLICK_1, x, y);
		} else {
			browser_window_mouse_track(current->bw, mouse, x, y);
		}
	} else if (m.buttons&1) {
		lastm = m;
		if (!in_sel) {
			in_sel = 1;
			browser_window_mouse_click(current->bw, BROWSER_MOUSE_PRESS_1, x, y);
		}
	} else if (m.buttons & 2) {
		menu2hit(gw, &m);
	} else if (m.buttons & 4) {
		menu3hit(gw, &m);
	} else if (m.buttons&8) {
		gui_window_scroll_y(current, x, y, -100);
	} else if (m.buttons&16) {
		gui_window_scroll_y(current, x, y, 100);
	}
	browser_window_mouse_track(current->bw, mouse, x, y);
}

int getnskey(int k)
{
	int n;

	switch (k) {
	case Kdel:
	case Kbs:
		n = NS_KEY_DELETE_LEFT;
		break;
	case Knack:
		n = NS_KEY_DELETE_LINE_START;
		break;
	case Ksoh:
	case Khome:
		n = NS_KEY_LINE_START;
		break;
	case Kenq:
	case Kend:
		n = NS_KEY_LINE_END;
		break;
	case Kleft:
		n = NS_KEY_LEFT;
		break;
	case Kright:
		n = NS_KEY_RIGHT;
		break;
	case Kup:
		n = NS_KEY_UP;
		break;
	case Kdown:
		n = NS_KEY_DOWN;
		break;
	default:
		n = k;
		break;
	}
	return n;	
}

void browser_keyboard_event(int k, void *data)
{
	struct gui_window *gw = data;
	Rectangle r;

	if (browser_window_key_press(gw->bw, getnskey(k))) {
		return;
	}

	r = dwindow_get_view_rect(gw->dw);
	switch(k) {
	case Kdel:
		drawui_exit(0);
		break;
	case Kpgup:
		gui_window_scroll_y(gw, 0, 0, -Dy(r));
		break;
	case Kpgdown:
		gui_window_scroll_y(gw, 0, 0, Dy(r));
		break;
	case Kup:
		gui_window_scroll_y(gw, 0, 0, -100);
		break;
	case Kdown:
		gui_window_scroll_y(gw, 0, 0, 100);
		break;
	case Khome:
		gui_window_scroll_y(gw, 0, 0, INT_MIN/2);
		break;
	case Kend:
		gui_window_scroll_y(gw, 0, 0, INT_MAX/2);
		break;
	case Kesc:
	case Knack:
		browser_window_search_clear(gw->bw);
		break;
	case Kstx:
		dwindow_focus_url_bar(gw->dw);
		break;
	}
}

void scrollbar_mouse_event(Mouse m, void *data)
{
	struct gui_window *gw = data;
	Rectangle r;
	int n, x, y, sx, sy, dy;

	r = dwindow_get_view_rect(gw->dw);
	sx = dwindow_get_scroll_x(gw->dw);
	sy = dwindow_get_scroll_y(gw->dw);
	x = sx + m.xy.x - r.min.x;
	y = sy + m.xy.y - r.min.y;
	dy = m.xy.y - r.min.y;
	if (m.buttons&1) {
		gui_window_scroll_y(gw, x, y, -dy);
	} else if (m.buttons & 4) {
		gui_window_scroll_y(gw, x, y, dy);
	} else if (m.buttons & 2) {
		dy = (m.xy.y - r.min.y) * dwindow_get_extent_y(gw->dw) / Dy(r);
		gui_window_scroll_y(gw, x, y, dy - sy);
	}
}

void back_button_mouse_event(Mouse m, void *data)
{
	if (m.buttons&1 == 0) {
		return;
	}
	if (browser_window_back_available(current->bw)) {
		browser_window_history_back(current->bw);
	}
}

void fwd_button_mouse_event(Mouse m, void *data)
{
	if (m.buttons&1 == 0) {
		return;
	}

	if (browser_window_forward_available(current->bw)) {
		browser_window_history_forward(current->bw);
	}
}

void stop_button_mouse_event(Mouse m, void *data)
{
	if (m.buttons&1 == 0) {
		return;
	}
	browser_window_stop(current->bw);

}

void reload_button_mouse_event(Mouse m, void *data)
{
	if (m.buttons&1 == 0) {
		return;
	}
	browser_window_reload(current->bw, true);
}

void url_entry_activated(char *text, void *data)
{
	nserror error;
	nsurl *url;

	if (text == NULL || text[0] == 0) {
		return;
	}
	error = nsurl_create(text, &url);
	if (error == NSERROR_OK) {
		browser_window_navigate(current->bw, url, NULL, BW_NAVIGATE_HISTORY,
			NULL, NULL, NULL);
		nsurl_unref(url);
		free(text);
	}
}

int
main(int argc, char *argv[])
{
	struct stat sb;
	struct browser_window *bw;
	nsurl *url;
	nserror ret;
	struct netsurf_table plan9_table = {
		.misc = plan9_misc_table,
		.window = plan9_window_table,
		.fetch = plan9_fetch_table,
		.bitmap = plan9_bitmap_table,
		.layout = plan9_layout_table,
		.clipboard = plan9_clipboard_table,
	};

	if (stat("/mnt/web", &sb) != 0 || !S_ISDIR(sb.st_mode)) {
		fprintf(stderr, "webfs not started\n");
		exit(1);
	}

	ret = netsurf_register(&plan9_table);
	if(ret != NSERROR_OK) {
		sysfatal("netsurf_register failed");
	}

	nslog_init(nslog_stream_configure, &argc, argv);

	respaths = init_resource_paths();
	if(respaths == NULL) {
		sysfatal("unable to initialize resource paths");
	}

	ret = init_options(argc, argv);
	if(ret != NSERROR_OK) {
		sysfatal("unable to initialize options");
	}

	ret = init_messages();
	if(ret != NSERROR_OK) {
		fprintf(stderr, "unable to load messages translations\n");
	}

	ret = netsurf_init(NULL);
	if(ret != NSERROR_OK) {
		sysfatal("netsurf initialization failed: %s", messages_get_errorcode(ret));
	}

	ret = drawui_init(argc, argv);
	if(ret != NSERROR_OK) {
		fprintf(stderr, "netsurf plan9 initialization failed: %s", messages_get_errorcode(ret));
	} else {
		drawui_run();
	}

	netsurf_exit();
	nsoption_finalise(nsoptions, nsoptions_default);
	nslog_finalise();
	return 0;
}
