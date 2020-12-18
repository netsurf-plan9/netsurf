#ifndef DRAWUI_SCROLLBAR_H
#define DRAWUI_SCROLLBAR_H

typedef struct dscrollbar dscrollbar;

struct dscrollbar
{
	dwidget;
	int scrollx;
	int scrolly;
	int viewx;
	int viewy;
	int extentx;
	int extenty;
	int buttons;
	void(*mouse_cb)(Mouse, void*);
	void *mouse_cb_data;
};

dscrollbar* dscrollbar_create(void);
void dscrollbar_set_view_size(dscrollbar *sb, int x, int y);
void dscrollbar_set_extents(dscrollbar *sb, int x, int y);
void dscrollbar_set_scroll(dscrollbar *sb, int x, int y);
int  dscrollbar_try_scroll(dscrollbar *sb, int sx, int sy);
void dscrollbar_set_rect(dscrollbar *sb, Rectangle r);
void dscrollbar_draw(dscrollbar *sb);
int dscrollbar_mouse_event(dscrollbar *sb, Event e);
void dscrollbar_keyboard_event(dscrollbar *sb, Event e);

#endif
