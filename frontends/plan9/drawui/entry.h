#ifndef DRAWUI_ENTRY_H
#define DRAWUI_ENTRY_H

typedef struct dentry dentry;

struct dentry
{
	dwidget;
	int tick_x;
	int pos;
	int len;
	int size;
	char* text;
	void(*activated_cb)(char*, void*);
	void *activated_cb_data;
};

dentry *dentry_create(void);
void dentry_set_text(dentry *entry, const char *text);
void dentry_set_activated_callback(dentry *entry, void(*cb)(char*, void*), void*);

void dentry_set_rect(dentry *entry, Rectangle r);
void dentry_draw(dentry *entry);
void dentry_mouse_event(dentry *entry, Event e);
void dentry_keyboard_event(dentry *entry, Event e);
#endif
