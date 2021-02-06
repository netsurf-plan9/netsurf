#include <u.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "utils/errors.h"
#include "utils/nscolour.h"
#include "utils/nsoption.h"
#include "utils/nsurl.h"
#include "netsurf/bitmap.h"
#include "netsurf/browser_window.h"
#include "netsurf/misc.h"
#include "netsurf/netsurf.h"
#include "netsurf/url_db.h"
#include "desktop/searchweb.h"
#include "plan9/searchweb.h"
#include "plan9/bitmap.h"
#include "plan9/plotter.h"
#include "plan9/drawui/widget.h"
#include "plan9/drawui/data.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/entry.h"

static void searchweb_window_init(void);
static void searchweb_window_destroy(void);
static void searchweb_window_draw(void);

static Rectangle sr;
static Rectangle wr;
static Rectangle r;
static Rectangle menur;
static Rectangle titler;
static Rectangle buttonr;
static Rectangle iconr;
static Rectangle entryr;
static dbutton *menu_button;
static dbutton *close_button;
static dentry *search_entry;
static Image *b;

static struct nsurl *selurl;
static int done;

static char* provider;
static int provider_index;

static void close_button_clicked(Mouse, void*)
{
	done = 1;
}

static void menu_button_clicked(Mouse m, void*)
{
	nserror err;
	ssize_t iter;
	int i;
	char *name;
	char *items[64] = {0};
	Menu menu = { items };

	i = 0;
	for(iter = search_web_iterate_providers(0, &name);
	    iter != -1;
	    iter = search_web_iterate_providers(iter, &name)) {
		items[i++] = name;
	}
	i = emenuhit(1, &m, &menu);
	if (i >= 0) {
		err = search_web_select_provider(i);
		if (err == NSERROR_OK) {
			nsoption_set_int(search_provider, i);
			provider_index = i;
			provider = items[i];
			searchweb_window_draw();
		}
	}
}

static void search_entry_activated(char *text, void*)
{
	nserror err;
	
	done = 1;
	err = search_web_omni(text, SEARCH_WEB_OMNI_SEARCHONLY, &selurl);
	if (err != NSERROR_OK) {
		fprintf(stderr, "unable to create search url: %s\n", messages_get_errorcode(err));
		selurl = NULL;
	}
}

static void searchweb_window_init(void)
{
	int x, y, w, h;

	h = PADDING/2 + ICON_SIZE /* button */
	  + 2 /* gap after title bar */
	  + PADDING
	  + PADDING + 2 + font->height + 2 + PADDING /* search entry */
	  + PADDING /* bottom gap */
	  ;
	y = (Dy(screen->r) - h) / 2;
	sr = screen->r;
	wr = Rect(sr.min.x + 50, sr.min.y + y, sr.max.x - 50, sr.min.y + y + h);
	r = insetrect(wr, 2);
	menur.min.x = r.min.x + PADDING;
	menur.min.y = r.min.y + PADDING/2;
	menur.max.x = menur.min.x + ICON_SIZE;
	menur.max.y = menur.min.y + ICON_SIZE;
	buttonr.min.x = r.max.x - (PADDING+ICON_SIZE);
	buttonr.min.y = r.min.y + PADDING/2;
	buttonr.max.x = buttonr.min.x + ICON_SIZE;
	buttonr.max.y = buttonr.min.y + ICON_SIZE;
	titler = Rect(menur.max.x + 2, r.min.y, buttonr.min.x - 1, buttonr.max.y);
	iconr = Rect(r.min.x + PADDING, menur.max.y + PADDING + PADDING + 2, r.min.x + PADDING + ICON_SIZE, menur.max.y + PADDING + PADDING + 2 + ICON_SIZE);
	entryr = Rect(iconr.max.x + PADDING, menur.max.y + 2*PADDING, r.max.x - PADDING, r.max.y - PADDING);
	menu_button = dbutton_create(menu_button_icon, focus_color);
	dbutton_set_rect(menu_button, menur);
	dbutton_set_mouse_callback(menu_button, menu_button_clicked, NULL);
	close_button = dbutton_create(close_button_icon, focus_color);
	dbutton_set_rect(close_button, buttonr);
	dbutton_set_mouse_callback(close_button, close_button_clicked, NULL);
	search_entry = dentry_create();
	dentry_set_rect(search_entry, entryr);
	dentry_set_activated_callback(search_entry, search_entry_activated, NULL);
	replclipr(screen, 0, wr);
	b = allocimage(display, wr, screen->chan, 0, DNofill);
	if (b != NULL) {
		draw(b, wr, screen, nil, wr.min);
	}
}

static void searchweb_window_destroy(void)
{
	replclipr(screen, 0, sr);
	if (b != NULL) {
		draw(screen, b->r, b, nil, b->r.min);
		freeimage(b);
	}
	free(close_button);
	free(search_entry);
}

static void searchweb_window_draw(void)
{
	Point p;
	int i, h, y;
	nserror err;
	struct bitmap *bitmap;
	Image *icon;

	replclipr(screen, 0, wr);
	draw(screen, wr, bg_color, nil, ZP);
	border(screen, wr, 2, focus_color, ZP);
	replclipr(screen, 0, r);
	p = Pt(titler.min.x + PADDING, titler.min.y + PADDING/2);
	p = string(screen, p, fg_color, ZP, font, "Search ");
	string(screen, p, fg_color, ZP, font, provider);
	line(screen, Pt(r.min.x, titler.max.y + 1), Pt(r.max.x, titler.max.y + 1), 0, 0, 0, fg_color, ZP);
	err = search_web_get_provider_bitmap(&bitmap);
	if (err == NSERROR_OK && bitmap != NULL) {
		icon = getimage(bitmap, 16, 16);
	}
	if (icon != NULL) {
		draw(screen, iconr, icon, nil, ZP);
	} else {
		draw(screen, iconr, search_icon, nil, ZP);
	}
	dbutton_draw(menu_button);
	dbutton_draw(close_button);
	dentry_draw(search_entry);
	dentry_set_focused(search_entry, false);
}

static void searchweb_init(void)
{
	provider_index = nsoption_int(search_provider);
	search_web_iterate_providers(provider_index, &provider);
	search_web_select_provider(provider_index);
}

static void searchweb_destroy(void)
{
}

struct nsurl* esearchweb(const struct browser_window *bw)
{
	Event ev;
	int e, n;

	done = 0;
	selurl = NULL;
	searchweb_init();
	searchweb_window_init();
	searchweb_window_draw();
	while (!done) {
		if (!eqrect(sr, screen->r)) { /* resize occured */
			searchweb_window_destroy();
			searchweb_window_init();
			searchweb_window_draw();
		}
		e = eread(Ekeyboard|Emouse, &ev);
		switch(e) {
		default:
			done = 1;
			break;
		case Ekeyboard:
			dentry_keyboard_event(search_entry, ev);
			switch(ev.kbdc) {
			case Kesc:
				done = 1;
				break;
			}
			break;
		case Emouse:
			dbutton_mouse_event(menu_button, ev);
			dbutton_mouse_event(close_button, ev);
			dentry_mouse_event(search_entry, ev);
			break;
		}
	}
	searchweb_window_destroy();
	searchweb_destroy();
	return selurl;
}
