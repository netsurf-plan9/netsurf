#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <lib9.h>
#include "utils/log.h"
#include "utils/utils.h"
#include "utils/nsurl.h"
#include "utils/messages.h"
#include "utils/nsoption.h"
#include "utils/string.h"
#include "desktop/download.h"
#include "netsurf/download.h"
#include "plan9/download.h"
#include "plan9/utils.h"


struct gui_download_window
{
	int fd;
};

static bool can_page(struct download_context *ctx)
{
	char *mime;

	mime = download_context_get_mime_type(ctx);
	if(strncmp(mime, "application/pdf", 15) == 0
	|| strncmp(mime, "application/postscript", 22) == 0
	|| strncmp(mime, "image/", 6) == 0) {
		return true;
	}
	return false;
}

struct gui_download_window *download_create(struct download_context *ctx, struct gui_window *parent)
{
	struct gui_download_window *w;
	int p[2], fd;

	if (can_page(ctx) == false)
		return NULL;
	if (pipe(p) < 0) {
		fprintf(stderr, "cannot create pipe\n");
		return NULL;
	}
	switch (rfork(RFPROC|RFFDG)) {
	case -1:
		fprintf(stderr, "rfork failed\n");
		return NULL;
	case 0:
		close(p[1]);
		dup2(p[0], 0);
		close(p[0]);
		execl("/bin/page", "page", "-w", 0);
		fprintf(stderr, "exec failed\n");
		exit(1);
	default:
		w = calloc(1, sizeof *w);
		w->fd = p[1];
		close(p[0]);
		break;
	}
	return w;
}

nserror download_data(struct gui_download_window *dw, const char *data, unsigned int size)
{
	int n;

	n = write(dw->fd, data, size);
	DBG("IN download_data (in:%zd out:%d errno:%d)", size, n, errno);
	return NSERROR_OK;
}

void download_error(struct gui_download_window *dw, const char *error_msg)
{
	DBG("IN download_error - error:%s", error_msg);
}

void download_done(struct gui_download_window *dw)
{
	DBG("IN download_done");
	close(dw->fd);
	free(dw);
}

static struct gui_download_table download_table = {
	.create = download_create,
	.data   = download_data,
	.error  = download_error,
	.done   = download_done
};

struct gui_download_table *plan9_download_table = &download_table;
