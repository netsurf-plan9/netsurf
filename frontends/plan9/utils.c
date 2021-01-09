#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <lib9.h>
#include "utils/errors.h"
#include "utils/filepath.h"
#include "plan9/utils.h"

static int debug = 0;

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
		fprintf(stderr, "unable to create %s/lib/netsurf: %s\n", path, messages_get_errorcode(ret));
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

void
exec_netsurf(const char *cmd, const char *url)
{
	switch (rfork(RFPROC|RFFDG|RFNAMEG|RFENVG|RFNOWAIT)) {
	case -1:
		fprintf(stderr, "rfork failed\n");
		return;
	case 0:
		execl("/bin/netsurf", "netsurf", url, 0);
		fprintf(stderr, "exec failed\n");
		exit(1);
	}
}

void
DBG(const char *format, ...)
{
	char buf[1024];
	va_list ap;

	if(!debug)
		return;
	va_start(ap, format);
	vsnprintf(buf, sizeof buf - 1, format, ap);
	va_end(ap);
	fprintf(stderr, "(debug) %s\n", buf);
}
