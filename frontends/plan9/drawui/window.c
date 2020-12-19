#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <draw.h>
#include <event.h>
#include "netsurf/browser_window.h"
#include "plan9/drawui/widget.h"
#include "plan9/drawui/window.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/entry.h"
#include "plan9/drawui/toolbar.h"
#include "plan9/drawui/scrollbar.h"
#include "plan9/drawui/data.h"

typedef struct dwindow dwindow;

struct dwindow
{
	Rectangle r;
	Rectangle iconr;
	Image *icon;
	Rectangle titler;
	char *title;
	Rectangle statusr;
	char *status;
	Rectangle viewr;
	void(*view_mouse_cb)(Mouse, void*);
	void *view_mouse_cb_data;
	void(*view_keyboard_cb)(int, void*);
	void *view_keyboard_cb_data;
	dtoolbar *toolbar;
	dscrollbar *scrollbar;
};

struct dwindow *dwindow_create(Rectangle initialr)
{
	struct dwindow *win;

	win = calloc(1, sizeof *win);
	if(win == NULL) {
		return NULL;
	}
	win->toolbar = dtoolbar_create();
	if(win->toolbar == NULL) {
		return NULL;
	}
	win->scrollbar = dscrollbar_create();
	if(win->scrollbar == NULL) {
		return NULL;
	}
	dwindow_resize(win, initialr);
	return win;
}

void dwindow_destroy(struct dwindow *window)
{
	/* XXX missing cleanup */
	free(window);
}

/* XXX should have a real layout manager...or not */
void dwindow_resize(struct dwindow *window, Rectangle newr)
{
	Rectangle r;
	int w, h, x, y, ix, iy;

	window->r = newr;
	w = Dx(window->r);
	h = Dy(window->r);

	x = window->r.min.x;
	y = window->r.min.y;

	h = PADDING+ICON_SIZE;
	ix = x + ICON_SIZE;
	r = Rect(x + PADDING, y + PADDING, ix + PADDING, y + PADDING + h);
	window->iconr = r;
	ix += PADDING;

	h = PADDING+font->height;
	r = Rect(ix, y, x + w, y + h);
	window->titler = r;
	y += h;

	h = PADDING+TOOLBAR_HEIGHT+PADDING+BORDER_WIDTH;
	r = Rect(x, y, x + w, y + h);
	dtoolbar_set_rect(window->toolbar, r);
	y += h;

	h = BORDER_WIDTH+PADDING+font->height+PADDING;
	r = Rect(x, window->r.max.y - h, x + w, window->r.max.y);
	window->statusr = r;

	r = Rect(x + SCROLL_WIDTH, window->toolbar->r.max.y + 1, x + w, window->statusr.min.y);
	window->viewr = r;

	r = Rect(x, window->toolbar->r.max.y + 1, x + SCROLL_WIDTH + 1, window->statusr.min.y);
	dscrollbar_set_rect(window->scrollbar, r);
	dscrollbar_set_view_size(window->scrollbar, Dx(window->viewr), Dy(window->viewr));
	dwindow_draw(window);		
}

static void draw_icon(dwindow *window)
{
	Image *i;

	i = bg_color;
	if (window->icon != NULL) {
		i = window->icon;
	}
	replclipr(screen, 0, window->iconr);
	draw(screen, window->iconr, bg_color, nil, ZP);
	draw(screen, window->iconr, i, nil, ZP);
}

static void draw_title(dwindow *window)
{
	Point p;
	int h;

	replclipr(screen, 0, window->titler);
	draw(screen, window->titler, bg_color, nil, ZP);
	p = Pt(window->titler.min.x + PADDING, window->titler.min.y + PADDING);
	string(screen, p, fg_color, ZP, font, window->title);
}

static void draw_status(dwindow *window)
{
	Point p0, p1;

	replclipr(screen, 0, window->statusr);
	draw(screen, window->statusr, bg_color, nil, ZP);
	p0 = Pt(window->statusr.min.x, window->statusr.min.y);
	p1 = Pt(window->statusr.max.x, window->statusr.min.y);
	line(screen, p0, p1, 0, 0, 0, fg_color, ZP);
	p0 = Pt(window->statusr.min.x + PADDING, window->statusr.min.y + 1 + PADDING);
	string(screen, p0, fg_color, ZP, font, window->status);
}

void dwindow_draw(struct dwindow *window)
{
	draw_title(window);
	draw_icon(window);
	dtoolbar_draw(window->toolbar);
	dscrollbar_draw(window->scrollbar);
	draw_status(window);
}

void dwindow_mouse_event(struct dwindow *window, Event e)
{
	if(dscrollbar_mouse_event(window->scrollbar, e) == 0 ||
	   dtoolbar_mouse_event(window->toolbar, e) == 0) {
		return;
	}
	if(ptinrect(e.mouse.xy, window->viewr) && window->view_mouse_cb != NULL) {
		window->view_mouse_cb(e.mouse, window->view_mouse_cb_data);
	}
}

void dwindow_keyboard_event(struct dwindow *window, Event e)
{
	dtoolbar_keyboard_event(window->toolbar, e);
	if(!(window->toolbar->url_entry->state & STATE_FOCUSED) && window->view_keyboard_cb != NULL) {
		window->view_keyboard_cb(e.kbdc, window->view_keyboard_cb_data);
	}
}

Rectangle dwindow_get_view_rect(struct dwindow *window)
{
	return window->viewr;
}

Rectangle dwindow_rect_in_view_rect(struct dwindow *window, Rectangle r)
{
	Rectangle vr;

	vr = dwindow_get_view_rect(window);	
	return rectaddpt(r, vr.min);
}

Point dwindow_point_in_view_rect(struct dwindow *window, Point p)
{
	Rectangle r;

	r = dwindow_get_view_rect(window);
	return addpt(r.min, p);
}

void dwindow_set_extents(struct dwindow *window, int x, int y)
{
	dscrollbar_set_extents(window->scrollbar, x, y);
}

int dwindow_get_extent_y(struct dwindow *window)
{
	return window->scrollbar->extenty;
}

int dwindowry_scroll(struct dwindow *window, int sx, int sy)
{
	return dscrollbar_try_scroll(window->scrollbar, sx, sy);
}

void dwindow_set_scroll(struct dwindow *window, int sx, int sy)
{
	dscrollbar_set_scroll(window->scrollbar, sx, sy);
}

int dwindow_get_scroll_x(struct dwindow *window)
{
	return window->scrollbar->scrollx;
}

int dwindow_get_scroll_y(struct dwindow *window)
{
	return window->scrollbar->scrolly;
}

int dwindow_try_scroll(struct dwindow *window, int sx, int sy)
{
	return dscrollbar_try_scroll(window->scrollbar, sx, sy);
}

void dwindow_set_icon(struct dwindow *window, Image *icon)
{
	window->icon = icon;
	draw_icon(window);
}

void dwindow_set_title(struct dwindow *window, const char *text)
{
	FILE *fp;
	char *buf;
	int len;

	if(window->title != NULL) {
		free(window->title);
	}
	len = strlen("NetSurf - ") + strlen(text) + 1;
	window->title = malloc(len * sizeof(char));
	snprintf(window->title, len, "NetSurf - %s", text);
	fp = fopen("/mnt/wsys/label", "w");
	if (fp != NULL) {
		fprintf(fp, window->title);
		fclose(fp);
	}
	draw_title(window);
}

void dwindow_set_url(struct dwindow *window, const char *text)
{
	dentry_set_text(window->toolbar->url_entry, text);
}

void dwindow_set_status(struct dwindow *window, const char *text)
{
	if(window->status != NULL) {
		free(window->status);
	}
	window->status = strdup(text);
	draw_status(window);
}

void dwindow_set_back_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	dbutton_set_mouse_callback(window->toolbar->back_button, cb, data);
}

void dwindow_set_forward_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	dbutton_set_mouse_callback(window->toolbar->fwd_button, cb, data);
}

void dwindow_set_stop_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	dbutton_set_mouse_callback(window->toolbar->stop_button, cb, data);
}

void dwindow_set_reload_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	dbutton_set_mouse_callback(window->toolbar->reload_button, cb, data);
}

void dwindow_set_entry_activated_callback(struct dwindow *window, void(*cb)(char*, void*), void *data)
{
	dentry_set_activated_callback(window->toolbar->url_entry, cb, data);
}

void dwindow_set_scrollbar_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	dscrollbar_set_mouse_callback(window->scrollbar, cb, data);
}

void dwindow_set_browser_mouse_callback(struct dwindow *window, mouse_callback cb, void *data)
{
	window->view_mouse_cb = cb;
	window->view_mouse_cb_data = data;
}

void dwindow_set_browser_keyboard_callback(struct dwindow *window, keyboard_callback cb, void *data)
{
	window->view_keyboard_cb = cb;
	window->view_keyboard_cb_data = data;
}

void dwindow_focus_url_bar(struct dwindow *window)
{
	dentry_set_focused(window->toolbar->url_entry);
}
