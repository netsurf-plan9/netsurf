#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>
#include "plan9/drawui/widget.h"
#include "plan9/drawui/entry.h"
#include "plan9/drawui/data.h"
#include "utils/utils.h"

static void text_insert(dentry *entry, char *s);
static void text_delete(dentry *entry, int bs);

static void roundedborder(Image *dst, Rectangle r, int thick, Image *src, Point sp);

static char *menu2str[] =
{
	"cut",
	"paste",
	"snarf",
	0 
};

enum
{
	Mcut,
	Mpaste,
	Msnarf,
};
static Menu menu2 = { menu2str };


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

int dentry_has_focus(dentry *entry) {
	return (entry->state & STATE_FOCUSED);
}

static void dentry_unfocus(dentry *entry) {
	if (!dentry_has_focus(entry))
		return;
	entry->state ^= STATE_FOCUSED; /* remove focus */
	entry->buttons = 0;
	entry->pos = entry->len;
	entry->pos2 = entry->len;
	dentry_draw(entry);
}

void dentry_set_focused(dentry *entry, bool select_all)
{
	if (dentry_has_focus(entry)) {
		return;
	}
	entry->state |= STATE_FOCUSED;
	if (select_all) {
		entry->pos = 0;
		entry->pos2 = entry->len;
	}
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
	entry->pos2 = entry->pos;
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
	int y, sels, sele;

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
	if (entry->pos != entry->pos2) {
		sels = min(entry->pos, entry->pos2);
		sele = max(entry->pos, entry->pos2);
		p.x += stringnwidth(font, entry->text, sels);
		stringnbg(screen, p, fg_color, ZP, font, entry->text+sels, sele-sels, sel_color, ZP);
	} else if (entry->state & STATE_FOCUSED) {
		entry->tick_x = stringnwidth(font, entry->text, entry->pos);
		p.x += entry->tick_x;
		r = Rect(p.x, p.y, p.x + Dx(tick->r), p.y + Dy(tick->r));
		draw(screen, r, tick, nil, ZP);
	}	
}

static int dentry_mouse_to_position(dentry *entry, Mouse m)
{
	int i, x, prev, cur;

	x = m.xy.x - entry->r.min.x - PADDING;
	prev = 0;
	for (i = 0; i < entry->len; i++) {
		cur = stringnwidth(font, entry->text, i);
		if ((prev+cur)/2 >= x) {
			i--;
			break;
		} else if (prev <= x && cur >= x) {
			break;
		}
		prev = cur;
	}

	return i;
}

static bool is_separator(char c)
{
	return c == 0 || c == '/' || (!isalnum(c) && c != '-');
}

static void entry_click_sel(dentry *entry)
{
	int s, e;

	if (entry->pos == 0)
		entry->pos2 = entry->len;
	else if (entry->pos == entry->len)
		entry->pos = 0;
	else {
		s = entry->pos;
		e = entry->pos;
		while ((s - 1) >= 0 && !is_separator(entry->text[s - 1]))
			--s;
		while (e < entry->len && !is_separator(entry->text[e]))
			++e;
		entry->pos = s;
		entry->pos2 = e;
	}
}

int dentry_mouse_event(dentry *entry, Event e)
{
	static int lastn = -1, lastms = -1;
	int in, n, sels, sele;
	char *s;
	size_t len;

	in = ptinrect(e.mouse.xy, entry->r);

	if (in && !entry->buttons && e.mouse.buttons) {
		entry->state |= STATE_FOCUSED;
	}
	if (entry->state & STATE_FOCUSED) {
		n = dentry_mouse_to_position(entry, e.mouse);
		if (!in && !entry->buttons && e.mouse.buttons) {
			dentry_unfocus(entry);
			return -1;
		}
		if (e.mouse.buttons & 1) { /* holding left button */
			sels = min(entry->pos, entry->pos2);
			sele = max(entry->pos, entry->pos2);
			if (e.mouse.buttons == (1|2) && entry->buttons == 1) {
				if (sels != sele) {
					plan9_snarf(entry->text+sels, sele-sels);
					text_delete(entry, 0);
				}
			} else if (e.mouse.buttons == (1|4) && entry->buttons == 1) {
				plan9_paste(&s, &len);
				if (len >= 0 && s != NULL)
					text_insert(entry, s);
				free(s);
			} else if (e.mouse.buttons == 1 && entry->buttons <= 1) {
				entry->pos = n;
				if (entry->buttons == 0) {
					entry->pos2 = n;
					if (n == lastn && lastms > 0 && (e.mouse.msec - lastms)<=250)
						entry_click_sel(entry);
				}
			}
			dentry_draw(entry);
			lastn = n;
			lastms = e.mouse.msec;
		} else if (e.mouse.buttons & 2) {
			sels = min(entry->pos, entry->pos2);
			sele = max(entry->pos, entry->pos2);
			n = emenuhit(2, &e.mouse, &menu2);
			switch(n) {
			case Mcut:
				if (sels != sele) {
					plan9_snarf(entry->text+sels, sele-sels);
					text_delete(entry, 0);
				}
				break;
			case Mpaste:
				plan9_paste(&s, &len);
				if (len >= 0 && s != NULL)
					text_insert(entry, s);
				free(s);
				break;
			case Msnarf:
				if (sels != sele) {
					plan9_snarf(entry->text+sels, sele-sels);
				}
				break;
			}
			dentry_draw(entry);
		}
		entry->buttons = e.mouse.buttons;
		return 0;
	}

	return -1;
}

void dentry_keyboard_event(dentry *entry, Event e)
{
	int sels, sele, n;
	char s[UTFmax+1];
	Rune k;

	if (!(entry->state & STATE_FOCUSED))
		return;

	sels = min(entry->pos, entry->pos2);
	sele = max(entry->pos, entry->pos2);
	k = e.kbdc;
	switch (k) {
	case Keof:
	case '\n':
		entry->pos = entry->pos2 = entry->len;
		if (entry->activated_cb != NULL) {
			entry->activated_cb(strdup(entry->text), entry->activated_cb_data);
			return;
		}
		break;
	case Knack:	/* ^U: delete selection, if any, and everything before that */
		memmove(entry->text, entry->text + sele, entry->len - sele);
		entry->len = entry->len - sele;
		entry->pos = 0;
		entry->text[entry->len] = 0;
		break;
	case Kleft:
		entry->pos = max(0, sels-1);
		break;
	case Kright:
		entry->pos = min(entry->len, sele+1);
		break;
	case Ksoh:	/* ^A: start of line */
	case Khome:
		entry->pos = 0;
		break;
	case Kenq:	/* ^E: end of line */
	case Kend:
		entry->pos = entry->len;
		break;
	case Kdel:
		text_delete(entry, 0);
		break;
	case Kbs:
		text_delete(entry, 1);
		break;
	case Ketb:
		while(sels > 0 && !isalnum(entry->text[sels-1]))
			sels--;
		while(sels > 0 && isalnum(entry->text[sels-1]))
			sels--;
		entry->pos = sels;
		entry->pos2 = sele;
		text_delete(entry, 0);
		break;
	case Kesc:
		if (sels == sele) {
			sels = entry->pos = 0;
			sele = entry->pos2 = entry->len;
		}
		plan9_snarf(entry->text+sels, sele-sels);
		text_delete(entry, 0);
		break;
	case 0x7: /* ^G: remove focus */
		dentry_unfocus(entry);
		return;
	default:
		if (k < 0x20 || (k & 0xFF00) == KF || (k & 0xFF00) == Spec || (n = runetochar(s, &k)) < 1) {
			return;
		}
		s[n] = 0;
		text_insert(entry, s);
	}
	entry->pos2 = entry->pos;
	dentry_draw(entry);
}

static void text_insert(dentry *entry, char *s)
{
	int sels, sele, n;
	char *p;

	n = strlen(s);
	if (entry->size <= entry->len + n) {
		entry->size = (entry->len + n)*2 + 1;
		if ((p = realloc(entry->text, entry->size)) == NULL) {
			return;
		}
		entry->text = p;
	}

	sels = min(entry->pos, entry->pos2);
	sele = max(entry->pos, entry->pos2);
	if (sels != sele) {
		memmove(entry->text + sels + n, entry->text + sele, entry->len - sele);
		entry->len -= sele - sels;
		entry->pos = sels;
	} else if (entry->pos != entry->len) {
		memmove(entry->text + entry->pos + n, entry->text + entry->pos, entry->len - entry->pos);
	}

	memmove(entry->text + sels, s, n);
	entry->len += n;
	entry->pos2 = sels;
	entry->pos = sels + n;
	entry->text[entry->len] = 0;		
}

static void text_delete(dentry *entry, int bs)
{
	int sels, sele;

	sels = min(entry->pos, entry->pos2);
	sele = max(entry->pos, entry->pos2);
	if(sels == sele && sels == 0)
		return;
	memmove(entry->text + sels - bs, entry->text + sele, entry->len - sele);
	entry->pos = sels - bs;
	entry->len -= sele - sels + bs;
	entry->pos2 = entry->pos;
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
