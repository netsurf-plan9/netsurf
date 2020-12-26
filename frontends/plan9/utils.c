#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <lib9.h>
#include "plan9/utils.h"

static int debug = 0;

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
