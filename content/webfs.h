#ifndef NETSURF_CONTENT_WEBFS_H_
#define NETSURF_CONTENT_WEBFS_H_

typedef struct webfs_handle webfs_handle;

nsurl *webfs_handle_get_url(const webfs_handle *wh);
nserror webfs_handle_change_callback(webfs_handle *wh,
		llcache_handle_callback cb, void *pw);
nserror webfs_handle_abort(webfs_handle *wh);
const uint8_t *webfs_handle_get_source_data(const webfs_handle *wh,
		size_t *size);
nserror webfs_handle_release(webfs_handle *wh);
nserror webfs_handle_invalidate_cache_data(webfs_handle *handle);
nserror webfs_handle_clone(llcache_handle *handle, webfs_handle *wh, webfs_handle **result);
const char *webfs_handle_get_header(const webfs_handle *wh,
		const char *key);
void webfs_clean(bool purge);
bool webfs_handle_references_same_object(const webfs_handle *a,
		const webfs_handle *b);
nserror webfs_handle_force_stream(webfs_handle *wh);
nserror webfs_initialise(const struct llcache_parameters *parameters);
void webfs_finalise(void);
nserror webfs_handle_retrieve(nsurl *url, uint32_t flags,
		nsurl *referer, const llcache_post_data *post,
		llcache_handle_callback cb, void *pw, llcache_handle *handle,
		webfs_handle **result);


#endif
