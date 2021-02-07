#include <u.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "netsurf/browser_window.h"
#include "netsurf/plot_style.h"
#include "netsurf/misc.h"
#include "netsurf/netsurf.h"
#include "netsurf/url_db.h"
#include "utils/nsurl.h"
#include "plan9/history.h"
#include "plan9/layout.h"
#include "plan9/window.h"
#include "plan9/drawui/widget.h"
#include "plan9/drawui/data.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/window.h"

enum { Margin = 16 };

struct hentry
{
	struct nsurl *url;
	char *title;
	char *host;
	time_t last_visit;
};

struct hentry_list
{
	struct hentry **items;
	int size;
	int count;
};

static char *menu3str[] =
{
	"delete",
	0,
};

enum
{
	Mdelete
};

static Menu menu3 = { menu3str };

static Rectangle sr;
static Rectangle wr;
static Rectangle headerr;
static Rectangle titler;
static Rectangle closer;
static Rectangle scrollr;
static Rectangle scrposr;
static Rectangle listr;
static Rectangle contentr;
static dbutton *close_button;
static Image *b;
static Image *header_fg;
static Image *header_bg;
static Image *link_bg;
static Font *header_font;
static Font *text_font;
static int charw;
static int lineh;
static int nlines;
static int offset;
static int lasti;

static struct hentry_list *hentries;

static int done;


static void close_button_clicked(Mouse, void*)
{
	done = 1;
}

static void history_window_init(Rectangle r)
{
	struct plot_font_style fstyle;

	sr = screen->r;
	wr = Rect(sr.min.x, r.min.y, sr.max.x, sr.max.y);
	headerr = Rect(wr.min.x, wr.min.y, wr.max.x, wr.min.y + 24);
	closer = rectaddpt(Rect(0, 0, 16, 16), wr.min);
	closer.min.x = headerr.max.x - PADDING - 16;
	closer.min.y += (Dy(headerr)-16)/2;
	closer.max.x = headerr.max.x - PADDING;
	closer.max.y += (Dy(headerr)-16)/2;
	titler = Rect(wr.min.x + 2*PADDING, wr.min.y, closer.min.x - PADDING, closer.max.y);
	scrollr = Rect(wr.min.x, headerr.max.y + 2, wr.min.x + SCROLL_WIDTH + 1, wr.max.y);
	listr  = Rect(scrollr.max.x, titler.max.y + 1, wr.max.x, wr.max.y);
	contentr = insetrect(listr, Margin);
	close_button = dbutton_create(close_button_icon, focus_color);
	dbutton_set_rect(close_button, closer);
	dbutton_set_mouse_callback(close_button, close_button_clicked, NULL);
	replclipr(screen, 0, sr);
	header_fg = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x55AAAAFF);
	header_bg = allocimagemix(display, DPalebluegreen, DWhite);
	link_bg = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x999999FF);
	b = allocimage(display, sr, screen->chan, 0, DNofill);
	if (b != NULL) {
		draw(b, sr, screen, nil, sr.min);
	}
	fstyle.size = 10*PLOT_STYLE_SCALE;
	fstyle.weight = 0;
	fstyle.family = PLOT_FONT_FAMILY_SANS_SERIF;
	text_font = getfont(&fstyle);
	fstyle.size = 12*PLOT_STYLE_SCALE;
	fstyle.weight = 500;
	header_font = getfont(&fstyle);
	lineh = PADDING + text_font->height + PADDING;
	nlines = Dy(listr)/lineh;
	offset = 0;
	charw = stringwidth(text_font, " ");
}

static void history_window_destroy(void)
{
	replclipr(screen, 0, sr);
	if (b != NULL) {
		draw(screen, b->r, b, nil, b->r.min);
		freeimage(b);
	}
	freeimage(link_bg);
	freeimage(header_bg);
	free(close_button);
	flushimage(display, 1);
}

static int index_at(Point p)
{
	int n;

	if (!ptinrect(p, contentr))
		return -1;
	n = (p.y - contentr.min.y) / lineh;
	if ((n+offset) >= hentries->count)
		return -1;
	return n;
}
	
static char ellipsis[] = "...";

static void draw_item(int i, bool selected)
{
	struct hentry *h;
	Image *bg;
	Rectangle r;
	Point p;
	int y, w, ew, n, l;
	Rune u;
	char *s;

	h = hentries->items[offset+i];
	w = stringwidth(text_font, h->host);
	ew = stringwidth(text_font, ellipsis);
	replclipr(screen, 0, contentr);
	y = i*lineh;
	r = Rect(contentr.min.x, contentr.min.y+y, contentr.max.x, contentr.min.y+y+lineh);
	p = addpt(r.min, Pt(PADDING, (Dy(r)-text_font->height)/2));
	bg = selected ? sel_color : bg_color;
	draw(screen, r, bg, nil, ZP);
	l = strlen(h->title);
	n = 0;
	while (n < l) {
		u = utf8_to_ucs4(h->title + n, l - n);
		n = utf8_next(h->title, l, n);
		if((p.x+ew+4*charw+w+Margin)>=r.max.x) {
			p = string(screen, p, fg_color, ZP, text_font, ellipsis);
			break;
		} else
			p = runestringn(screen, p, fg_color, ZP, text_font, &u, 1);
	}
	p.x += 4*charw;
	string(screen, p, link_bg, ZP, text_font, h->host);
}

static void history_window_draw(void)
{
	Point p;
	int i, h, y;

	replclipr(screen, 0, wr);
	draw(screen, wr, bg_color, nil, ZP);
	draw(screen, headerr, header_bg, nil, ZP);
	p = titler.min;
	p.y = headerr.min.y + (Dy(headerr) - header_font->height)/2;
	string(screen, p, header_fg, ZP, header_font, "History");
	line(screen, Pt(wr.min.x, headerr.max.y+1), Pt(wr.max.x, headerr.max.y+1), 0, 0, 0, header_fg, ZP);
	draw(screen, scrollr, scroll_bg_color, nil, ZP);
	if (hentries->count > 0) {
		h = ((double)nlines/hentries->count) * Dy(scrollr);
		y = ((double)offset/hentries->count) * Dy(scrollr);
		scrposr = Rect(scrollr.min.x, scrollr.min.y+y, scrollr.max.x-1, scrollr.min.y+y+h);
	} else {
		scrposr = Rect(scrollr.min.x, scrollr.min.y, scrollr.max.x-1, scrollr.max.y);
	}
	draw(screen, scrposr, bg_color, nil, ZP);
	for(i = 0; i < nlines && offset+i < hentries->count; i++) {
		draw_item(i, false);
	}
	dbutton_draw(close_button);
	flushimage(display, 1);
}

static void scrollup(int off)
{
	if (offset == 0)
		return;
	offset -= off;
	if (offset < 0)
		offset = 0;
	history_window_draw();
}

static void scrolldown(int off)
{
	if (offset+nlines > hentries->count)
		return;
	offset += off;
	if (offset+nlines > hentries->count)
		offset = hentries->count - nlines + 1;
	history_window_draw();
}

static bool history_add(const struct nsurl *url, const struct url_data *data)
{
	struct hentry *e;
	char *h;
	lwc_string *s;
	int l;

	if (data->last_visit == 0)
		return true;
	e = calloc(1, sizeof *e);
	if(e == NULL) {
		sysfatal("out of memory: %r");
	}
	e->url = url;
	e->last_visit = data->last_visit;
	e->title = strdup(data->title != NULL ? data->title : "<No title>");
	s = nsurl_get_component(url, NSURL_HOST);
	if (s == NULL)
		s = nsurl_get_component(url, NSURL_SCHEME); /* file url */
	l = lwc_string_length(s) + 1;
	e->host = malloc(l);
	strncpy(e->host, lwc_string_data(s), l - 1);
	e->host[l-1] = 0;
	lwc_string_unref(s);
	if (hentries->count >= hentries->size) {
		hentries->size *= 2;
		hentries->items = realloc(hentries->items, hentries->size * sizeof(struct hentry));
	}
	hentries->items[hentries->count++] = e;
	return true;
}

static void history_del(int i)
{
	struct hentry *h;
	int n;

	n = i + offset;
	h = hentries->items[n];
	free(h->title);
	free(h->host);
	free(h);
	hentries->items[n] = NULL;
	memmove(hentries->items+n, hentries->items+n+1, (hentries->count-n-1)*sizeof(struct hentry*));
	hentries->count--;
}

int timecmp(const void *a, const void *b)
{
	struct hentry *ha, *hb;

	ha = *(struct hentry**)a;
	hb = *(struct hentry**)b;
	return hb->last_visit - ha->last_visit;
}

void history_init(void)
{
	hentries = malloc(sizeof *hentries);
	hentries->count = 0;
	hentries->size = 256;
	hentries->items = malloc(hentries->size * sizeof(struct hentry));
	urldb_iterate_entries(history_add);
	if (hentries->count > 0)
		qsort(hentries->items, hentries->count - 1, sizeof(hentries->items[0]), timecmp);
}

void history_destroy(void)
{
	int i;

	for(i = 0; i < hentries->count; i++) {
		free(hentries->items[i]->title);
		free(hentries->items[i]->host);
		free(hentries->items[i]);
	}
	free(hentries->items);
	free(hentries);
}

void history_show(struct gui_window *gw)
{
	Event ev;
	int e, n, i, l;
	Rectangle r;
	nserror error;
	nsurl *url;
	char buf[1024];

	url = NULL;
	done = 0;
	lasti = -1;
	history_init();
	history_window_init(dwindow_get_view_rect(gw->dw));
	history_window_draw();
	while (!done) {
		if(!eqrect(sr, screen->r)) { /* resize occured */
			history_window_destroy();
			dwindow_resize(gw->dw, screen->r);
			dwindow_draw(gw->dw);
			history_window_init(dwindow_get_view_rect(gw->dw));
			history_window_draw();
		}
		e = eread(Ekeyboard|Emouse, &ev);
		switch (e) {
		default:
			done = 1;
			break;
		case Ekeyboard:
			switch(ev.kbdc) {
			case Kesc:
				done = 1;
				break;
			case Khome:
				scrollup(-(INT_MIN/2));
				break;
			case Kend:
				scrolldown(INT_MAX/2);
				break;
			case Kpgup:
				scrollup(nlines);
				break;
			case Kpgdown:
				scrolldown(nlines);
				break;
			}
			break;
		case Emouse:
			dbutton_mouse_event(close_button, ev);
			n = index_at(ev.mouse.xy);
			if (n == -1) {
				if (lasti != -1) {
					draw_item(lasti, false);
					lasti = -1;
				}
				continue;
			}
			if (n != lasti) {
				if (lasti != -1)
					draw_item(lasti, false);
				flushimage(display, 1);
				draw_item(n, true);
				flushimage(display, 1);
				lasti = n;
			}
			if (ev.mouse.buttons & 1) {
				url = hentries->items[offset+n]->url;
				done = 1;
			} else if (ev.mouse.buttons & 4) {
				i = emenuhit(3, &ev.mouse, &menu3);
				switch(i) {
				case Mdelete:
					history_del(n);
					lasti = -1;
					history_window_draw();
					break;
				}
			} else if (ev.mouse.buttons & 8) {
				scrollup(10);
			} else if (ev.mouse.buttons & 16) {
				scrolldown(10);
			}
			break;
		}
	}
	history_window_destroy();
	history_destroy();
	if (url != NULL) {
		browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_HISTORY,
				NULL, NULL, NULL);
	}
}
