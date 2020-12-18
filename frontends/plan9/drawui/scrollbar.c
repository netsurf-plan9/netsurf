#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <draw.h>
#include <event.h>
#include "plan9/drawui/widget.h"
#include "plan9/drawui/scrollbar.h"
#include "plan9/drawui/data.h"

dscrollbar* dscrollbar_create(void)
{
	dscrollbar *sb;

	sb = calloc(1, sizeof *sb);
	if(sb == NULL) {
		return NULL;
	}
	sb->extentx = 0;
	sb->extenty = 0;
	sb->viewx = 0;
	sb->viewy = 0;
	sb->scrollx = 0;
	sb->scrolly = 0;
	return sb;
}

void dscrollbar_set_view_size(dscrollbar *sb, int x, int y)
{
	sb->viewx = x;
	sb->viewy = y;
}

void dscrollbar_set_extents(dscrollbar *sb, int x, int y)
{
	sb->extentx = x;
	sb->extenty = y;
	dscrollbar_draw(sb);
}

void dscrollbar_set_scroll(dscrollbar *sb, int x, int y)
{
	sb->scrollx = x;
	sb->scrolly = y;
	dscrollbar_draw(sb);
}

int dscrollbar_try_scroll(dscrollbar *sb, int sx, int sy)
{
	int ex, ey, ox, oy;

	if(sb->viewy == sb->extenty)
		return 0;

	ox = sb->scrollx + sx;
	oy = sb->scrolly + sy;
	ex = sb->extentx;
	ey = sb->extenty;
	if (oy < 0) { 
		if(sb->scrolly == 0)
			return 0;
		oy = 0;
	} else {
		if(sb->viewy + oy == ey)
			return 0;
		if(oy > (sb->extenty - sb->viewy))
			oy = sb->extenty - sb->viewy;
	}
	sb->scrollx = ox;
	sb->scrolly = oy;
	dscrollbar_draw(sb);
	return 1;
}

void dscrollbar_set_mouse_callback(dscrollbar *sb, void(*mouse_cb)(Mouse, void*), void *data)
{
	sb->mouse_cb = mouse_cb;
	sb->mouse_cb_data = data;
}

void dscrollbar_set_rect(dscrollbar *sb, Rectangle r)
{
	sb->r = r;
}

void dscrollbar_draw(dscrollbar *sb)
{
	Rectangle r, cr;
	int y, h;

	replclipr(screen, 0, sb->r);
	r = insetrect(sb->r, 1);
	draw(screen, r, scroll_bg_color, nil, ZP);
	cr = sb->r;
	cr.max.x -= 2;
	if(sb->extenty > 0) {
		h = ((double)sb->viewy / sb->extenty) * Dy(sb->r);
		y = ((double)sb->scrolly / sb->extenty) * Dy(sb->r);
		cr.min.y += y;
		cr.max.y = cr.min.y + h;
	}
	draw(screen, cr, bg_color, nil, ZP);		
}

int dscrollbar_mouse_event(dscrollbar *sb, Event e)
{
	if(ptinrect(e.mouse.xy, sb->r) || sb->buttons) {
		if(sb->mouse_cb != NULL)
			sb->mouse_cb(e.mouse, sb->mouse_cb_data);
		sb->buttons = e.mouse.buttons;
		return 0;
	}
	sb->buttons = 0;
	return -1;
}

void dscrollbar_keyboard_event(dscrollbar *sb, Event e)
{

}
