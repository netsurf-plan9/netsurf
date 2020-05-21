#ifndef DRAWUI_BUTTON_H
#define DRAWUI_BUTTON_H

typedef struct dbutton dbutton;

struct dbutton
{
	dwidget;
	Image *i;
	Image *focus;
	void(*mouse_cb)(Mouse, void*);
	void *mouse_cb_data;
};

dbutton* dbutton_create(Image *i, Image *f);
void dbutton_set_mouse_callback(dbutton *self, void(*mouse_cb)(Mouse, void*), void *data);

void dbutton_set_rect(dbutton *button, Rectangle r);
void dbutton_draw(dbutton *button);
void dbutton_mouse_event(dbutton *button, Event e);
void dbutton_keyboard_event(dbutton *button, Event e);

#endif
