#include <stdio.h>
#include <string.h>
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
	return file_type(unix_path);
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
