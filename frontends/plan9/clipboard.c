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

enum
{
	Block = 8192,
};

/**
 * Core asks front end for clipboard contents.
 *
 * \param  buffer  UTF-8 text, allocated by front end, ownership yeilded to core
 * \param  length  Byte length of UTF-8 text in buffer
 */
void plan9_paste(char **buffer, size_t *length)
{
	char *buf, *p;
	int fd, n, size, len;

	*buffer = NULL;
	*length = 0;
	if ((fd = open("/dev/snarf", O_RDONLY)) < 0)
		return;

	buf = NULL;
	size = 0;
	for (len = 0;; len += n) {
		if (size - len < Block) {
			size += Block;
			if ((p = realloc(buf, size+1)) == NULL)
				break;
			buf = p;
		}
		if ((n = read(fd, buf+len, Block)) <= 0)
			break;
	}
	close(fd);

	buf[len] = 0;
	*buffer = buf;
	*length = len;
}

void plan9_snarf(const char *buffer, size_t length)
{
	int fd;

	if ((fd = open("/dev/snarf", O_WRONLY)) >= 0) {
		write(fd, buffer, length);
		close(fd);
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
	plan9_snarf(buffer, length);
}

static struct gui_clipboard_table clipboard_table = {
	.get = plan9_paste,
	.set = gui_set_clipboard,
};

struct gui_clipboard_table *plan9_clipboard_table = &clipboard_table;
