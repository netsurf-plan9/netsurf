#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <draw.h>
#include <event.h>
#include <lib9.h>
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/nsoption.h"
#include "netsurf/keypress.h"
#include "netsurf/browser_window.h"
#include "netsurf/content.h"
#include "desktop/search.h"
#include "plan9/bookmarks.h"
#include "plan9/search.h"
#include "plan9/searchweb.h"
#include "plan9/history.h"
#include "plan9/utils.h"
#include "plan9/window.h"
#include "plan9/menus.h"
#include "plan9/utils.h"

extern int esepmenuhit(int but, Mouse *m, Menu *menu);
extern void drawui_exit(int);
static char *menu2gen(int);
static char* menu3gen(int); 

char *menu2str[] =
{
/*
	"debug render",
	"debug dom",
*/
	"orig size",
	"zoom in",
	"zoom out",
	"-",
	"source",
	"export image",
	"export text",
	"-",
	"cut",
	"paste",
	"snarf",
	"plumb",
	"look",
	"/",
	0 
};

enum
{
/*
	Mdebugrender,
	Mdebugdom,
*/
	Morigsize,
	Mzoomin,
	Mzoomout,
	Msep1,
	Msource,
	Mexportimage,
	Mexporttext,
	Msep,
	Mcut,
	Mpaste,
	Msnarf,
	Mplumb,
	Msearch,
	Msearchnext,
};

Menu menu2 = { 0, menu2gen };

char *menu2altstr[] =
{
	"new window",
	"snarf url",
	"plumb url",
	"-",
	"open in page",
	"snarf image url",
	"plumb image url",
	0
};

enum
{
	Mnewwin,
	Msnarfurl,
	Mplumburl,
	Msepm2,
	Mopeninpage,
	Msnarfimageurl,
	Mplumbimageurl,
	Mlast,
	Mend,
};

static char *menu3str[] =
{
	"back",
	"forward",
	"stop",
	"reload",
	"search",
	"history",
	"bookmark",
	"bookmarks",
	"js",
	"exit",
	0
};

enum
{
	Mback,
	Mforward,
	Mstop,
	Mreload,
	Msearchweb,
	Mhistory,
	Maddbookmark,
	Mbookmarks,
	Mjavascript,
	Mexit,
};

static Menu menu3 = { 0, menu3gen };

static void viewsource(struct browser_window *bw)
{
	struct hlcache_handle *hlcontent;
	const uint8_t *source_data;
	size_t source_size;
	char *filename;
	nserror err;
 
	hlcontent = browser_window_get_content(bw);
	if (hlcontent == NULL)
		return;	
	if (content_get_type(hlcontent) != CONTENT_HTML)
		return;
	source_data = content_get_source_data(hlcontent, &source_size);
	err = nsurl_nice(browser_window_access_url(bw), &filename, false);
	if (err != NSERROR_OK)
		filename = "netsurf:view-source";
	send_data_to_plumber("edit", filename, (char*)source_data, source_size);
}

static void snarf_url(struct nsurl *url)
{
	char *s;

	s = nsurl_access(url);
	if (s != NULL) {
		plan9_snarf(s, strlen(s));
	}
}

static void plumb_url(struct nsurl *url)
{
	char *s;

	s = nsurl_access(url);	
	if (s != NULL) {
		send_to_plumber(s);
	}
}


static char* menu2gen(int index)
{
	char buf[1025] = {0};

	if (index == Msearchnext) {
		if (search_has_next() == false && search_should_wrap() == false)
			return NULL;
		snprintf(buf, sizeof buf, "/%s", search_text());
		return buf;
	}
	return menu2str[index];
}

static void menu2hitstd(struct gui_window *gw, Mouse *m)
{
	char buf[1024] = {0};
	char *s, *e;
	size_t len;
	int n, flags, fd;
	browser_editor_flags eflags;

	n = esepmenuhit(2, m, &menu2);
	switch (n) {
/*
	case Mdebugrender:
		browser_window_debug_dump(gw->bw, stderr, CONTENT_DEBUG_RENDER);
		break;
	case Mdebugdom:
		browser_window_debug_dump(gw->bw, stderr, CONTENT_DEBUG_DOM);
		break;
*/
	case Morigsize:
		browser_window_set_scale(gw->bw, 1.0, true);
		break;
	case Mzoomin:
		browser_window_set_scale(gw->bw, 0.1, false);
		break;
	case Mzoomout:
		browser_window_set_scale(gw->bw, -0.1, false);
		break;
	case Msource:
		viewsource(gw->bw);
		break;
	case Mexportimage:
		if(eenter("Save as", buf, sizeof buf, m) > 0) {
			fd = open(buf, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (fd > 0) {
				writeimage(fd, gw->b, 0);
				close(fd);
			}
		}
		break;
	case Mexporttext:
		if(eenter("Save as", buf, sizeof buf, m) > 0) {
			save_as_text(browser_window_get_content(gw->bw), buf);
		}
		break;
	case Msearch:
		strcpy(buf, search_text());
		if(eenter("Search for", buf, sizeof buf, m) > 0) {
			search(gw, buf);
		} else {
			search_reset(gw);
		}
		break;
	case Msearchnext:
		if (search_should_wrap() == true) {
			strcpy(buf, search_text());
			search_reset(gw);
			search(gw, buf);
		} else {
			search_next(gw);
		}
		break;
	case Mcut:
		eflags = browser_window_get_editor_flags(gw->bw);
		if (eflags & BW_EDITOR_CAN_CUT)
			browser_window_key_press(gw->bw, NS_KEY_CUT_SELECTION);
		else
			browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
		break;
	case Mpaste:
		browser_window_key_press(gw->bw, NS_KEY_PASTE);
		break;
	case Msnarf:
		browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
		break;
	case Mplumb:
		browser_window_key_press(gw->bw, NS_KEY_COPY_SELECTION);
		plan9_paste(&s, &len);
		if (s != NULL) {
			e = strchr(s, ' ');
			if (e)
				*e = 0;
			send_to_plumber(s);
		}
		break;
	}
}
static void menu2hitalt(struct gui_window *gw, Mouse *m, struct nsurl *url, struct hlcache_handle *h)
{
#define ADDITEM(ITEM) do { items[i] = menu2altstr[ITEM]; actions[i] = ITEM; i++; } while(0);

	Menu menu;
	char *items[Mend];
	int actions[Mend], i, n;

	i = 0;
	if(url != NULL) {
		ADDITEM(Mnewwin);
		ADDITEM(Msnarfurl);
		ADDITEM(Mplumburl);
	}
	if (h != NULL) {
		if (i > 0)
			ADDITEM(Msepm2);
		ADDITEM(Mopeninpage);
		ADDITEM(Msnarfimageurl);
		ADDITEM(Mplumbimageurl);
	}
	items[i] = 0;
	menu.item = items;
	n = esepmenuhit(2, m, &menu);
	if (n < 0)
		return;
	switch (actions[n]) {
	case Mnewwin:
		exec_netsurf(nsurl_access(url));
		break;
	case Msnarfurl:
		snarf_url(url);
		break;
	case Mplumburl:
		plumb_url(url);
		break;
	case Mopeninpage:
		browser_window_navigate(gw->bw, hlcache_handle_get_url(h), NULL, BW_NAVIGATE_DOWNLOAD,
			NULL, NULL, NULL);
		break;
	case Msnarfimageurl:
		snarf_url(hlcache_handle_get_url(h));
		break;
	case Mplumbimageurl:
		plumb_url(hlcache_handle_get_url(h));
		break;
	}
}

void menu2hit(struct gui_window *gw, Mouse *m, struct browser_window_features *features)
{
	bool islink, isimage;

	islink = features->link != NULL;
	isimage = features->object != NULL && content_get_type(features->object) == CONTENT_IMAGE;
	if (islink == false && isimage == false) {
		menu2hitstd(gw, m);
	} else {
		menu2hitalt(gw, m, features->link, features->object);
	}
}

static char* menu3gen(int index)
{
	if (index == Mjavascript) {
		if (nsoption_bool(enable_javascript) == true)
			return "nojs";
		else
			return "js";
	}
	return menu3str[index];
}

void menu3hit(struct gui_window *gw, Mouse *m)
{
	char buf[255] = {0};
	int n;
	struct nsurl *url;

	n = emenuhit(3, m, &menu3);
	switch (n) {
	case Mback:
		if (browser_window_back_available(gw->bw)) {
			browser_window_history_back(gw->bw);
		}
		break;
	case Mforward:
		if (browser_window_forward_available(gw->bw)) {
			browser_window_history_forward(gw->bw);
		}
		break;
	case Mstop:
		browser_window_stop(gw->bw);
		break;
	case Mreload:
		browser_window_reload(gw->bw, true);
		break;
	case Msearchweb:
		url = esearchweb(gw->bw);
		if (url != NULL) {
			browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_HISTORY,
				NULL, NULL, NULL);
		}
		break;		
	case Mhistory:
		history_show(gw);
		break;
	case Maddbookmark:
		if (eenter("Add bookmark: ", buf, sizeof buf, m) > 0) {
			bookmarks_add(buf, gw->bw);
		}
		break;
	case Mbookmarks:
		bookmarks_show(gw);
		break;
	case Mjavascript:
		if (nsoption_bool(enable_javascript) == true) {
			nsoption_set_bool(enable_javascript, false);
		} else {
			nsoption_set_bool(enable_javascript, true);
		}
		break;
	case Mexit:
		drawui_exit(0);
	}
}
