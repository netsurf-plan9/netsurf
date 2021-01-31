#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <u.h>
#include <lib9.h>
#include <utf.h>
#include <bio.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "netsurf/browser_window.h"
#include "netsurf/plot_style.h"
#include "utils/nsurl.h"
#include "plan9/utils.h"
#include "plan9/bookmarks.h"
#include "plan9/layout.h"
#include "plan9/window.h"
#include "plan9/drawui/widget.h"
#include "plan9/drawui/data.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/window.h"

enum { Margin = 16 };

struct bookmark
{
	char *title;
	char *url;
};

struct bookmark_list
{
	struct bookmark **items;
	int size;
	int count;
};

static char *menu3str[] =
{
	"change title",
	"change url",
	"delete",
	0
};

enum
{
	Mchangetitle,
	Mchangeurl,
	Mdelete,
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
static int maxw;
static int charw;
static int lineh;
static int nlines;
static int offset;
static int lasti;

static struct bookmark_list *bookmarks;

static int done;

static void close_button_clicked(Mouse, void*)
{
	done = 1;
}

static int compute_max_width(void)
{
	int i, l, m;

	for(i = 0, m = 0; i < bookmarks->count; i++){
		l = stringwidth(text_font, bookmarks->items[i]->title);
		if(l > m)
			m = l;
	}
	return m;
}

static void bookmarks_window_init(Rectangle r)
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
	//header_bg = allocimage(display, Rect(0,0,1,1), screen->chan, 1, 0xCCCCCCFF);
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
	maxw = compute_max_width();
}

static void bookmarks_window_destroy(void)
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
	if ((n+offset) >= bookmarks->count)
		return -1;
	return n;
}

static void draw_item(int i, bool selected)
{
	struct bookmark *bookmark;
	Image *bg;
	Rectangle r;
	Point p;
	int y;

	replclipr(screen, 0, contentr);
	bookmark = bookmarks->items[offset+i];
	y = i*lineh;
	r = Rect(contentr.min.x, contentr.min.y+y, contentr.max.x, contentr.min.y+y+lineh);
	p = addpt(r.min, Pt(PADDING, (Dy(r)-text_font->height)/2));
	bg = selected ? sel_color : bg_color;
	draw(screen, r, bg, nil, ZP);
	string(screen, p, fg_color, ZP, text_font, bookmark->title);
	p.x = contentr.min.x + PADDING + maxw + 4*charw;
	string(screen, p, link_bg, ZP, text_font, bookmark->url);
}

static void bookmarks_window_draw(void)
{
	Point p;
	int i, h, y;

	replclipr(screen, 0, wr);
	draw(screen, wr, bg_color, nil, ZP);
	draw(screen, headerr, header_bg, nil, ZP);
	p = titler.min;
	p.y = headerr.min.y + (Dy(headerr) - header_font->height)/2;
	string(screen, p, header_fg, ZP, header_font, "Bookmarks");
	line(screen, Pt(wr.min.x, headerr.max.y+1), Pt(wr.max.x, headerr.max.y+1), 0, 0, 0, header_fg, ZP);
	draw(screen, scrollr, scroll_bg_color, nil, ZP);
	if (bookmarks->count > 0) {
		h = ((double)nlines/bookmarks->count) * Dy(scrollr);
		y = ((double)offset/bookmarks->count) * Dy(scrollr);
		scrposr = Rect(scrollr.min.x, scrollr.min.y+y, scrollr.max.x-1, scrollr.min.y+y+h);
	} else {
		scrposr = Rect(scrollr.min.x, scrollr.min.y, scrollr.max.x-1, scrollr.max.y);
	}
	draw(screen, scrposr, bg_color, nil, ZP);
	for(i = 0; i < nlines && offset+i < bookmarks->count; i++) {
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
	bookmarks_window_draw();
}

static void scrolldown(int off)
{
	if (offset+nlines > bookmarks->count)
		return;
	offset += off;
	if (offset+nlines > bookmarks->count)
		offset = bookmarks->count - nlines + 1;
	bookmarks_window_draw();
}

void add_bookmark(char *t, char *u)
{
	struct bookmark *b;

	b = malloc(sizeof *b);
	b->title = strdup(t);
	b->url   = strdup(u);

	if(bookmarks->count >= bookmarks->size) {
		bookmarks->size *= 2;
		bookmarks->items = realloc(bookmarks->items, bookmarks->size);
	}
	bookmarks->items[bookmarks->count++] = b;
}

void del_bookmark(int i)
{
	struct bookmark *b;
	int n;

	n = i+offset;
	b = bookmarks->items[n];
	free(b->title);
	free(b->url);
	free(b);
	bookmarks->items[n] = NULL;
	memmove(bookmarks->items+n, bookmarks->items+n+1, (bookmarks->count-n-1)*sizeof(struct bookmark*));
	bookmarks->count--;
}

void bookmarks_init(void)
{
	char *path;
	Biobuf *bp;
	char *s, *t;

	bookmarks = malloc(sizeof *bookmarks);
	bookmarks->count = 0;
	bookmarks->size	 = 64;
	bookmarks->items = malloc(bookmarks->size * sizeof(struct bookmark));
	path = userdir_file("bookmarks");
	if (access(path, R_OK) != 0)
		return;
	bp = Bopen(path, 0);
	for(;;){
		s = Brdstr(bp, '\n', 1);
		if (s == NULL)
			break;
		if (strlen(s) == 0 || s[0] == '#')
			continue;
		t = strchr(s, '\t');
		if (t == NULL) {
			free(s);
			continue;
		}
		*t = 0;
		add_bookmark(s, t+1);
		*t = '\t';
		free(s);
	}
	Bterm(bp);
}

void bookmarks_save(void)
{
	char *path;
	Biobuf *bp;
	int i;

	path = userdir_file("bookmarks");
	bp = Bopen(path, 1); /* 1=OWRITE */
	for(i = 0; i < bookmarks->count; i++)
		Bprint(bp, "%s\t%s\n", bookmarks->items[i]->title, bookmarks->items[i]->url);
	Bterm(bp);
}

void bookmarks_add(const char *title, struct browser_window *bw)
{
	add_bookmark(title, nsurl_access(browser_window_access_url(bw)));
}

void bookmarks_show(struct gui_window *gw)
{
	Event ev;
	int e, n, i, l;
	Rectangle r;
	nserror error;
	nsurl *url;
	char *u, buf[1024];
	struct bookmark *b;

	u = NULL;
	done = 0;
	lasti = -1;
	bookmarks_window_init(dwindow_get_view_rect(gw->dw));
	bookmarks_window_draw();
	while (!done) {
		if(!eqrect(sr, screen->r)) { /* resize occured */
			bookmarks_window_destroy();
			dwindow_resize(gw->dw, screen->r);
			dwindow_draw(gw->dw);
			bookmarks_window_init(dwindow_get_view_rect(gw->dw));
			bookmarks_window_draw();
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
			b = bookmarks->items[n+offset];
			if (n != lasti) {
				if (lasti != -1)
					draw_item(lasti, false);
				flushimage(display, 1);
				draw_item(n, true);
				flushimage(display, 1);
				lasti = n;
			}
			if (ev.mouse.buttons & 1) {
				u = bookmarks->items[offset+n]->url;
				done = 1;
			} else if (ev.mouse.buttons & 4) {
				i = emenuhit(3, &ev.mouse, &menu3);
				switch(i) {
				case Mchangetitle:
					strncpy(buf, b->title, strlen(b->title));
					if (eenter("Change title: ", buf, sizeof buf, &ev.mouse) > 0) {
						free(b->title);
						b->title = strdup(buf);
						l = compute_max_width();
						if (l != maxw) {
							maxw = l;
							bookmarks_window_draw();
						} else {
							draw_item(n, lasti == n);
						}
					}
					break;
				case Mchangeurl:
					strncpy(buf, b->url, strlen(b->url));
					if (eenter("Change url: ", buf, sizeof buf, &ev.mouse) > 0) {
						free(b->url);
						b->url = strdup(buf);
						draw_item(n, lasti == n);
					}
					break;
				case Mdelete:
					del_bookmark(n);
					lasti = -1;
					maxw = compute_max_width();
					bookmarks_window_draw();
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
	bookmarks_window_destroy();
	if (u != NULL) {
		error = nsurl_create(u, &url);
		if (error == NSERROR_OK) {
			browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_HISTORY,
				NULL, NULL, NULL);
			nsurl_unref(url);
		}
	}
}
