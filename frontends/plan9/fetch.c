#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <draw.h>
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/filepath.h"
#include "netsurf/fetch.h"
#include "plan9/fetch.h"
#include "plan9/utils.h"

extern char **respaths;

/**
 * Determine the MIME type of a local file.
 *
 * @note used in file fetcher
 *
 * \param unix_path Unix style path to file on disk
 * \return Pointer to MIME type string (should not be freed) -
 *	   invalidated on next call to fetch_filetype.
 */
const char*
fetch_filetype(const char *unix_path)
{
	int l;

	l = strlen(unix_path);
	if (2 < l && strcasecmp(unix_path + l - 3, "css") == 0)
		return "text/css";
	if (2 < l && strcasecmp(unix_path + l - 3, "f79") == 0)
		return "text/css";
	if (2 < l && strcasecmp(unix_path + l - 3, "jpg") == 0)
		return "image/jpeg";
	if (3 < l && strcasecmp(unix_path + l - 4, "jpeg") == 0)
		return "image/jpeg";
	if (2 < l && strcasecmp(unix_path + l - 3, "gif") == 0)
		return "image/gif";
	if (2 < l && strcasecmp(unix_path + l - 3, "png") == 0)
		return "image/png";
	if (2 < l && strcasecmp(unix_path + l - 3, "b60") == 0)
		return "image/png";
	if (2 < l && strcasecmp(unix_path + l - 3, "jng") == 0)
		return "image/jng";
	if (2 < l && strcasecmp(unix_path + l - 3, "svg") == 0)
		return "image/svg";
	if (2 < l && strcasecmp(unix_path + l - 3, "bmp") == 0)
		return "image/bmp";
	if (3 < l && strcasecmp(unix_path + l - 4, "html") == 0)
		return "text/html";
	if (2 < l && strcasecmp(unix_path + l - 3, "htm") == 0)
		return "text/html";
	return "text/plain";
}

/**
 * Translate resource to full url.
 *
 * @note Only used in resource fetcher
 *
 * Transforms a resource: path into a full URL. The returned URL
 * is used as the target for a redirect. The caller takes ownership of
 * the returned nsurl including unrefing it when finished with it.
 *
 * \param path The path of the resource to locate.
 * \return A string containing the full URL of the target object or
 *         NULL if no suitable resource can be found.
 */
struct nsurl*
fetch_get_resource_url(const char *path)
{
	char buf[PATH_MAX];
	nsurl *url;

	url = NULL;
	if(strcmp(path, "favicon.ico")==0)
		path = "favicon.png";
	netsurf_path_to_nsurl(filepath_sfind(respaths, buf, path), &url);
	return url;
}

static struct gui_fetch_table fetch_table = {
	.filetype = fetch_filetype,
	.get_resource_url = fetch_get_resource_url,
};

struct gui_fetch_table *plan9_fetch_table = &fetch_table;
