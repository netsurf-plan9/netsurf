#include <u.h>
#include <lib9.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
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
#include "netsurf/cookie_db.h"
#include "netsurf/browser.h"
#include "netsurf/browser_window.h"
#include "netsurf/content.h"
#include "netsurf/misc.h"
#include "netsurf/bitmap.h"
#include "netsurf/plotters.h"
#include "netsurf/netsurf.h"
#include "content/fetch.h"
#include "content/backing_store.h"
#include "desktop/search.h"
#include "desktop/searchweb.h"
#include "desktop/browser_history.h"
#include "plan9/menus.h"
#include "plan9/bookmarks.h"
#include "plan9/search.h"
#include "plan9/searchweb.h"
#include "plan9/download.h"
#include "plan9/history.h"
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
#include "plan9/webfs.h"

static void back_button_mouse_event(Mouse, void*);
static void fwd_button_mouse_event(Mouse, void*);
static void stop_button_mouse_event(Mouse, void*);
static void reload_button_mouse_event(Mouse, void*);
static void url_entry_activated(char*, void*);
static void scrollbar_mouse_event(Mouse, void*);
static void browser_mouse_event(Mouse, void*);
static void browser_keyboard_event(int, void*);
void visit_url(struct gui_window *gw, char *text);

char **respaths;
static struct gui_window *current = NULL;
static bool unhide_on_plumb;
static bool altmode;

static bool nslog_stream_configure(FILE *fptr)
{
        /* set log stream to be non-buffering */
	setbuf(fptr, NULL);

	return true;
}

static void init_log(bool verbose)
{
	int argc;
	char *argv[] = { argv0, "-v", 0 };

	argc = 1;
	if (verbose) {
		++argc;
	}
	nslog_init(nslog_stream_configure, &argc, argv);
	if (log_debug) {
		nslog_stream_configure(stderr);
	}
}

static char** init_resource_paths(void)
{
	char *langv[] = { "C", 0 }; /* XXX: no lang management on plan9 */
	char **pathv;
	char **respath;

	pathv = filepath_path_to_strvec(NETSURF_RESPATH);
	if(pathv == NULL) {
		return NULL;
	}
	respath = filepath_generate(pathv, langv);
	filepath_free_strvec(pathv);
	return respath;
}

static nserror init_options_defaults(struct nsoption_s *defaults)
{
	nsoption_set_charp(homepage_url, "about:welcome");
	nsoption_set_int(max_fetchers, 44);
	nsoption_set_int(max_fetchers_per_host, 20);
	nsoption_set_bool(animate_images, false);
	nsoption_set_bool(enable_javascript, false);
	nsoption_set_bool(search_url_bar, true);
	nsoption_set_int(search_provider, 18); //ddg
	return NSERROR_OK;
}

static nserror init_options(void)
{
	nserror ret;
	char *options;

	ret = nsoption_init(init_options_defaults, &nsoptions, &nsoptions_default);
	if(ret != NSERROR_OK) {
		return ret;
	}
	options = userdir_file("options");
	nsoption_read(options, nsoptions);
	free(options);
	return NSERROR_OK;
}

static void save_options(void)
{
	nserror ret;
	char *path;

	path = userdir_file("options");
	ret = nsoption_write(path, nsoptions, nsoptions_default);
	if (ret != NSERROR_OK) {
		fprintf(stderr, "unable to save options: %s\n", messages_get_errorcode(ret));
	}
	free(path);
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

static nserror init_history(void)
{
	nserror ret;
	char *path;

	path = userdir_file("history");
	if (access(path, F_OK) < 0) {
		return NSERROR_OK;
	}
	ret = urldb_load(path);
	free(path);
	return ret;
}

static void save_history(void)
{
	nserror ret;
	char *path;

	path = userdir_file("history");
	ret = urldb_save(path);
	if (ret != NSERROR_OK) {
		fprintf(stderr, "unable to save history: %s\n", messages_get_errorcode(ret));
	}
	free(path);
}

static nserror init_cookies(void)
{
	nserror ret;
	char *path;

	path = userdir_file("cookies");
	nsoption_setnull_charp(cookie_file, strdup(path));
	nsoption_setnull_charp(cookie_jar, strdup(path));
	if (access(path, F_OK) < 0) {
		return NSERROR_OK;
	}
	urldb_load_cookies(path);
	free(path);
	return NSERROR_OK;
}

static void save_cookies(void)
{
	char *path;

	path = userdir_file("cookies");
	urldb_save_cookies(path);
	free(path);
}

static nserror init_websearch(void)
{
	nserror ret;
	char *path;
	path = filepath_find(respaths, "SearchEngines");
	ret = search_web_init(path);
	if (ret == NSERROR_OK && nsoption_int(search_provider) != 0)
		search_web_select_provider(nsoption_int(search_provider));
	free(path);
	return ret;
}

static nserror drawui_init(int argc, char *argv[])
{
	struct browser_window *bw;
	nserror ret;
	char *addr, *path;
	nsurl *url;
	size_t len;

	if(initdraw(nil, nil, "netsurf") < 0) {
		fprintf(stderr, "initdraw failed\n");
		exit(1);
	}
	einit(Emouse|Ekeyboard);
	data_init();

	addr = NULL;
	if (argc > 0) {
		if (access(argv[0], F_OK) == F_OK) {
			path = file_fullpath(argv[0]);
			if (path != NULL) {
				len = SLEN("file://") + strlen(path) + 1;
				addr = malloc(len*sizeof(char));
				if(addr != NULL)
					snprintf(addr, len, "file://%s", path);
				free(path);
			}
		} else {
			if(is_url(argv[0]))
				addr = argv[0];
			else{
				len = SLEN("http://") + strlen(argv[0]) + 1;
				addr = malloc(len*sizeof(char));
				if(addr != NULL)
					snprintf(addr, len, "http://%s", argv[0]);
			}
		}
	}
	if (addr != NULL) {
		/* file url */
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

static bool plumbed_to_page(char *s)
{
	nsurl *url;
	nserror error;
	size_t l;

	if(page_accept_file(s)) {
		error = nsurl_create(s, &url);
		if (error == NSERROR_OK) {
			browser_window_navigate(current->bw, url, NULL, BW_NAVIGATE_DOWNLOAD,
				NULL, NULL, NULL);
			nsurl_unref(url);
		}
		return true;
	}
	return false;
}

static void gui_window_unhide(void)
{
	int fd;

	fd = open("/mnt/wsys/wctl", O_WRONLY);
	if(fd < 0)
		return;
	write(fd, "unhide", 6);
	close(fd);
}

static void drawui_run(void)
{
	enum { Eplumb = 128 };
	Plumbmsg *pm;
	Event ev;
	int e, t, i, s;
	ulong keys;
	fd_set rs, ws, es;
	int maxfd;

	keys = (Emouse|Ekeyboard|Eplumb);
	eresized(0);
	eplumb(Eplumb, "web");
	for(;;){
		t = schedule_run();
		_SLEEP(10);
		while(ecanread(keys)) {
			e = eread(keys, &ev);
			switch(e){
			case Eplumb:
				pm = ev.v;
				if (pm->ndata > 0) {
					if(!plumbed_to_page(pm->data)) {
						if(unhide_on_plumb)
							gui_window_unhide();
						visit_url(current, strdup(pm->data));
					}
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
			if (!ecanread(keys))
				flushimage(display, 1);
		}
	}
}

void drawui_exit(int status)
{
	struct browser_window *bw;

	if (current != NULL) {
		bw = current->bw;
		current->bw = NULL;
		browser_window_destroy(bw);
	}
	save_cookies();
	save_history();
	search_web_finalise();
	netsurf_exit();
	save_options();
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
	gui_window_resize(current);
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
	if(altmode)
		dwindow_toggle_altdisplay(gw->dw);
	dwindow_set_back_button_mouse_callback(gw->dw, back_button_mouse_event, gw);
	dwindow_set_forward_button_mouse_callback(gw->dw, fwd_button_mouse_event, gw);
	dwindow_set_stop_button_mouse_callback(gw->dw, stop_button_mouse_event, gw);
	dwindow_set_reload_button_mouse_callback(gw->dw, reload_button_mouse_event, gw);
	dwindow_set_entry_activated_callback(gw->dw, url_entry_activated, gw);
	dwindow_set_scrollbar_mouse_callback(gw->dw, scrollbar_mouse_event, gw);
	dwindow_set_browser_mouse_callback(gw->dw, browser_mouse_event, gw);
	dwindow_set_browser_keyboard_callback(gw->dw, browser_keyboard_event, gw);
	r = dwindow_get_view_rect(gw->dw);
	gw->b = allocimage(display, Rect(0, 0, Dx(r), Dy(r)), XBGR32, 0, DWhite);
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
	Point p;
	struct rect clip;
	int x, y;
	struct redraw_context ctx = {
		.interactive = true,
		.background_images = true,
		.plot = plan9_plotter_table,
		.priv = gw->b,
	};

	if (browser_window_redraw_ready(gw->bw) == false) {
		return;
	}
	clip.x0 = clipr.min.x;
	clip.y0 = clipr.min.y;
	clip.x1 = clipr.max.x;
	clip.y1 = clipr.max.y;
	clipr = dwindow_rect_in_view_rect(gw->dw, clipr);
	r = dwindow_get_view_rect(gw->dw);
	x = dwindow_get_scroll_x(gw->dw);
	y = dwindow_get_scroll_y(gw->dw);
	browser_window_redraw(gw->bw, -x, -y, &clip, &ctx);
	if(gw->caret_height > 0) {
		p = addpt(gw->caret, Pt(-x, -y+1));
		draw(gw->b, rectaddpt(tick->r, p), tick, 0, ZP);
	}
	replclipr(screen, 0, clipr);
	draw(screen, r, gw->b, 0, ZP);
}

void gui_window_resize(struct gui_window *gw)
{
	Rectangle r;

	dwindow_resize(gw->dw, screen->r);
	r = dwindow_get_view_rect(gw->dw);
	freeimage(gw->b);
	gw->b = allocimage(display, Rect(0, 0, Dx(r), Dy(r)), XBGR32, 0, DWhite);
	browser_window_schedule_reformat(gw->bw);
	gui_window_redraw(gw, gw->b->r);
	dwindow_draw(gw->dw);
}

static void gui_window_scroll(struct gui_window *gw, int x, int y, int sx, int sy)
{
	if (browser_window_scroll_at_point(gw->bw, x, y, sx, sy) == false) {
		if (dwindow_try_scroll(gw->dw, sx, sy)) {
			gui_window_redraw(gw, gw->b->r);
		}
	}
}

void browser_mouse_event(Mouse m, void *data)
{
	static Mouse lastm;
	static int in_sel = 0;
	struct gui_window *gw = data;
	browser_mouse_state mouse = 0;;
	Rectangle r;
	int x, y, sx, sy, lx, ly, dy;
	struct browser_window_features features;
	browser_editor_flags eflags;
	nserror err;

	gw->m = m;
	r = dwindow_get_view_rect(current->dw);
	sx = dwindow_get_scroll_x(current->dw);
	sy = dwindow_get_scroll_y(current->dw);
	x = m.xy.x - r.min.x + sx;
	y = m.xy.y - r.min.y + sy;
	lx = lastm.xy.x - r.min.x + sx;
	ly = lastm.xy.y - r.min.y + sy;

	if (m.buttons && in_sel && (abs(x - lastm.xy.x) > 5 || abs(y - lastm.xy.y) > 5)) {
		if (m.buttons & 1) {
			if (m.buttons != (1|2)) {
				browser_window_mouse_click(gw->bw, BROWSER_MOUSE_DRAG_1, lx, ly);
				mouse |= BROWSER_MOUSE_DRAG_ON | BROWSER_MOUSE_HOLDING_1;
			}
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
			browser_window_mouse_click(gw->bw, BROWSER_MOUSE_CLICK_1, x, y);
		} else {
			browser_window_mouse_track(gw->bw, mouse, x, y);
		}
	} else if (m.buttons&1) {
		lastm = m;
		if (!in_sel) {
			in_sel = 1;
			browser_window_mouse_click(gw->bw, BROWSER_MOUSE_PRESS_1, x, y);
		} else {
			browser_window_mouse_track(gw->bw, mouse, x, y);
			eflags = browser_window_get_editor_flags(gw->bw);
			if (m.buttons == (1|2)) {
				/* surprinsigly, for readonly selection the editor flags
				   only become valid after the second mouse track */
				browser_window_mouse_track(gw->bw, mouse, x, y);
				eflags = browser_window_get_editor_flags(gw->bw);
				if (eflags & BW_EDITOR_CAN_CUT)
					browser_window_key_press(gw->bw, NS_KEY_CUT_SELECTION);
				else if (eflags & BW_EDITOR_CAN_COPY)
					browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
			} else if (m.buttons == (1|4)) {
				if (eflags & BW_EDITOR_CAN_PASTE)
					browser_window_key_press(gw->bw, NS_KEY_PASTE);
			}
		}
	} else if (m.buttons & 2) {
		err = browser_window_get_features(gw->bw, x, y, &features);
		menu2hit(gw, &m, (err == NSERROR_OK) ? &features : NULL);
	} else if (m.buttons & 4) {
		menu3hit(gw, &m);
	} else if (m.buttons & 8) {
		dy = m.xy.y - r.min.y;
		gui_window_scroll(gw, x, y, 0, -dy);
	} else if (m.buttons & 16) {
		dy = m.xy.y - r.min.y;
		gui_window_scroll(gw, x, y, 0, dy);
	}
	browser_window_mouse_track(current->bw, mouse, x, y);
}

int getnskey(int k)
{
	int n;

	switch (k) {
	case Kdel:
		n = NS_KEY_DELETE_RIGHT;
		break;
	case Kbs:
		n = NS_KEY_DELETE_LEFT;
		break;
	case Ketb:
		n = NS_KEY_DELETE_WORD_LEFT;
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
	char buf[1024] = {0};

	if (browser_window_key_press(gw->bw, getnskey(k))) {
		return;
	}

	r = dwindow_get_view_rect(gw->dw);
	switch(k) {
	case 'g':
		if(dwindow_is_altdisplay(gw->dw) == 0)
			break;
		if(eenter("Go to:", buf, sizeof(buf), &gw->m))
			visit_url(gw, strdup(buf));
		break;
	case Kpgup:
		gui_window_scroll(gw, 0, 0, 0, -Dy(r));
		break;
	case Kpgdown:
		gui_window_scroll(gw, 0, 0, 0, Dy(r));
		break;
	case Kup:
		gui_window_scroll(gw, 0, 0, 0, -100);
		break;
	case Kdown:
		gui_window_scroll(gw, 0, 0, 0, 100);
		break;
	case Khome:
		gui_window_scroll(gw, 0, 0, 0, INT_MIN/2);
		break;
	case Kend:
		gui_window_scroll(gw, 0, 0, 0, INT_MAX/2);
		break;
	case Kleft:
		gui_window_scroll(gw, 0, 0, -100, 0);
		break;
	case Kright:
		gui_window_scroll(gw, 0, 0, 100, 0);
		break;
	case Kesc:
	case Knack:
		search_reset(gw);
		break;
	case Kstx:
		if(dwindow_is_altdisplay(gw->dw) == 1)
			break;
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
		gui_window_scroll(gw, x, y, 0, -dy);
	} else if (m.buttons & 4) {
		gui_window_scroll(gw, x, y, 0, dy);
	} else if (m.buttons & 2) {
		dy = (m.xy.y - r.min.y) * dwindow_get_extent_y(gw->dw) / Dy(r);
		gui_window_scroll(gw, x, y, 0, dy - sy);
	}
}

struct hentry_list
{
	struct history_entry **entries;
	int size;
	int count;
};

bool history_enumerate(const struct browser_window *bw,
					int x0, int y0, int x1, int y1,
					const struct history_entry *entry,
					void *user_data)
{
	struct hentry_list *l;

	l = user_data;
	if (l->count >= l->size) {
		l->size *= 1.5;
		l->entries = realloc(l->entries, l->size * sizeof(struct history_entry*));
	}
	l->entries[l->count++] = entry;
	return true;
}

void show_back_history_menu(struct gui_window *gw, Mouse m)
{
	struct hentry_list *l;
	struct nsurl *u;
	char **items;
	Menu menu;
	int n;

	l = malloc(sizeof *l);
	l->entries = calloc(16, sizeof *(l->entries));
	l->size = 16;
	l->count = 0;
	browser_window_history_enumerate_back(gw->bw, history_enumerate, l);
	if (l->count <= 0)
		goto Error;
	items = calloc(l->count + 1, sizeof *items);
	for (n = 0; n < l->count; n++) {
		items[n] = browser_window_history_entry_get_title(l->entries[n]);
	}
	menu.item = items;
	n = emenuhit(3, &m, &menu);
	if (n >= 0) {
		u = browser_window_history_entry_get_url(l->entries[n]);
		browser_window_navigate(gw->bw, u, NULL, BW_NAVIGATE_HISTORY,
			NULL, NULL, NULL);
	}
	free(items);
Error:
	free(l->entries);
	free(l);
}

void back_button_mouse_event(Mouse m, void *data)
{
	struct gui_window *gw;

	gw = data;
	if (m.buttons&1) {
		if (browser_window_back_available(gw->bw)) {
			browser_window_history_back(gw->bw, false);
		}
	} else if (m.buttons&4) {
		show_back_history_menu(gw, m);
	}
}

void fwd_button_mouse_event(Mouse m, void *data)
{
	if (m.buttons&1 == 0) {
		return;
	}

	if (browser_window_forward_available(current->bw)) {
		browser_window_history_forward(current->bw, false);
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

bool is_url(char *text)
{
	return 
		strncmp(text, "http://", 7) == 0 ||
		strncmp(text, "https://", 8) == 0 ||
		strncmp(text, "file://", 7) == 0 ||
		strncmp(text, "about:", 6) == 0 ||
		strncmp(text, "resource:", 9) == 0;
}

void visit_url(struct gui_window *gw, char *text)
{
	nserror error;
	nsurl *url;
	int mode;

	if (text == NULL || text[0] == 0) {
		return;
	}
	url = NULL;
	mode = nsoption_bool(search_url_bar) ? SEARCH_WEB_OMNI_SEARCHONLY : SEARCH_WEB_OMNI_NONE;
	if (mode == SEARCH_WEB_OMNI_SEARCHONLY && is_url(text))
		mode = SEARCH_WEB_OMNI_NONE;
	error = search_web_omni(text, mode, &url);
	if (error == NSERROR_OK) {
		esetcursor(&waitcursor);
		browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_HISTORY,
			NULL, NULL, NULL);
		nsurl_unref(url);
		free(text);
	}
}

void url_entry_activated(char *text, void *data)
{
	struct gui_window *gw;

	gw = data;
	visit_url(gw, text);
}

static nserror launch_url(const nsurl *url)
{
	char *s;

	s = nsurl_access(url);
	if (strncmp(s, "mailto:", 7) == 0)
		s += 7;
	if (send_to_plumber(s) < 0) {
		return NSERROR_NO_FETCH_HANDLER;
	}
	return NSERROR_OK;
}

static struct gui_misc_table misc_table = {
	.schedule = misc_schedule,
	.launch_url = launch_url,
};

int
main(int argc, char *argv[])
{
	struct stat sb;
	struct browser_window *bw;
	nsurl *url;
	nserror ret;
	char *path;
	char *cachedir;
	bool verbose;
	struct netsurf_table plan9_table = {
		.misc = &misc_table,
		.window = plan9_window_table,
		.fetch = plan9_fetch_table,
		.bitmap = plan9_bitmap_table,
		.layout = plan9_layout_table,
		.clipboard = plan9_clipboard_table,
		.download = plan9_download_table,
		.search = plan9_search_table,
		.llcache = filesystem_llcache_table,
	};

	cachedir = NULL;
	unhide_on_plumb = false;
	verbose = false;
	ARGBEGIN {
	case 'a':
		altmode = true;
		break;
	case 'c':
		cachedir = "/tmp/nscache";
		break;
	case 'u':
		unhide_on_plumb = true;
		break;
	case 'd':
		log_debug = true;
		break;
	case 'v':
		verbose = true;
		break;
	default:
		fprintf(stderr, "usage: %s [-acudv] [url]\n", argv0);
		exit(1);
	} ARGEND

	setfcr(getfcr() & ~(FPPDBL|FPINVAL|FPZDIV|FPOVFL));
	if (access("/mnt/web/clone", F_OK) != 0) {
		fprintf(stderr, "webfs not started\n");
		exit(1);
	}

	ret = netsurf_register(&plan9_table);
	if(ret != NSERROR_OK) {
		sysfatal("netsurf_register failed: %s\n", messages_get_errorcode(ret));
	}

	init_log(verbose);

	respaths = init_resource_paths();
	if(respaths == NULL) {
		sysfatal("unable to initialize resource paths");
	}

	ret = init_options();
	if(ret != NSERROR_OK) {
		sysfatal("unable to initialize options: %s\n", messages_get_errorcode(ret));
	}

	ret = init_messages();
	if(ret != NSERROR_OK) {
		fprintf(stderr, "unable to load messages translations: %s\n", messages_get_errorcode(ret));
	}

	ret = init_history();
	if(ret != NSERROR_OK) {
		fprintf(stderr, "unable to initialize history: %s\n", messages_get_errorcode(ret));
	}

	ret = init_cookies();
	if(ret != NSERROR_OK) {
		fprintf(stderr, "unable to initialize cookies: %s\n", messages_get_errorcode(ret));
	}

	if (cachedir != NULL && (stat(cachedir, &sb) != 0 || !S_ISDIR(sb.st_mode))) {
		mkdir(cachedir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}
	ret = netsurf_init(cachedir);
	if(ret != NSERROR_OK) {
		sysfatal("netsurf initialization failed: %s\n", messages_get_errorcode(ret));
	}

	webfs_register();

	ret = init_websearch();
	if(ret != NSERROR_OK) {
		fprintf(stderr, "unable to initialize web search: %s\n", messages_get_errorcode(ret));
	}

	bookmarks_init();

	ret = drawui_init(argc, argv);
	if(ret != NSERROR_OK) {
		fprintf(stderr, "netsurf plan9 initialization failed: %s", messages_get_errorcode(ret));
	} else {
		drawui_run();
	}
	
	drawui_exit(0);
	return 0;
}
