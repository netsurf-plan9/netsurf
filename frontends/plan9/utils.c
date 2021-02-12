#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <time.h>
#include <lib9.h>
#include <plumb.h>
#include <errno.h>
#include "utils/errors.h"
#include "utils/log.h"
#include "utils/filepath.h"
#include "utils/utils.h"
#include "plan9/utils.h"

bool log_debug = false;

static char* userdir(void)
{
	static char *home = NULL;
	
	if (home == NULL) {
		home = getenv("home");
		if (home == NULL) {
			home = "/tmp";
		}
	}
	return home;
}

char* userdir_file(char *filename)
{
	nserror ret;
	char buf[PATH_MAX+1];
	char *home, *path;

	if (filename == NULL) {
		return NULL;
	}
	home = userdir();
	snprint(buf, sizeof buf, "%s/lib/netsurf/%s", home, filename);
	ret = netsurf_mkdir_all(buf);
	if (ret != NSERROR_OK) {
		fprintf(stderr, "unable to create %s/lib/netsurf: %s\n", home, messages_get_errorcode(ret));
		return NULL;
	}
	path = strdup(buf);
	return path;
}

char* read_file(char *path, int *size)
{
	char *buf;
	int fd, n, s, r;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		DBG("unable to open file %s: %r");
		return NULL;
	}
	n = 0;
	s = 256;
	buf = malloc(s * sizeof(char));
	if (buf == NULL) {
		DBG("unable to allocate memory: %r");
		close(fd);
		return NULL;
	}
	for (;;) {
		r = read(fd, buf+n, s-n);
		if (r == 0)
			break;
		if (r == -1) {
			free(buf);
			return NULL;
		}
		n += r;
		if (n == s) {
			s *= 1.5;
			buf = realloc(buf, s*sizeof(char));
		}
	}
	buf[n] = '\0';
	*size = n;
	close(fd);
	return buf;
}

char* file_fullpath(char *filename)
{
	int fd, n;
	char buf[256], *path;

	path = NULL;
	fd = open(filename, 0); /* 0: OREAD */
	if(fd < 0)
		return NULL;
	n = _FD2PATH(fd, buf, sizeof buf);
	if(n == 0)
		path = strdup(buf);
	close(fd);
	return path;
}

char*
file_ext(const char *filename)
{
	char *d;

	d = strrchr(filename, '.');
	if (d == 0)
		return NULL;
	return d+1;
}

const char*
file_type(const char *path)
{
	char *mime_types[] = {
		"html",	"text/html",
		"htm",	"text/html",
		"css",	"text/css",
		"f79",	"text/css",
		"jpg",	"image/jpeg",
		"jpeg", "image/jpeg",
		"gif",	"image/gif",
		"png",	"image/png",
		"b60",	"image/png",
		"jng",	"image/jng",
		"svg",	"image/svg",
		"bmp",	"image/bmp",
		"ps",	"application/postscript",
		"pdf",	"application/pdf",
		0
	};
	char *ext;
	int i;

	ext = file_ext(path);
	if (ext == NULL)
		return "text/plain";
	for(i = 0; mime_types[i] != NULL; i += 2) {
		if(strcasecmp(ext, mime_types[i]) == 0)
			return mime_types[i+1];
	}
	return "text/plain";
}

bool
page_accept_mimetype(char *mime)
{
	char *types[] = {
		"application/pdf",
		"application/postscript",
		"image/",
		0
	};
	int i;

	for(i = 0; types[i] != NULL; i++) {
		if(strncmp(mime, types[i], strlen(types[i])) == 0)
			return true;
	}
	return false;
}

bool
page_accept_file(char *filename)
{
	char *type;

	type = file_type(filename);
	return page_accept_mimetype(type);
}

int
send_to_plumber(const char *text)
{
	int fd;

	fd = plumbopen("send", 1); /* 1 = OWRITE */
	if (fd <= 0)
		return -1;
	plumbsendtext(fd, "netsurf", NULL, NULL, text);
	close(fd);
	return 0;
}

static Plumbattr*
plumbattr(char *name, char *value)
{
	Plumbattr *a;

	a = malloc(sizeof *a);
	if (a == NULL)
		return NULL;
	a->name = name;
	a->value = value;
	a->next = NULL;
}

int
send_data_to_plumber(char *dst, char *filename, char *data, int ndata)
{
	int fd, r;
	Plumbmsg *m;
	Plumbattr *a;

	r = 0;
	m = malloc(sizeof *m);
	if (m == NULL) {
		NSLOG(netsurf, WARNING,
			"unable to allocate memory for plumb message: %s",
			strerror(errno));
		r = -1;
		goto done;
	}
	m->src = "netsurf";
	m->dst = dst;
	m->wdir = NULL;
	m->type = "text";
	m->data = data;
	m->ndata = ndata;
	m->attr = NULL;
	a = plumbattr("action", "showdata");
	if(a == NULL) {
		NSLOG(netsurf, WARNING,
			"unable to allocate memory for plumb attribute: %s",
			strerror(errno));
		r = -1;
		goto done;
	}
	m->attr = plumbaddattr(m->attr, a);
	if (filename != NULL) {
		a = plumbattr("filename", strdup(filename));
		if(a == NULL) {
			NSLOG(netsurf, WARNING,
				"unable to allocate memory for plumb attribute: %s",
				strerror(errno));
			r = -1;
			goto done;
		}
		m->attr = plumbaddattr(m->attr, a);
	}	
	fd = plumbopen("send", 1);
	if (fd < 0) {
		NSLOG(netsurf, WARNING,
			"unable to open plumb send port: %s", strerror(errno));
		r = -1;
		goto done;
	}
	if (plumbsend(fd, m) < 0) {
		NSLOG(netsurf, WARNING,
			"unable to send plumb message: %s", strerror(errno));
		r = -1;
	}
done:
	free(a);
	free(m);
	return r;
}

void
exec_netsurf(const char *url)
{
	char buf[1024] = {0};

	switch (rfork(RFPROC|RFNOWAIT|RFNOTEG|RFNAMEG|RFENVG|RFFDG)) {
	case -1:
		fprintf(stderr, "rfork failed\n");
		return;
	case 0:
		snprintf(buf, sizeof buf, "window %s '''%s'''", "netsurf", url);
		execl("/bin/rc", "rc", "-c", buf, 0);
		fprintf(stderr, "exec failed\n");
		exit(1);
	}
}

void
DBG(const char *format, ...)
{
	va_list ap;
	time_t t = time(NULL);
	struct tm *tm;
	char buf[20];

	if(!log_debug)
		return;
	tm = localtime(&t);
	strftime(buf, sizeof buf, "%Y/%m/%d %H:%M:%S", tm);
	fprintf(stderr, "%s (debug) ", buf);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}
