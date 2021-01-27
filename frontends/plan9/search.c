#include <stdbool.h>
#include <string.h>
#include <draw.h>
#include <event.h>
#include "desktop/search.h"
#include "netsurf/search.h"
#include "netsurf/browser_window.h"
#include "netsurf/content.h"
#include "plan9/search.h"
#include "plan9/window.h"

static char search_buf[1024] = {0};
static bool found = false;
static bool has_next = false;

void search(struct gui_window *gw, const char *text)
{
	browser_window_search(gw->bw, gw, SEARCH_FLAG_FORWARDS, text);
	strcpy(search_buf, text);
}

void search_reset(struct gui_window *gw)
{
	browser_window_search_clear(gw->bw);
	search_buf[0] = 0;
	found = false;
	has_next = false;
}

bool search_has_next(void)
{
	return found && has_next;
}

bool search_should_wrap(void)
{
	return found && !has_next;
}

void search_next(struct gui_window *gw)
{
	browser_window_search(gw->bw, gw, SEARCH_FLAG_FORWARDS, search_buf);
}

char* search_text(void)
{
	return search_buf;
}

static void search_status(bool f, void*)
{
	found = f;
}

static void search_forward_state(bool active, void*)
{
	has_next = active;
}

struct gui_search_table search_table = {
	.status = search_status,
	.forward_state = search_forward_state,
};

struct gui_search_table *plan9_search_table = &search_table;
