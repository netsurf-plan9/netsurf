#include <stdlib.h>
#include <draw.h>
#include <event.h>
#include "plan9/drawui/widget.h"
#include "plan9/drawui/button.h"
#include "plan9/drawui/data.h"

dbutton* dbutton_create(Image *i, Image *f)
{
	dbutton *button;

	button = calloc(1, sizeof *button);
	if(button == NULL) {
		return NULL;
	}
	button->state = 0;
	button->i = i;
	button->focus = f;
	return button;
}

void dbutton_set_mouse_callback(dbutton *button, void(*mouse_cb)(Mouse, void*), void *data)
{
	button->mouse_cb = mouse_cb;
	button->mouse_cb_data = data;
}

void dbutton_set_rect(dbutton *button, Rectangle r)
{
	button->r = r;
}

void dbutton_draw(dbutton *button)
{
	replclipr(screen, 0, button->r);
	if (button->state & STATE_FOCUSED) {
		draw(screen, button->r, button->focus, button->i, ZP);
	} else {
		draw(screen, button->r, fg_color, button->i, ZP);
	}
}

void dbutton_mouse_event(dbutton *button, Event e)
{
	int in;

	in = ptinrect(e.mouse.xy, button->r);
	if(in && !(button->state & STATE_FOCUSED)) {
		button->state |= STATE_FOCUSED;
		dbutton_draw(button);
	} else if (!in && (button->state & STATE_FOCUSED)) {
		button->state -= STATE_FOCUSED;
		dbutton_draw(button);
	}
	if (in && e.mouse.buttons != 0 && button->mouse_cb != NULL) {
		button->mouse_cb(e.mouse, button->mouse_cb_data);
	}
}

void dbutton_keyboard_event(dbutton *button, Event e)
{
}
