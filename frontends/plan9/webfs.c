#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/limits.h>
#include <lock.h>
#include <libwapcaplet/libwapcaplet.h>
#include "content/fetch.h"
#include "content/fetchers.h"
#include "utils/log.h"
#include "utils/messages.h"
#include "utils/corestrings.h"
#include "utils/nsurl.h"
#include "utils/string.h"
#include "plan9/utils.h"

enum
{
	Snone,
	Sstarted,
	Sfinished,
	Sredirect,
	Snotmodified,
	Saborted,
	Serror,
};

struct webfs_fetch
{
	struct fetch *parent;
	struct nsurl *url;
	char *post_url;
	struct fetch_multipart_data *post_data;
	char **headers;
	int id;
	int fd;
	int state;
	char *data;
	int data_len;
};

struct webfs_fetch_list
{
	struct webfs_fetch *fetch;
	struct webfs_fetch_list *next;
};

static int fetcher_init_count = 0;
static struct webfs_fetch_list *fetch_list = NULL;

static void fetch_list_add(struct webfs_fetch *f)
{
	struct webfs_fetch_list *l;

	l = malloc(sizeof *l);
	l->fetch = f;
	l->next = fetch_list;
	fetch_list = l;
}

static bool webfs_acceptable(const struct nsurl *url)
{
	return nsurl_has_component(url, NSURL_HOST);
}

static void* webfs_setup(struct fetch *parent_fetch, struct nsurl *url,
		bool only_2xx, bool downgrade_tls, const char *post_urlenc,
		const struct fetch_multipart_data *post_multipart,
		const char **headers)
{
	struct webfs_fetch *f;
	int n;

	f = calloc(1, sizeof *f);
	if (f == NULL)
		return NULL;
	f->fd = -1;
	f->state = Snone;
	f->parent = parent_fetch;
	f->url = nsurl_ref(url);
	if (post_urlenc != NULL) {
		f->post_url = strdup(post_urlenc);
	}
	if (post_multipart != NULL) {
		f->post_data = fetch_multipart_data_clone(post_multipart);
	}
	if (headers != NULL){
		for (n = 0; headers[n] != NULL; n++)
			;
		f->headers = calloc(n+1, sizeof(char*));
		for (n = 0; headers[n] != NULL; n++) {
			f->headers[n] = strdup(headers[n]);
		}
	}
	return f;
}

static char* header_string(char *path, char *name, int *size)
{
	char *h, *s;
	int n;
	s = read_file(path, &n);
	if (s == NULL) {
		DBG("cannot open %s: %r", path);
		return NULL;
	}
	n = strlen(name) + 1 + 1 + n + 1; /* <name>: <s>\0 */
	h = malloc(n*sizeof(char));
	if ((n = snprintf(h, n, "%s: %s", name, s)) != n) {
		DBG("unable to create header string: %r");
		free(s);
		free(h);
		return NULL;
	}
	*size = n;
	free(s);
	return h;
}

static char *Headers[] = {
	"contentlength", "Content-Length",
	"contentlocation", "Content-Location",
	"contenttype", "Content-Type",
	"lastmodified", "Last-Modified",
	"date", "Date",
	"cachecontrol", "Cache-Control",
	"connection", "Connection",
	"transferencoding", "Transfer-Encoding",
	"etag", "ETag",
	"age", "Age",
	"expires", "Expires",
	NULL,
};

static void send_headers(struct webfs_fetch *f)
{
	char path[PATH_MAX+1], *s;
	fetch_msg msg;
	int i, n;

	msg.type = FETCH_HEADER;
	for (i = 0; Headers[i] != NULL; i += 2) {
		snprintf(path, sizeof path, "/mnt/web/%d/%s", f->id, Headers[i]);
		if (access(path, R_OK) == 0) {
			s = header_string(path, Headers[i+1], &n);
			if (s == NULL) {
				DBG("NULL header_string");
				continue;
			}
			msg.data.header_or_data.buf = (const uint8_t*)s;
			msg.data.header_or_data.len = n;
			fetch_send_callback(&msg, f->parent);
		}
	}
}

static int send_request(int cfd, struct webfs_fetch *f)
{
	char path[PATH_MAX+1], *s;
	int i, n, fd;
	size_t l;
	struct fetch_multipart_data *part;

	/* URL */
	n = 3+1+strlen(nsurl_access(f->url))+1;
	s = malloc(n*sizeof(char)); /* 'url <url>\0' */
	if ((n = snprintf(s, n, "url %s", nsurl_access(f->url))) != n) {
		DBG("unable to create url string: %s", strerror(errno));
		free(s);
		return -1;
	}
	s[n] = 0;
	if ((n = write(cfd, s, n)) != n) {
		DBG("could not write url: %s", strerror(errno));
		free(s);
		return -1;
	}
	free(s);

	/* HEADERS */
	if (f->headers != NULL) {
		for(i = 0; f->headers[i] != NULL; i++) {
			s = NULL;
			l = 0;
			if (snstrjoin(&s, &l, ' ', 2, "headers", f->headers[i]) != NSERROR_OK) {
				return -1;
			}
			if ((n = write(cfd, s, l)) != l) {
				DBG("could not write headers: %s", strerror(errno));
				free(s);
				return -1;
			}
		}
	}

	/* POST */
	if (f->post_url != NULL || f->post_data != NULL) {
		if(f->post_data != NULL)
			write(cfd, "headers Content-Type: multipart/form-data; boundary=HJBOUNDARY", 62);
		if(snprintf(path, sizeof path, "/mnt/web/%d/postbody", f->id) <= 0) {
			DBG("could not create postbody path: %s", strerror(errno));
			return -1;
		}
		fd = open(path, O_WRONLY);
		if(fd < 0) {
			DBG("could not open postbody path: %s\n", strerror(errno));
			return -1;
		}
		if(f->post_url != NULL) {
			l = strlen(f->post_url);
			if ((n = write(fd, f->post_url, l)) != l) {
				close(fd);
				DBG("could not write post data: %s", strerror(errno));
				return -1;
			}
		} else if(f->post_data != NULL) {
			for(part = f->post_data; part != NULL; part = part->next) {
				if(part->file == true) {
					DBG("webfs: file part not implemented");
					continue;
				}
				n = snprintf(NULL, 0, "--HJBOUNDARY\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", part->name, part->value);
				s = malloc((n+1)*sizeof(char));
				snprintf(s, n+1, "--HJBOUNDARY\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n", part->name, part->value);
				write(fd, s, n);
				free(s);
			}
			write(fd, "--HJBOUNDARY--\r\n", strlen("--HJBOUNDARY--\r\n"));
		}
		close(fd);
	}
}

static bool should_redirect(int cfd, struct webfs_fetch *f)
{
	char path[PATH_MAX+1], *u;
	int n;

	if (snprintf(path, sizeof path, "/mnt/web/%d/parsed/url", f->id) <= 0) {
		DBG("could not create parsed/url path: %s", strerror(errno));
		return false;
	}
	u = read_file(path, &n);
	if(strncmp(nsurl_access(f->url), u, n) != 0) {
		f->state = Sredirect;
		f->data = u;
		return true;
	}
	return false;
}

static void handle_error(struct webfs_fetch *f, char *path, char *err)
{
	fetch_msg msg;
	char *p;
	int len;
	long code;

	p = NULL;
	code = 0;
	len = strlen(path);
	/* HTTP error returned from webfs with format:
	 * '/mnt/web/XXX/body' <CODE> <MESSAGE>
	 */
	if (strncmp(err+1, path, len) == 0) {
		p = err+1+len+1+1;
		code = strtol(p, NULL, 10);
	}
	/* message needs to be sent after start so we add the fetch 
	 * to the fetch list. It will be processed during the next
	 * webfs_poll() call and handled appropriately 
	 */
	switch (code) {
	case 304:
		fetch_set_http_code(f->parent, code);
		f->state = Snotmodified;
		fetch_list_add(f);
		break;
	case 0:
		code = 400;
	default:
		fetch_set_http_code(f->parent, code);
		f->state = Serror;
		f->data = p != NULL ? p : err;
		fetch_list_add(f);
		break;
	}
}

/* 
Not useful as such as cookies are handled by webcookies(1).
This would be useful if we add a cookie explorer window though
 */
static void read_cookies(struct webfs_fetch *f)
{
	char cookie[4097], buf[1024], *p, *e;
	int fd, n, c, r, s;
	size_t l;

	fd = open("/mnt/webcookies/http", O_RDWR);
	if (fd < 0) {
		DBG("unable to open webcookies: %s", strerror(errno));
		return;
	}
	l = strlen(nsurl_access(f->url));
	if ((n = write(fd, nsurl_access(f->url), l)) != l) {
		DBG("unable to write url to webcookies: %s", strerror(errno));
		close(fd);
		return;
	}
	c = 0;
	r = 0;
	for (;;) {
		n = read(fd, buf + r, sizeof(buf) - r);
		if (n == 0)
			break;
		if (n < 0) {
			DBG("unable to read from webcookies: %s", strerror(errno));
			close(fd);
			break;
		}
		buf[n] = 0;
		p = buf;
		e = strchr(buf, '\n');
		s = n;
		if (c == 0) {
			p += 8; /* skip 'Cookie: ' */
			s -= 8;
		}
		if (e != NULL) {
			s = (int)(e - p+s) + 1;
			strncpy(cookie+c, p, s);
			c += s;
			cookie[c] = 0;
			//DBG("COOKIE [%s]", cookie);
			fetch_set_cookie(f->parent, cookie);
			c = 0;
			r = n-s;
			memmove(buf, e+1, sizeof(buf) - n);
		} else {
			DBG("PARSING [%s]", buf);
			strncpy(cookie+c, buf, n);
			c += n;
		}
	}
	close(fd);
}

static bool webfs_start(void *fetch)
{
	struct webfs_fetch_list *l;
	struct webfs_fetch *f;
	int n, cfd;
	char *e, *s, buf[5], path[PATH_MAX+1];
	fetch_msg msg;
	nserror err;

	f = fetch;
	f->state = Serror;
	cfd = open("/mnt/web/clone", O_RDWR);
	if (cfd <= 0) {
		DBG("unable to open /mnt/web/clone: %s", strerror(errno));
		f->state = Serror;
		msg.type = FETCH_ERROR;
		msg.data.error = e;
		fetch_send_callback(&msg, f->parent);		
		return true;
	}
	if ((n = read(cfd, buf, sizeof(buf)-1)) <= 0) {
		DBG("could not get connection from webfs: %s", strerror(errno));
		goto Error;
	}
	buf[n-1] = 0; /* remove \n */
	f->id = strtol(buf, &s, 10);
	if (f->id == 0 && buf==s) {
		DBG("could not parse connection id: %s", strerror(errno));
		goto Error;
	}
	if (send_request(cfd, f) < 0) {
		goto Error;
	}
	if (snprintf(path, sizeof path, "/mnt/web/%d/body", f->id) <= 0) {
		goto Error;
	}
	f->fd = open(path, O_RDONLY);
	if (f->fd <= 0) {
		e = strerror(errno);
		handle_error(f, path, e);
		goto Error;
	}
	if (should_redirect(cfd, f) == true) {
		fetch_set_http_code(f->parent, 301);
		fetch_list_add(f);
		goto Error;
	}
	close(cfd);
	fetch_set_http_code(f->parent, 200);
	send_headers(f);
	/* read_cookies(f); */
	f->state = Sstarted;
	fetch_list_add(f);
	return true;
Error:
	f->fd = -1;
	close(cfd);
	return true;
}

static void webfs_abort(void *fetch)
{
	struct webfs_fetch *f;

	f = fetch;
	f->state = Saborted;
	close(f->fd);
	f->fd = -1;
}

static void webfs_free(void *fetch)
{
	struct webfs_fetch *f;
	int i;

	f = fetch;
	nsurl_unref(f->url);
	if (f->post_url != NULL) {
		free(f->post_url);
	}
	if (f->post_data != NULL) {
		fetch_multipart_data_destroy(f->post_data);
	}
	if (f->headers != NULL) {
		for (i = 0; f->headers[i] != NULL; i++) {
			free(f->headers[i]);
		}
		free(f->headers);
	}
	free(f);
}

static void webfs_read(struct webfs_fetch *f)
{
	char *buf;
	int s, r, n;
	char *e;
	fetch_msg msg;

	s = 16*1024;
	buf = calloc(s, sizeof(char));
	for(n = 0; n < s; n += r) {
		if ((r = read(f->fd, buf+n, s-n)) <= 0)
			break;
	}
	if (n < 0) {
		e = strerror(errno);
		DBG("webfs read error [%s]: %s", nsurl_access(f->url), e);
		f->state = Serror;
		close(f->fd);
		f->fd = -1;
		msg.type = FETCH_ERROR;
		msg.data.error = e;
		fetch_send_callback(&msg, f->parent);
	} else if (n == 0) {
		f->state = Sfinished;
		close(f->fd);
		f->fd = -1;
		msg.type = FETCH_FINISHED;
		fetch_send_callback(&msg, f->parent);
	} else {
		msg.type = FETCH_DATA;
		msg.data.header_or_data.buf = (const uint8_t*)buf;
		msg.data.header_or_data.len = n;
		fetch_send_callback(&msg, f->parent);
	}
}

static void webfs_poll(lwc_string *scheme)
{
	struct webfs_fetch_list *l, *d, *p;
	fetch_msg msg;
	uint64_t t;

	for (l = fetch_list; l != NULL; l = l->next) {
		if (l->fetch == NULL) {
			continue;
		}
		if (l->fetch->state == Sstarted) {
			webfs_read(l->fetch);
		} else if (l->fetch->state == Snotmodified) {
			msg.type = FETCH_NOTMODIFIED;
			fetch_send_callback(&msg, l->fetch->parent);
		} else if (l->fetch->state == Sredirect) {
			msg.type = FETCH_REDIRECT;
			msg.data.redirect = l->fetch->data;
			fetch_send_callback(&msg, l->fetch->parent);
		} else if (l->fetch->state == Serror) {
			msg.type = FETCH_ERROR;
			msg.data.redirect = l->fetch->data;
			fetch_send_callback(&msg, l->fetch->parent);
		}
		/* new state might have already changed here so we don't want to wait
		 * the next poll to react
		 * - webfs_read can return one of { Sstarted, Sfinished, Serror }
		 * - if state was Snotmodified or Sredirect, it is now Saborted */
		switch(l->fetch->state) {
		case Sstarted:
		case Snotmodified:
		case Sredirect:
			/* nothing to do */
			break;
		case Sfinished:
		case Saborted:
		case Serror:
			fetch_remove_from_queues(l->fetch->parent);
			fetch_free(l->fetch->parent);
			l->fetch = NULL;
			break;
		}
	}
	/* cleanup is done afterward as we cannot guarantee that 
	 * a start will not happen and modify the fetch list during the 
	 * main poll loop */
	p = NULL;
	for (l = fetch_list; l != NULL; ) {
		if (l->fetch == NULL) {
			d = l;
			l = l->next;
			if (p != NULL)
				p->next = l;
			else
				fetch_list = l;
			free(d);
		} else {
			p = l;
			l = l->next;
		}
	}
}

static bool webfs_initialise(lwc_string *scheme)
{
	++fetcher_init_count;
	return true;
}

static void webfs_finalise(lwc_string *scheme)
{
	struct webfs_fetch_list *l, *del;

	--fetcher_init_count;
	if (fetcher_init_count == 0) {
		l = fetch_list;
		while (l != NULL) {
			del = l;
			l = del->next;
			if (del->fetch != NULL)
			webfs_free(del->fetch);
			free(del);
		}
	}
}

nserror webfs_register(void)
{
	nserror err;
	lwc_string *scheme;
	const struct fetcher_operation_table fetcher_ops = {
		.initialise = webfs_initialise,
		.finalise = webfs_finalise,
		.acceptable = webfs_acceptable,
		.setup = webfs_setup,
		.start = webfs_start,
		.abort = webfs_abort,
		.free = webfs_free,
		.poll = webfs_poll,
	};

	scheme = lwc_string_ref(corestring_lwc_http);
	err = fetcher_add(scheme, &fetcher_ops);
	if (err != NSERROR_OK) {
		DBG("webfs_register: unable to register as HTTP fetcher (%s)", messages_get_errorcode(err));
		return err;
	}
	scheme = lwc_string_ref(corestring_lwc_https);
	err = fetcher_add(scheme, &fetcher_ops);
	if (err != NSERROR_OK) {
		DBG("webfs_register: unable to register as HTTPS fetcher (%s)", messages_get_errorcode(err));
		return err;
	}
	return NSERROR_OK;
}
