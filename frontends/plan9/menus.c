#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <draw.h>
#include <event.h>
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
static char* menu3gen(int); 

char *menu2str[] =
{
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

static char *menu2gen(int);

Menu menu2 = { 0, menu2gen };

char *menu2lstr[] =
{
//	"open in new window",
	"snarf url",
	"plumb url",
	0
};

enum
{
//	Mopeninwin,
	Msnarfurl,
	Mplumburl,
};
Menu menu2l = { menu2lstr };


char *menu2istr[] =
{
	"open in page",
	"snarf image url",
	"plumb image url",
	0,
};

enum
{
	Mopeninpage,
	Msnarfimageurl,
	Mplumbimageurl,
};
Menu menu2i = { menu2istr };

char *menu2ilstr[] =
{
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
	Milsnarflinkurl,
	Milplumblinkurl,
	Milsep,
	Milopeninpage,
	Milsnarfimageurl,
	Milplumbimageurl,
};
Menu menu2il = { menu2ilstr };

static char *menu3str[] =
{
	"back",
	"forward",
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
	Mreload,
	Msearchweb,
	Mhistory,
	Maddbookmark,
	Mbookmarks,
	Mjavascript,
	Mexit,
};

static Menu menu3 = { 0, menu3gen };


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

static void menu2hitlink(struct gui_window *gw, Mouse *m, struct nsurl *url)
{
	int n;

	n = esepmenuhit(2, m, &menu2l);
	switch (n) {
/*
	case Mopeninwin:
		s = nsurl_access(url);
		if (s != NULL) {
			exec_netsurf(argv0, s);
		}
		break;
*/
	case Msnarfurl:
		snarf_url(url);
		break;
	case Mplumburl:
		plumb_url(url);
		break;
	}
}

static void menu2hitimage(struct gui_window *gw, Mouse *m, struct hlcache_handle *h)
{
	int n;
	nsurl *url;

	url = hlcache_handle_get_url(h);
	n = esepmenuhit(2, m, &menu2i);
	switch (n) {
	case Mopeninpage:
		browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_DOWNLOAD,
			NULL, NULL, NULL);
		break;
	case Msnarfimageurl:
		snarf_url(url);
		break;
	case Mplumbimageurl:
		plumb_url(url);
		break;
	}
}

static void menu2hitimagelink(struct gui_window *gw, Mouse *m, struct nsurl *url, struct hlcache_handle *h)
{
	int n;
	nsurl *urli;

	urli = hlcache_handle_get_url(h);
	n = esepmenuhit(2, m, &menu2il);
	switch (n) {
	case Milopeninpage:
		browser_window_navigate(gw->bw, urli, NULL, BW_NAVIGATE_DOWNLOAD,
			NULL, NULL, NULL);
		break;
	case Milsnarflinkurl:
		snarf_url(url);
		break;
	case Milsnarfimageurl:
		snarf_url(urli);
		break;
	case Milplumblinkurl:
		plumb_url(url);
		break;
	case Milplumbimageurl:
		plumb_url(urli);
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
	} else if (islink == true && isimage == true) {
		menu2hitimagelink(gw, m, features->link, features->object);
	} else if (islink == true) {
		menu2hitlink(gw, m, features->link);
	} else if (isimage == true) {
		menu2hitimage(gw, m, features->object);
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
		url = ehistory(gw->bw);
		if (url != NULL) {
			browser_window_navigate(gw->bw, url, NULL, BW_NAVIGATE_HISTORY,
				NULL, NULL, NULL);
		}
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
