#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <lib9.h>
#include <draw.h>
#include <event.h>
#include "utils/log.h"
#include "utils/utils.h"
#include "utils/nsurl.h"
#include "utils/messages.h"
#include "utils/nsoption.h"
#include "utils/string.h"
#include "desktop/download.h"
#include "netsurf/download.h"
#include "plan9/download.h"
#include "plan9/window.h"
#include "plan9/utils.h"


struct gui_download_window
{
	bool page;
	int fd;
	int ofd;
	long r;
	long s;
	char *data;
};

static bool can_page(struct download_context *ctx)
{
	char *mime;
	char *filename;

	mime = download_context_get_mime_type(ctx);
	if(page_accept_mimetype(mime))
		return true;
	/* some web servers do not send the Content-Type header
	   leading to netsurf using text/plain.
	   we do another check based on filename to be sure we
	   really cannot send to page */
	if(strncmp(mime, "text/plain", 10) == 0){
		filename = download_context_get_filename(ctx);
		if(page_accept_file(filename))
			return true;
	}
	return false;
}

struct gui_download_window *download_create(struct download_context *ctx, struct gui_window *parent)
{
	struct gui_download_window *w;
	int p[2], fd;
	long len;
	char buf[PATH_MAX+1] = {0}, cmd[1024] = {0};
	bool page;

	w = calloc(1, sizeof *w);
	if (w == NULL)
		return NULL;
	len = download_context_get_total_length(ctx);

	if (can_page(ctx)) {
		w->page = true;
		if (len == 0)
			len = 1024;
		w->data = calloc(len+1, sizeof(char));
		w->r = 0;
		w->s = len;
		if (w->data == NULL)
			return NULL;
		return w;
	}

	snprintf(buf, sizeof buf, download_context_get_filename(ctx));
	if(eenter("Save as:", buf, sizeof buf, &parent->m)<=0)
		return NULL;
	fd = open(buf, O_WRONLY|O_CREAT|O_TRUNC, 00644);
	if(fd < 0){
		DBG("unable to open '%s': %s", buf, strerror(errno));
		return NULL;
	}
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
		snprintf(cmd, sizeof cmd, "aux/statusbar %s", buf);
		execl("/bin/rc", "rc", "-c", cmd, 0);
		fprintf(stderr, "exec failed\n");
		exit(1);
	default:
		w->page = false;
		w->fd = p[1];
		w->ofd = fd;
		w->r = 0;
		w->s = len;
		close(p[0]);
		break;
	}
	return w;
}

nserror download_data(struct gui_download_window *dw, const char *data, unsigned int size)
{
	int n;
	char buf[64];

	if (dw->page) {
		if (dw->r+size > dw->s) {
			if(dw->s * 2 < size)
				dw->s = dw->r + size + 1024;
			else
				dw->s *= 2;
			dw->data = realloc(dw->data, dw->s*sizeof(char));
		}
		memmove(dw->data+dw->r, data, size);
		dw->r += size;
	} else {
		n = write(dw->ofd, data, size);
		if (n < 0) {
			DBG("download error: %s", strerror(errno));
			return NSERROR_SAVE_FAILED;
		}
		if (!dw->page){
			dw->r += size;
			n = snprintf(buf, sizeof buf, "%ld %ld\n", dw->r, dw->s);
			write(dw->fd, buf, n);
		}
	}
	return NSERROR_OK;
}

void download_error(struct gui_download_window *dw, const char *error_msg)
{
	DBG("IN download_error - error:%s", error_msg);
}

void download_done(struct gui_download_window *dw)
{
	esetcursor(NULL);
	if (dw->page) {
		send_data_to_plumber("image", NULL, dw->data, dw->r);
		free(dw->data);
	} else {
		close(dw->fd);
		close(dw->ofd);
	}
	free(dw);
}

static struct gui_download_table download_table = {
	.create = download_create,
	.data   = download_data,
	.error  = download_error,
	.done   = download_done
};

struct gui_download_table *plan9_download_table = &download_table;
