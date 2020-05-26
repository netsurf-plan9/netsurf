#ifndef NETSURF_PLAN9_BITMAP_H
#define NETSURF_PLAN9_BITMAP_H

enum { BITMAP_BPP = 4 };

struct bitmap
{
	unsigned char *data;
	int width;
	int height;
	int opaque;
	int modified;
};

extern struct gui_bitmap_table *plan9_bitmap_table;

#endif
