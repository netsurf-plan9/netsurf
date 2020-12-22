#ifndef NETSURF_PLAN9_BITMAP_H
#define NETSURF_PLAN9_BITMAP_H

enum { BITMAP_BPP = 4 };

struct bitmap
{
	Image *i;
	unsigned char *data;
	int width;
	int height;
	int opaque;
	int modified;
};

void bitmap_alpha_blend(struct bitmap *bitmap, colour bg);

extern struct gui_bitmap_table *plan9_bitmap_table;

#endif
