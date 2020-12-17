#ifndef DRAWUI_TOOLBAR_H
#define DRAWUI_TOOLBAR_H

typedef struct dtoolbar dtoolbar;

struct dtoolbar
{
	dwidget;
	dbutton *back_button;
	dbutton *fwd_button;
	dbutton *stop_button;
	dbutton *reload_button;
	dentry  *url_entry;
};

dtoolbar *dtoolbar_create(void);
void dtoolbar_set_rect(dtoolbar *toolbar, Rectangle r);
void dtoolbar_draw(dtoolbar *toolbar);
int dtoolbar_mouse_event(dtoolbar *toolbar, Event e);
void dtoolbar_keyboard_event(dtoolbar *toolbar, Event e);

#endif
