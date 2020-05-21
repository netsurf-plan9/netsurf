#include <stdlib.h>
#include <draw.h>
#include <event.h>
#include "plan9/drawui/widget.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/entry.h"
#include "plan9/drawui/toolbar.h"
#include "plan9/drawui/data.h"

dtoolbar *dtoolbar_create(void)
{
	dtoolbar *toolbar;
	Rectangle r;
	Image *i;

	toolbar = calloc(1, sizeof *toolbar);
	if(toolbar == NULL) {
		return NULL;
	}
	toolbar->back_button = dbutton_create(back_button_icon, back_button_focus_color);
	toolbar->fwd_button = dbutton_create(forward_button_icon, forward_button_focus_color);
	toolbar->stop_button = dbutton_create(stop_button_icon, stop_button_focus_color);
	toolbar->reload_button = dbutton_create(reload_button_icon, reload_button_focus_color);
	toolbar->url_entry = dentry_create();
	return toolbar;
}

void dtoolbar_set_rect(dtoolbar *toolbar, Rectangle r)
{
	Rectangle cr;
	int x0, y0, x1, y1;

	toolbar->r = r;
	x0 = r.min.x + PADDING;
	y0 = r.min.y + PADDING + PADDING;
	x1 = x0 + ICON_SIZE;
	y1 = y0 + ICON_SIZE;
	dbutton_set_rect(toolbar->back_button, Rect(x0, y0, x1, y1));
	x0 = x1 + PADDING;
	x1 = x0 + ICON_SIZE;
	dbutton_set_rect(toolbar->fwd_button, Rect(x0, y0, x1, y1));
	x0 = x1 + PADDING;
	x1 = x0 + ICON_SIZE;
	dbutton_set_rect(toolbar->stop_button, Rect(x0, y0, x1, y1));
	x0 = x1 + PADDING;
	x1 = x0 + ICON_SIZE;
	dbutton_set_rect(toolbar->reload_button, Rect(x0, y0, x1, y1));
	x0 = x1 + PADDING;
	y0 -= PADDING;
	x1 = r.max.x - PADDING;
	y1 += PADDING;
	dentry_set_rect(toolbar->url_entry, Rect(x0, y0, x1, y1));
}

void dtoolbar_draw(dtoolbar *toolbar)
{
	Point p0, p1;

	replclipr(screen, 0, toolbar->r);
	draw(screen, toolbar->r, display->white, nil, ZP);
	dbutton_draw(toolbar->back_button);
	dbutton_draw(toolbar->fwd_button);
	dbutton_draw(toolbar->stop_button);
	dbutton_draw(toolbar->reload_button);
	dentry_draw(toolbar->url_entry);
	p0 = Pt(toolbar->r.min.x, toolbar->r.max.y);
	p1 = Pt(toolbar->r.max.x, toolbar->r.max.y);
	replclipr(screen, 0, insetrect(toolbar->r, -1));
	line(screen, p0, p1, 0, 0, 0, display->black, ZP);
}

void dtoolbar_mouse_event(dtoolbar *toolbar, Event e)
{
	dbutton_mouse_event(toolbar->back_button, e);
	dbutton_mouse_event(toolbar->fwd_button, e);
	dbutton_mouse_event(toolbar->stop_button, e);
	dbutton_mouse_event(toolbar->reload_button, e);
	dentry_mouse_event(toolbar->url_entry, e);
}

void dtoolbar_keyboard_event(dtoolbar *toolbar, Event e)
{
	dbutton_keyboard_event(toolbar->back_button, e);
	dbutton_keyboard_event(toolbar->fwd_button, e);
	dbutton_keyboard_event(toolbar->stop_button, e);
	dbutton_keyboard_event(toolbar->reload_button, e);
	dentry_keyboard_event(toolbar->url_entry, e);
}
