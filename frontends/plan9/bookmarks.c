#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "netsurf/browser_window.h"
#include "utils/nsurl.h"
#include "plan9/utils.h"
#include "plan9/bookmarks.h"

void bookmarks_init(void)
{
	char *path;
	FILE *fp;

	path = userdir_file("bookmarks.html");
	if (access(path, F_OK) < 0) {
		fp = fopen(path, "a");
		fprintf(fp, "<html><head>\n");
		fprintf(fp, "<link rel=\"stylesheet\" type=\"text/css\" href=\"resource:internal.css\">\n");
		fprintf(fp, "<title>Bookmarks</title></head>\n");
		fprintf(fp, "<body id=\"dirlist\" class=\"ns-even-bg ns-even-fg ns-border\">\n");
		fprintf(fp, "<h1 class=\"ns-border\">Bookmarks</h1>\n<br/>");
		fclose(fp);
	}
	free(path);
}

void bookmarks_add(const char *title, struct browser_window *bw)
{
	char *path;
	FILE *fp;

	path = userdir_file("bookmarks.html");
	fp = fopen(path, "a");
	fprintf(fp, "<p><a href='%s'>%s</a></p>\n", nsurl_access(browser_window_access_url(bw)), title);
	fclose(fp);
	free(path);
}

void bookmarks_show(struct browser_window *bw)
{
	nserror error;
	nsurl *url;
	char *path;

	path = userdir_file("bookmarks.html");
	error = netsurf_path_to_nsurl(path, &url);
	if (error == NSERROR_OK) {
		browser_window_navigate(bw, url, NULL, BW_NAVIGATE_NONE,
			NULL, NULL, NULL);
		nsurl_unref(url);
	} else {
		fprintf(stderr, "unable to create url: %s\n", messages_get_errorcode(error));
	}
	free(path);
}
