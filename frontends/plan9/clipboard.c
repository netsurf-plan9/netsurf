#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "utils/log.h"
#include "netsurf/browser_window.h"
#include "netsurf/clipboard.h"
#include "plan9/utils.h"
#include "plan9/clipboard.h"

/**
 * Core asks front end for clipboard contents.
 *
 * \param  buffer  UTF-8 text, allocated by front end, ownership yeilded to core
 * \param  length  Byte length of UTF-8 text in buffer
 */
static void gui_get_clipboard(char **buffer, size_t *length)
{
	int fd;
	char *buf;
	int n, size, len;

	fd = open("/dev/snarf", O_RDONLY);
	if (fd < 0)
		return;
	size = 4096; /* XXX do this properly */
	buf = calloc(size, sizeof(char));
	if (buf == NULL)
		return;
	n = read(fd, buf, size);
	if (n < 0) {
		*buffer = NULL;
		*length = 0;
	} else {
		*length = n;
		*buffer = calloc(n + 1, sizeof(char));
		memcpy(*buffer, buf, n);
	}
}

/**
 * Core tells front end to put given text in clipboard
 *
 * \param  buffer    UTF-8 text, owned by core
 * \param  length    Byte length of UTF-8 text in buffer
 * \param  styles    Array of styles given to text runs, owned by core, or NULL
 * \param  n_styles  Number of text run styles in array
 */
static void gui_set_clipboard(const char *buffer, size_t length,
		nsclipboard_styles styles[], int n_styles)
{
	int fd;

	fd = open("/dev/snarf", O_WRONLY);
	if (fd < 0)
		return;
	write(fd, buffer, length);
	close(fd);
}

static struct gui_clipboard_table clipboard_table = {
	.get = gui_get_clipboard,
	.set = gui_set_clipboard,
};

struct gui_clipboard_table *plan9_clipboard_table = &clipboard_table;
