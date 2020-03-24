#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>


#include "utils/nsurl.h"
#include "content/llcache.h"
#include "content/fetch.h"
#include "netsurf/misc.h"
#include "desktop/gui_internal.h"
#include "libwapcaplet/libwapcaplet.h"
#include "webfs.h"

// TODO: Condense these. There's no reason we need to divide up the work this much.
typedef enum {
	DATA_STATE_UNSTARTED,
	DATA_STATE_CLONED,
	DATA_STATE_REQUESTED,
	DATA_STATE_DATA_READY,
	DATA_STATE_DONE,
	DATA_STATE_ERROR,
} data_state;

typedef enum {
	HANDLE_STATE_START, // Have not sent LLCACHE_EVENT_HAD_HEADERS yet.
	HANDLE_STATE_DATA,  // In process of sending LLCACHE_EVENT_HAD_DATA.
	HANDLE_STATE_DONE,  // We are finished. Either by LLCACHE_EVENT_DONE or LLCACHE_EVENT_ERROR.
} handle_state;

struct webfs_data {
	data_state state;
	char *err;
	char *urls;
	char *urlenc;
	struct fetch_multipart_data *multipart;

	int ctlfd; // fd for /mnt/web/N/ctl
	int bodyfd; // fd for /mnt/web/N/body
	char *webdir;

	char *data;
	int datalen;

	int handle_refcount;
};

struct webfs_handle {
	handle_state state;
	llcache_handle_callback cb;
	void *pw;
	bool aborted;
	bool released;
	bool removed;
	int sendindex;

	struct webfs_data *data;

	llcache_handle *handle;
	
	struct webfs_handle *prev;
	struct webfs_handle *next;
};

struct webfs_handle *handle_head;
bool scheduled;
void update_webfs(void *ignored);

void add_node(struct webfs_handle *n) {
	n->prev = NULL;
	n->next = handle_head;
	if(handle_head) {
		handle_head->prev = n;
	}
	handle_head = n;
	if(!scheduled) {
		scheduled=true;
		guit->misc->schedule(10, update_webfs, NULL);
	}
}

void webfs_data_release(struct webfs_data *d) {
	if(d->urls != NULL) {
		free(d->urls);
	}
	if(d->urlenc != NULL) {
		free(d->urlenc);
	}
	if(d->multipart != NULL) {
		fetch_multipart_data_destroy(d->multipart);
	}
	if(d->ctlfd != -1) {
		close(d->ctlfd);
		d->ctlfd = -1;
	}
	if(d->bodyfd != -1) {
		close(d->bodyfd);
		d->bodyfd = -1;
	}
	if(d->webdir != NULL) {
		free(d->webdir);
	}
	if(d->data != NULL) {
		free(d->data);
	}
	free(d);
}

void try_release_node(webfs_handle *wh) {
	if(wh->released && wh->removed) {
		wh->data->handle_refcount-=1;
		if(wh->data->handle_refcount <= 0) {
			webfs_data_release(wh->data);
			wh->data = NULL;
		}
		free(wh);
	}
}

void remove_node(struct webfs_handle *n) {
	if(n->prev) {
		n->prev->next = n->next;
	}
	if(n->next) {
		n->next->prev = n->prev;
	}
	if(handle_head == n) {
		handle_head = n->next;
	}
	n->prev = NULL;
	n->next = NULL;
	n->removed = true;
	try_release_node(n);
}

void webfs_send_data(llcache_handle *handle, webfs_handle *wh) {
	int tosend = 1024;
	if(wh->data->datalen - wh->sendindex < tosend) {
		tosend = wh->data->datalen - wh->sendindex;
		if(tosend == 0) return;
	}
	llcache_event ev;
	ev.type = LLCACHE_EVENT_HAD_DATA;
	ev.data.data.buf = (const unsigned char *)wh->data->data + wh->sendindex;
	ev.data.data.len = tosend;
	
	nserror err = wh->cb(handle, &ev, wh->pw);
	if(err == NSERROR_NEED_DATA) {
		return;		
	}
	else if (err != NSERROR_OK) {
		// TODO: proper error handling
		fprintf(stderr, "[DBG]: UNKNOWN ERROR!\n");
		return;
	}
	else {
		wh->sendindex += tosend;
	}
}

/**
 * Retrieve the post-redirect URL of a low-level cache object
 *
 * \param handle  Handle to retrieve URL from
 * \return Post-redirect URL of cache object
 */
nsurl *webfs_handle_get_url(const webfs_handle *wh) {
	nsurl *url;
	nserror err = nsurl_create(wh->data->urls, &url);
	if(err != NSERROR_OK) {
		return NULL;
	}
	return url;
}

/**
 * Change the callback associated with a low-level cache handle
 *
 * \param handle  Handle to change callback of
 * \param cb      New callback
 * \param pw      Client data for new callback
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_change_callback(webfs_handle *wh,
		llcache_handle_callback cb, void *pw) {
	wh->cb = cb;
	wh->pw = pw;
	return NSERROR_OK;
}

/**
 * Abort a low-level fetch, informing all users of this action.
 *
 * \param handle  Handle to abort
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_abort(webfs_handle *wh) {
	wh->aborted = true;
	return NSERROR_OK;
}

/**
 * Retrieve source data of a low-level cache object
 *
 * \param handle  Handle to retrieve source data from
 * \param size    Pointer to location to receive byte length of data
 * \return Pointer to source data
 */
const uint8_t *webfs_handle_get_source_data(const webfs_handle *wh,
		size_t *size) {
	*size = wh->data->datalen;
	return (const uint8_t *)wh->data->data;
	return NULL;
}

/**
 * Release a low-level cache handle
 *
 * \param handle  Handle to release
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_release(webfs_handle *wh) {
	wh->released = true;
	try_release_node(wh);
	return NSERROR_OK;
}

/**
 * Invalidate cache data for a low-level cache object
 *
 * \param handle  Handle to invalidate
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_invalidate_cache_data(	) {
	// TODO
	return NSERROR_OK;
}

/**
 * Clone a low-level cache handle, producing a new handle to
 * the same fetch/content.
 *
 * \param handle  Handle to clone
 * \param result  Pointer to location to receive cloned handle
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_clone(llcache_handle *handle, webfs_handle *wh, webfs_handle **result) {
	// TODO (This might be enough already, but revisit and make sure.)
	webfs_handle *cp = malloc(sizeof (webfs_handle));
	*cp=*wh;
	cp->handle = handle;
	cp->sendindex = 0;
	cp->state = HANDLE_STATE_START;
	cp->next = NULL;
	cp->prev = NULL;
	*result = cp;
	add_node(cp);
	return NSERROR_OK;
}

char *transform_header_name(char *key) {
	// TODO I haven't looked at how webfs does this.
	// this is just a guess. Need to actually do this
	// right.
	char *new = malloc(strlen(key));
	char *newp=new;
	for(char *c = key; *c != 0; c++) {
		if((*c >='a' && *c <='z') || (*c >='0' && *c <= '9')){
			*newp++ = *c;
		}
		else if(*c >='A' && *c <= 'Z') {
			*newp++ = (*c + 32);
		}
	}
	*newp=0;
	return new;
}

/**
 * Retrieve a header value associated with a low-level cache object
 *
 * \param handle  Handle to retrieve header from
 * \param key     Header name
 * \return Header value, or NULL if header does not exist
 *
 * \todo Make the key an enumeration, to avoid needless string comparisons
 * \todo Forcing the client to parse the header value seems wrong.
 *       Better would be to return the actual value part and an array of
 *       key-value pairs for any additional parameters.
 * \todo Deal with multiple headers of the same key (e.g. Set-Cookie)
 */
const char *webfs_handle_get_header(const webfs_handle *wh,
		const char *key) {
	// TODO Make this function not terrible.
	char *newkey = transform_header_name(key);
	char fname[1024]; // TODO: Again, static? Too big?
	sprintf(fname, "%s%s", wh->data->webdir, newkey);

	int hfd = open(fname, O_RDONLY);
	if(hfd < 0) {
		return NULL;
	}

	char header_content[4097]; // TODO: Again, really far too big and static.
	int n = read(hfd, header_content, 4096);
	header_content[n]=0;
	char *headerbuf = malloc(n+1);
	memcpy(headerbuf, header_content, n+1);
	close(hfd);
	return headerbuf;
}

/**
 * Cause the low-level cache to attempt to perform cleanup.
 *
 * No guarantees are made as to whether or not cleanups will take
 * place and what, if any, space savings will be made.
 *
 * \param purge Any objects held in the cache that are safely removable will
 *              be freed regardless of the configured size limits.
 */
void webfs_clean(bool purge) {
	// TODO
}

/**
 * Determine if the same underlying object is referenced by the given handles
 *
 * \param a  First handle
 * \param b  Second handle
 * \return True if handles reference the same object, false otherwise
 */
bool webfs_handle_references_same_object(const webfs_handle *a,
		const webfs_handle *b) {
	return a->data == b->data;
}

/**
 * Force a low-level cache handle into streaming mode
 *
 * \param handle  Handle to stream
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_force_stream(webfs_handle *wh) {
	// TODO
	return NSERROR_NOT_IMPLEMENTED;
}

/**
 * Initialise the low-level cache
 *
 * \param parameters cache configuration parameters.
 * \return NSERROR_OK on success, appropriate error otherwise.
 */
nserror webfs_initialise(const struct llcache_parameters *parameters) {
	handle_head = NULL;
	scheduled=false;
	return NSERROR_OK;
}

/**
 * Finalise the low-level cache
 */
void webfs_finalise(void) {
	// TODO
}

void start_data(struct webfs_data *d) {
	int ctlfd = open("/mnt/web/clone", O_RDWR);
	if(ctlfd < 0) {
		char *e = strerror(errno);
		fprintf(stderr, "[DBG]: Failed to clone webfs. Error: %s\n", e);
		d->state = DATA_STATE_ERROR;
		d->err = "Failed to clone webfs."; // TODO: report strerror?
		return;
	}
	char webd[10]; // 9 digits is plenty.
	memset(webd, 0, 10);
	int n = read(ctlfd, webd, 9);
	webd[n-1]=0;

	d->webdir = malloc(11 + strlen(webd)); // length of "/mnt/web//" + length of webd + NULL
	n = sprintf(d->webdir, "/mnt/web/%s/", webd);
	d->state = DATA_STATE_CLONED;
	d->ctlfd = ctlfd;
}

void send_request(struct webfs_data *d) {
	char *s;
	int fd, n;
	struct fetch_multipart_data *part;

	s = calloc(3+1+strlen(d->urls)+1, sizeof(char));
	if(s == NULL){
		fprintf(stderr, "[DBG] send_request: OOM\n");
		d->state = DATA_STATE_ERROR;
		d->err = "OOM";
		return;
	}
	sprintf(s, "url %s", d->urls);
	n = write(d->ctlfd, s, strlen(s));
	free(s);
	if(n <= 0) {
		char *e = strerror(errno);
		fprintf(stderr, "[DBG]: [%s] Failed to open URL. Error: %s\n", d->urls, e);
		d->state = DATA_STATE_ERROR;
		d->err = "Failed to clone webfs."; // TODO: report strerror?
		return;
	}
	/* POST */
	if(d->urlenc != NULL || d->multipart != NULL) {
		if(d->multipart != NULL)
			write(d->ctlfd, "headers Content-Type: multipart/form-data; boundary=HJBOUNDARY", 62);
		s = calloc(strlen(d->webdir)+8+1, sizeof(char));
		sprintf(s, "%spostbody", d->webdir);
		fd = open(s, O_WRONLY);
		free(s);
		if(fd < 0) {
			char *e = strerror(errno);
			fprintf(stderr, "[DBG] failed to open postbody file; %s\n", e);
			d->state = DATA_STATE_ERROR;
			d->err = "Failed to open POST body";
			return;
		}
		if(d->urlenc != NULL) {
			n = write(fd, d->urlenc, strlen(d->urlenc));
			if(n <= 0) {
				close(fd);
				char *e = strerror(errno);
				fprintf(stderr, "[DBG] Could not write post data: Error: %s\n", e);
				d->state = DATA_STATE_ERROR;
				d->err = "Could not write post data."; // TODO: report strerror?
				return;
			}
		} else if(d->multipart != NULL) {
			for(part = d->multipart; part; part = part->next) {
				if(part->file) {
					fprintf(stderr, "[DBG] webfs::send_request: file part not implemented\n");
					continue;
				}
				n = snprintf(NULL, 0, "--HJBOUNDARY\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", part->name, part->value);
				s = malloc((n+1)*sizeof(char));
				sprintf(s, "--HJBOUNDARY\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", part->name, part->value);
				write(fd, s, n);
				free(s);
			}
			write(fd, "--HJBOUNDARY--\r\n", strlen("--HJBOUNDARY--\r\n"));
		}
		close(fd);
	}
	d->state = DATA_STATE_REQUESTED;
}

void start_body(struct webfs_data *d) {
	char fname[1024];
	sprintf(fname, "%sbody", d->webdir);
	int bodyfd = open(fname, O_RDONLY);
	if(bodyfd < 0) {
		char *e = strerror(errno);
		fprintf(stderr, "[DBG]: [%s] Failed to read body. Error: %s\n", d->urls, e);
		d->state = DATA_STATE_ERROR;
		d->err = "Failed to clone webfs."; // TODO: report strerror?
		return;
	}
	close(d->ctlfd); //TODO Can we use this?
	d->ctlfd = -1;
	d->bodyfd = bodyfd;
	d->state = DATA_STATE_DATA_READY;

	// Check if our URL has been redirected.
	sprintf(fname, "%sparsed/url", d->webdir);
	int urlfd = open(fname, O_RDONLY);
	if(urlfd < 0) {
		char *e = strerror(errno);
		fprintf(stderr, "[DBG]: [%s] Failed to read body. Error: %s\n", d->urls, e);
		d->state = DATA_STATE_ERROR;
		d->err = "Failed to read parsed URL.";
		return;
	}
	char urlbuf[1024];
	int n = read(urlfd, urlbuf, 1023);
	urlbuf[n] = 0;
	if(strcmp(urlbuf, d->urls) != 0) {
		char *newUrls = calloc(1, strlen(urlbuf) + 1);
		strcpy(newUrls, urlbuf);
		free(d->urls);
		d->urls = newUrls;
	}
	close(urlfd);
}

void read_data(struct webfs_data *d) {
	llcache_event ev;
	char buf[4097];
	buf[4096] = 0;

	int n = read(d->bodyfd, buf, 4096);
	if(n < 0) {
		char *e = strerror(errno);
		fprintf(stderr, "[DBG]: [%s] Failed TO READ FROM BODY. Error: %s\n", d->urls, e);
		d->state = DATA_STATE_ERROR;
		return;
	}

	if(n == 0) {
		close(d->bodyfd); /* XXX */
		d->state = DATA_STATE_DONE;
		return;
	}

	if(d->data == NULL) {
		d->data = malloc(n);
	} else {
		d->data = realloc(d->data, d->datalen + n);
	}
	memcpy(d->data + d->datalen, buf, n);
	d->datalen+=n;
}

void update_data(struct webfs_data *d) {
	if(d->state == DATA_STATE_ERROR || d->state == DATA_STATE_DONE) {
		return;
	}
	else if(d->state == DATA_STATE_UNSTARTED) {
		start_data(d);
	}
	else if(d->state == DATA_STATE_CLONED) {
		send_request(d);
	}
	else if(d->state == DATA_STATE_REQUESTED) {
		start_body(d);
	}
	else if(d->state = DATA_STATE_DATA_READY) {
		read_data(d);
	}
}

bool llcache_progress(void *h) {
	struct webfs_handle *wh = h;
	llcache_handle *handle = wh->handle;

	llcache_event ev;

	// This handle is done. Do nothing.
	if(wh->state == HANDLE_STATE_DONE || wh->aborted || wh->released) {
		return false;
	}

	// The data has received an error but we haven't reported it
	// through the handle yet.
	if(wh->data->state == DATA_STATE_ERROR) {
		wh->state = HANDLE_STATE_DONE;
		ev.type = LLCACHE_EVENT_ERROR;
		wh->cb(handle, &ev, wh->pw);
		return false;
	}
	

	// Make progress on the underlying data.
	update_data(wh->data);

	// Main if/else chain. Starting with HANDLE_STATE_START
	if(wh->state == HANDLE_STATE_START) {
		if (wh->data->state >= DATA_STATE_DATA_READY) {
			// Data is ready. 
			ev.type = LLCACHE_EVENT_HAD_HEADERS;
			wh->cb(handle, &ev, wh->pw);
			wh->state = HANDLE_STATE_DATA;
		}
	}
	else if(wh->state == HANDLE_STATE_DATA) {
		// We know data is ready.
		if(wh->sendindex == wh->data->datalen) {
			// We're at the end. Check if the data is still coming.
			if(wh->data->state == DATA_STATE_DONE) {
				// We sent all the data and no more is coming!
				ev.type = LLCACHE_EVENT_DONE;
				wh->cb(handle, &ev, wh->pw);
				wh->state = HANDLE_STATE_DONE;
				return true;
			}
			// We have no data now, but more is coming.
			return true;
		}
		else {
			webfs_send_data(handle, wh);
		}
	}
	return true;
}

void update_webfs(void *ignored) {
	struct webfs_handle *wh = handle_head;
	if(wh == NULL) {
		// Return early to stop scheduling.
		scheduled=false;
		return;
	}
	while(wh != NULL) {
		struct webfs_handle *next = wh->next;
		if(!llcache_progress(wh)) {
			remove_node(wh);
		}
		wh = next;
	}
	guit->misc->schedule(10, update_webfs, NULL);
}

/**
 * Retrieve a handle for a low-level cache object
 *
 * \param url      URL of the object to fetch
 * \param flags    Object retrieval flags
 * \param referer  Referring URL, or NULL if none
 * \param post     POST data, or NULL for a GET request
 * \param cb       Client callback for events
 * \param pw       Pointer to client-specific data
 * \param result   Pointer to location to recieve cache handle
 * \return NSERROR_OK on success, appropriate error otherwise
 */
nserror webfs_handle_retrieve(nsurl *url, uint32_t flags,
		nsurl *referer, const llcache_post_data *post,
		llcache_handle_callback cb, void *pw, llcache_handle *handle,
		webfs_handle **result) {

	// We are going to ignore flags for now.

	char *scheme = lwc_string_data(nsurl_get_component(url, NSURL_SCHEME));
	if(strcmp("http", scheme) != 0 && strcmp("https", scheme) != 0) {
		// TODO proper error handling.
		fprintf(stderr, "[DBG]: Cannot handle the scheme [%s]\n", scheme);
		abort();
	}

	webfs_handle *wh = malloc(sizeof *wh);
	wh->state = HANDLE_STATE_START;
	wh->cb = cb;
	wh->pw = pw;
	wh->aborted = false;
	wh->released = false;
	wh->removed = false;
	wh->sendindex = 0;
	wh->handle = handle;
	wh->next = NULL;
	wh->prev = NULL;
	wh->data = malloc(sizeof *wh->data);
	
	wh->data->state = DATA_STATE_UNSTARTED;
	char *url_cstr = nsurl_access(url);
	wh->data->urls = malloc(strlen(url_cstr) + 1);
	strcpy(wh->data->urls, url_cstr);
	wh->data->urlenc = NULL;
	wh->data->multipart = NULL;
	if(post != NULL) {
		switch(post->type) {
		case LLCACHE_POST_URL_ENCODED:
			wh->data->urlenc = strdup(post->data.urlenc);
			break;
		case LLCACHE_POST_MULTIPART:
			wh->data->multipart = fetch_multipart_data_clone(post->data.multipart);
			break;
		default:
			fprintf(stderr, "[DBG] encode_post_data: unknown post request type %d\n", post->type);
			break;
		}
	}
	wh->data->ctlfd = -1;
	wh->data->bodyfd = -1;
	wh->data->webdir = NULL;
	wh->data->data = NULL;
	wh->data->datalen = 0;
	wh->data->handle_refcount = 1;
	add_node(wh);

	*result = wh;

	return NSERROR_OK;
}
