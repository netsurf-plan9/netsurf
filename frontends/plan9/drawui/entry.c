#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "plan9/drawui/widget.h"
#include "plan9/drawui/entry.h"
#include "plan9/drawui/data.h"

static void text_insert(dentry *entry, int c);
static void text_delete(dentry *entry);

static void roundedborder(Image *dst, Rectangle r, int thick, Image *src, Point sp);

dentry *dentry_create(void)
{
	dentry *e;

	e = calloc(1, sizeof *e);
	if(e == NULL) {
		return NULL;
	}
	e->state = 0;
	e->size = 255;
	e->text = calloc(e->size, sizeof(char));
	e->text[0] = 0;
	e->pos = 0;
	e->len = 0;
	return e;
}

void dentry_set_focused(dentry *entry)
{
	if (entry->state & STATE_FOCUSED) {
		return;
	}
	entry->state |= STATE_FOCUSED;
	dentry_draw(entry);
}

void dentry_set_text(dentry *entry, const char *text)
{
	int l;
	
	l = strlen(text);
	if(l >= entry->size) {
		entry->size = l;
		entry->text = realloc(entry->text, entry->size * sizeof(char));
	}
	strncpy(entry->text, text, l);
	entry->text[l] = 0;
	entry->len = l;
	entry->pos = entry->len;
	entry->tick_x = stringnwidth(font, entry->text, entry->len);
	dentry_draw(entry);
}

void dentry_set_activated_callback(dentry *entry, void(*activated_cb)(char*, void*), void *data)
{
	entry->activated_cb = activated_cb;
	entry->activated_cb_data = data;
}

void dentry_set_rect(dentry *entry, Rectangle r)
{
	entry->r = r;
}

void dentry_draw(dentry *entry)
{
	Rectangle r;
	Point p;
	int y;

	replclipr(screen, 0, entry->r);
	draw(screen, entry->r, bg_color, nil, ZP);
	if (entry->state & STATE_FOCUSED) {
		roundedborder(screen, entry->r, 1, focus_color, ZP);
	} else {
		roundedborder(screen, entry->r, 0, fg_color, ZP);
	}
	y = (Dy(entry->r) - font->height) / 2;
	p = Pt(entry->r.min.x + PADDING, entry->r.min.y + y);
	stringn(screen, p, fg_color, ZP, font, entry->text, entry->len);
	if (entry->state & STATE_FOCUSED) {
		entry->tick_x = stringnwidth(font, entry->text, entry->pos);
		p.x += entry->tick_x;
		r = Rect(p.x, p.y, p.x + Dx(tick->r), p.y + Dy(tick->r));
		draw(screen, r, tick, nil, ZP);
	}	
}

void dentry_mouse_event(dentry *entry, Event e)
{
	int in;

	in = ptinrect(e.mouse.xy, entry->r);
	if(in && e.mouse.buttons&1 && !(entry->state & STATE_FOCUSED)) {
		entry->state |= STATE_FOCUSED;
		dentry_draw(entry);
	} else if(!in && e.mouse.buttons && (entry->state & STATE_FOCUSED)) {
		entry->state -= STATE_FOCUSED;
		dentry_draw(entry);
	}
}

void dentry_keyboard_event(dentry *entry, Event e)
{
	int k;

	if (!(entry->state & STATE_FOCUSED))
		return;

	k = e.kbdc;
	switch (k) {
	case Keof:
	case '\n':
		if (entry->activated_cb != NULL) {
			entry->activated_cb(strdup(entry->text), entry->activated_cb_data);
			return;
		}
		break;
	case Kesc:
		entry->len = 0;
		entry->pos = 0;
		entry->text[entry->len] = 0;
		break;
	case Kleft:
		if (entry->pos == 0) {
			return;
		}
		entry->pos--;
		break;
	case Kright:
		if (entry->pos == entry->len) {
			return;
		}
		entry->pos++;
		break;
	case Khome:
		if (entry->pos == 0) {
			return;
		}
		entry->pos = 0;
		break;
	case Kend:
		if (entry->pos == entry->len) {
			return;
		}
		entry->pos = entry->len;
		break;
	case Kbs:
		if (entry->pos == 0) {
			return;
		}
		text_delete(entry);
		break;
	default:
		if (k < 0x20 || k == Kdel || (k & 0xFF00) == KF || (k & 0xFF00) == Spec) {
			return;
		}
		text_insert(entry, k);
	}
	dentry_draw(entry);
}

static void text_insert(dentry *entry, int c)
{
	if (entry->len + 1 >= entry->size) {
		entry->size *= 1.5;
		entry->text = realloc(entry->text, entry->size * sizeof(char));
	}
	if (entry->pos != entry->len) {
		memmove(entry->text + entry->pos + 1, entry->text + entry->pos, entry->len - entry->pos);
	}
	entry->text[entry->pos] = c;
	entry->pos++;
	entry->len++;
	entry->text[entry->len] = 0;		
}

static void text_delete(dentry *entry)
{
	memmove(entry->text + entry->pos - 1, entry->text + entry->pos, entry->len - entry->pos);
	entry->pos--;
	entry->len--;
	entry->text[entry->len] = 0;
}


static void roundedborder(Image *dst, Rectangle r, int thick, Image *src, Point sp)
{
	int x0, x1, y0, y1, radius;
	Point tl, bl, tr, br;

	radius = 3;
	x0 = r.min.x;
	x1 = r.max.x-1;
	y0 = r.min.y;
	y1 = r.max.y-1;
	tl = Pt(x0+radius, y0+radius);
	bl = Pt(x0+radius, y1-radius);
	br = Pt(x1-radius, y1-radius);
	tr = Pt(x1-radius, y0+radius);
	arc(dst, tl, radius, radius, thick, src, sp, 90, 90);
	arc(dst, bl, radius, radius, thick, src, sp, 180, 90);
	arc(dst, br, radius, radius, thick, src, sp, 270, 90);
	arc(dst, tr, radius, radius, thick, src, sp, 0, 90);
	line(dst, Pt(x0, y0+radius), Pt(x0, y1-radius), 0, 0, thick, src, sp);
	line(dst, Pt(x0+radius, y1), Pt(x1-radius, y1), 0, 0, thick, src, sp);
	line(dst, Pt(x1, y1-radius), Pt(x1, y0+radius), 0, 0, thick, src, sp);
	line(dst, Pt(x1-radius, y0), Pt(x0+radius, y0), 0, 0, thick, src, sp);
}
