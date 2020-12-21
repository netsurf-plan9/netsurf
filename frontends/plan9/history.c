#include <u.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "netsurf/browser_window.h"
#include "netsurf/misc.h"
#include "netsurf/netsurf.h"
#include "netsurf/url_db.h"
#include "utils/nsurl.h"
#include "plan9/history.h"
#include "plan9/drawui/widget.h"
#include "plan9/drawui/data.h"
#include "plan9/drawui/button.h"

struct hentry
{
	struct nsurl *url;
	struct url_data *data;
};

static Rectangle sr;
static Rectangle wr;
static Rectangle r;
static Rectangle titler;
static Rectangle buttonr;
static Rectangle scrollr;
static Rectangle scrposr;
static Rectangle listr;
static dbutton *close_button;
static Image *b;
static Image *headercol;
static Image *urlcol;
static int charw;
static int lineh;
static int nlines;
static int offset;

static struct hentry **entries;
static int nentries;
static int size;

static struct nsurl *selurl;
static int done;

static void close_button_clicked(Mouse, void*)
{
	done = 1;
}

static void history_window_init(void)
{
	int x, y, w, h;

	sr = screen->r;	
	wr = insetrect(sr, 20);
	r = insetrect(wr, 2);
	x = r.min.x;
	y = r.min.y;
	w = Dx(r);
	h = Dy(r);
	buttonr = Rect(0, 0, ICON_SIZE, ICON_SIZE);
	buttonr.min.x = r.max.x - (PADDING+ICON_SIZE);
	buttonr.min.y = r.min.y+PADDING/2;
	buttonr.max.x = buttonr.min.x + ICON_SIZE;
	buttonr.max.y = buttonr.min.y + ICON_SIZE;
	titler = Rect(r.min.x, r.min.y, buttonr.min.x - 1, buttonr.max.y);
	scrollr = Rect(x, titler.max.y + 2, x + SCROLL_WIDTH + 1, y + h);
	listr = Rect(scrollr.max.x, titler.max.y + 1, r.max.x, r.max.y);
	close_button = dbutton_create(close_button_icon, focus_color);
	dbutton_set_rect(close_button, buttonr);
	dbutton_set_mouse_callback(close_button, close_button_clicked, NULL);
	replclipr(screen, 0, wr);
	headercol = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xEEEEEEFF);
	urlcol = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0x999999FF);
	b = allocimage(display, wr, screen->chan, 0, DNofill);
	if (b != NULL) {
		draw(b, wr, screen, nil, wr.min);
	}
	lineh = font->height + 2;
	nlines = Dy(listr)/lineh;
	offset = 0;
	charw = stringwidth(font, " ");
}

static void history_window_destroy(void)
{
	replclipr(screen, 0, sr);
	if (b != NULL) {
		draw(screen, b->r, b, nil, b->r.min);
		freeimage(b);
	}
	freeimage(headercol);
	freeimage(urlcol);
	free(close_button);
	flushimage(display, 1);
}

static void history_window_draw(void)
{
	Point p;
	int i, h, y, n, l;
	char *url;

	replclipr(screen, 0, wr);
	draw(screen, wr, bg_color, nil, ZP);
	border(screen, wr, 2, focus_color, ZP);
	replclipr(screen, 0, r);
	//draw(screen, Rpt(r.min, Pt(r.max.x, titler.max.y)), headercol, nil, ZP);
	p = Pt(titler.min.x + PADDING, titler.min.y + PADDING/2);
	string(screen, p, fg_color, ZP, font, "History");
	line(screen, Pt(r.min.x, titler.max.y + 1), Pt(r.max.x, titler.max.y + 1), 0, 0, 0, fg_color, ZP);
	draw(screen, scrollr, scroll_bg_color, nil, ZP);
	if (nentries > 0) {
		h = ((double)nlines/nentries)*Dy(scrollr);
		y = ((double)offset/nentries)*Dy(scrollr);
		scrposr = Rect(scrollr.min.x, scrollr.min.y+y, scrollr.max.x-1, scrollr.min.y+y+h);
	} else {
		scrposr = Rect(scrollr.min.x, scrollr.min.y, scrollr.max.x-1, scrollr.max.y);
	}
	draw(screen, scrposr, bg_color, nil, ZP);
	p = addpt(listr.min, Pt(PADDING, PADDING));
	for(i = 0; i < nlines && offset+i < nentries; i++) {
		p = string(screen, p, fg_color, ZP, font, entries[offset+i]->data->title);
		p.x += charw;
		url = nsurl_access(entries[offset+i]->url);
		l = strlen(url);
		n = (listr.max.x - charw - p.x) / charw - 2;
		p = string(screen, p, urlcol, ZP, font, "[");
		if (n >= l) {
			p = string(screen, p, urlcol, ZP, font, url);
		} else {
			p = stringn(screen, p, urlcol, ZP, font, url, n-3);
			p = string(screen, p, urlcol, ZP, font, "...");
		}
		p = string(screen, p, urlcol, ZP, font, "]");
		p.x = listr.min.x + PADDING;
		p.y += lineh;
	}
	dbutton_draw(close_button);
	flushimage(display, 1);
}

static bool history_add(const struct nsurl *url, const struct url_data *data)
{
	struct hentry **t, *e;

	if (data->last_visit == 0)
		return true;
	if(nentries >= size) {
		size *= 1.5;
		t = realloc(entries, size * sizeof(struct hentry*));
		if(t == NULL) {
			sysfatal("realloc: %r");
		}
		entries = t;
	}
	e = calloc(1, sizeof *e);
	if(e == NULL) {
		sysfatal("out of memory: %r");
	}
	e->url = url;
	e->data = data;
	entries[nentries++] = e;
	return true;
}

int timecmp(const void *a, const void *b)
{
	struct hentry *ha, *hb;

	ha = *(struct hentry**)a;
	hb = *(struct hentry**)b;
	return hb->data->last_visit - ha->data->last_visit;
}

void history_init(void)
{
	int i;
	char d[20];

	nentries = 0;
	size = 32;
	entries = calloc(size, sizeof *entries);
	if(entries == NULL) {
		sysfatal("out of memory: %r");
	}
	urldb_iterate_entries(history_add);
	qsort(entries, nentries - 1, sizeof(struct hentry*), timecmp);
/*
	for(i = 0; i < history->n; i++) {
		strftime(d, sizeof d, "%Y-%m-%d %H:%M:%S", gmtime(&history->e[i]->data->last_visit));
DBG(">> %s (%s)", history->e[i]->data->title, d);
	}
*/
}

void history_destroy(void)
{
	free(entries);
}

static void scrollup(int off)
{
	if(offset == 0)
		return;
	offset -= off;
	if(offset < 0)
		offset = 0;
	history_window_draw();
}

static void scrolldown(int off)
{
	if(offset+nlines > nentries)
		return;
	offset += off;
	history_window_draw();
}

struct nsurl* ehistory(const struct browser_window *bw)
{
	Event ev;
	int e, n;
	Rectangle r;

	done = 0;
	selurl = NULL;
	history_init();
	history_window_init();
	history_window_draw();
	while (!done) {
		if (!eqrect(sr, screen->r)) { /* resize occured */
			history_window_destroy();
			history_window_init();
			history_window_draw();
		}
		e = eread(Ekeyboard|Emouse, &ev);
		switch(e) {
		default:
			done = 1;
			break;
		case Ekeyboard:
			switch(ev.kbdc) {
			case Kesc:
				done = 1;
				break;
			case Kpgup:
				scrollup(nlines);
				break;
			case Kpgdown:
				scrolldown(nlines);
				break;
			case Khome:
				offset = 0;
				history_window_draw();
				break;
			}
			break;
		case Emouse:
			dbutton_mouse_event(close_button, ev);
			if ((ev.mouse.buttons & 1) && ptinrect(ev.mouse.xy, listr)) {
				n = (ev.mouse.xy.y - listr.min.y) / lineh;
				if (n < nentries) {
					selurl = entries[n]->url;
					done = 1;
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
	return selurl;
}
