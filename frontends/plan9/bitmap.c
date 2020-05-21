#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <draw.h>
#include "utils/errors.h"
#include "netsurf/bitmap.h"
#include "netsurf/content.h"
#include "netsurf/plotters.h"
#include "plan9/bitmap.h"
#include "plan9/plotter.h"
#include "plan9/utils.h"

/**
 * Create a new bitmap.
 *
 * \param width width of image in pixels
 * \param height width of image in pixels
 * \param state The state to create the bitmap in.
 * \return A bitmap structure or NULL on error.
 */
void*
bitmap_create(int width, int height, unsigned int state)
{
	Image *i;
/*
	DBG("IN bitmap_create - width=%d height=%d state=%d", width, height, state);
	i = allocimage(display, Rect(0, 0, width, height), screen->chan, 0, DNofill);
	DBG("OUT bitmap_create - bitmap=%p", i);
*/
	return NULL;
}

/**
 * Destroy a bitmap.
 *
 * \param bitmap The bitmap to destroy.
 */
void
bitmap_destroy(void *bitmap)
{
	DBG("IN bitmap_destroy");
//	freeimage(bitmap);
}

/**
 * Set the opacity of a bitmap.
 *
 * \param bitmap The bitmap to set opacity on.
 * \param opaque The bitmap opacity to set.
 */
void
bitmap_set_opaque(void *bitmap, bool opaque)
{
	DBG("IN bitmap_set_opaque");
}

/**
 * Get the opacity of a bitmap.
 *
 * \param bitmap The bitmap to examine.
 * \return The bitmap opacity.
 */
bool 
bitmap_get_opaque(void *bitmap)
{
	DBG("IN bitmap_get_opaque");
	return 0;
}

/**
 * Test if a bitmap is opaque.
 *
 * \param bitmap The bitmap to examine.
 * \return The bitmap opacity.
 */
bool
bitmap_test_opaque(void *bitmap)
{
	DBG("IN bitmap_test_opaque");
	return 1;
}

/**
 * Get the image buffer from a bitmap
 *
 * \param bitmap The bitmap to get the buffer from.
 * \return The image buffer or NULL if there is none.
 */
unsigned char*
bitmap_get_buffer(void *bitmap)
{
	DBG("IN bitmap_get_buffer");
	return NULL;
}

/**
 * Get the number of bytes per row of the image
 *
 * \param bitmap The bitmap
 * \return The number of bytes for a row of the bitmap.
 */
size_t
bitmap_get_rowstride(void *bitmap)
{
	Image *i;

	DBG("IN bitmap_get_rowstride");
	/*i = bitmap;
	return bytesperline(i->r, i->depth);*/
	return 0;
}

/**
 * Get the bitmap width
 *
 * \param bitmap The bitmap
 * \return The bitmap width in pixels.
 */
int
bitmap_get_width(void *bitmap)
{
	Image *i;

	DBG("IN bitmap_get_width");
	i = bitmap;
	return Dx(i->r);;
}

/**
 * Get the bitmap height
 *
 * \param bitmap The bitmap
 * \return The bitmap height in pixels.
 */
int
bitmap_get_height(void *bitmap)
{
	return 0;
/*
	Image *i;

	DBG("IN bitmap_get_height");
	i = bitmap;
	return Dy(i->r);*/
}

/**
 * The the *bytes* per pixel.
 *
 * \param bitmap The bitmap
 */
size_t
bitmap_get_bpp(void *bitmap)
{
	return 0;
/*
	Image *i;

	DBG("IN bitmap_get_bpp");
	i = bitmap;
	return i->depth/8;*/
}

/**
 * Savde a bitmap to disc.
 *
 * \param bitmap The bitmap to save
 * \param path The path to save the bitmap to.
 * \param flags Flags affecting the save.
 */
bool
bitmap_save(void *bitmap, const char *path, unsigned flags)
{
	DBG("IN bitmap_save");
	return 0;
}

/**
 * Marks a bitmap as modified.
 *
 * \param bitmap The bitmap set as modified.
 */
void
bitmap_modified(void *bitmap)
{
	DBG("IN bitmap_modified");
}

/**
 * Render content into a bitmap.
 *
 * \param bitmap The bitmap to render into.
 * \param content The content to render.
 */
nserror
bitmap_render(struct bitmap *bitmap, struct hlcache_handle *content)
{
/*
	Image *i;
	int w, h;
	uint8_t *data;
	size_t sz;
	struct redraw_context ctx = {
		.interactive = false,
		.background_images = true,
		.plot = plan9_plotter_table,
	};

	DBG("IN bitmap_render - bitmap=%p title=%s", bitmap, content_get_title(content));
	i = (Image*)bitmap;
	w = content_get_width(content);
	h = content_get_height(content);
	content_scaled_redraw(content, w, h, &ctx);
	data = content_get_source_data(content, &sz);
	if(data != NULL){
		loadimage(i, Rect(0, 0, w, h), (uchar*)data, sz);
		DBG("image loaded (%d bytes)", (int)sz);
	//	draw(screen, Rect(screen->r.min.x, screen->r.min.y, screen->r.min.x+w, screen->r.min.x+h), i, nil, ZP);
	}
*/
	return NSERROR_OK;
}

static struct gui_bitmap_table bitmap_table = {
	.create = bitmap_create,
	.destroy = bitmap_destroy,
	.set_opaque = bitmap_set_opaque,
	.get_opaque = bitmap_get_opaque,
	.test_opaque = bitmap_test_opaque,
	.get_buffer = bitmap_get_buffer,
	.get_rowstride = bitmap_get_rowstride,
	.get_width = bitmap_get_width,
	.get_height = bitmap_get_height,
	.get_bpp = bitmap_get_bpp,
	.save = bitmap_save,
	.modified = bitmap_modified,
	.render = bitmap_render,
};

struct gui_bitmap_table *plan9_bitmap_table = &bitmap_table;
