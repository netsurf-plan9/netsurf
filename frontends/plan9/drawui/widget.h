#ifndef DRAWUI_WIDGET_H
#define DRAWUI_WIDGET_H

typedef struct dwidget dwidget;

struct dwidget
{
	Rectangle r;
	int state;
};

enum
{
	STATE_ENABLED = 1 << 0,
	STATE_FOCUSED = 1 << 1,
};

enum
{
	PADDING = 4,
	BORDER_WIDTH = 1,
	ICON_SIZE = 16,
	TOOLBAR_HEIGHT = 24,
	SCROLL_WIDTH = 14,
};

#endif
