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
#include "plan9/gui.h"
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
	struct bitmap *bitmap;

	bitmap = calloc(1, sizeof(struct bitmap));
	if (bitmap == NULL) {
		return NULL;
	}
	bitmap->data = calloc(BITMAP_BPP * width * height, sizeof (unsigned char));
	if (bitmap->data == NULL) {
		return NULL;
	}
	bitmap->i = NULL;
	bitmap->width = width;
	bitmap->height = height;
	bitmap->opaque = (state & BITMAP_OPAQUE) ? 1 : 0;
	bitmap->modified = 0;
	return bitmap;
}

/**
 * Destroy a bitmap.
 *
 * \param bitmap The bitmap to destroy.
 */
void
bitmap_destroy(void *bitmap)
{
	struct bitmap *b = bitmap;

	if (b->i != NULL) {
		freeimage(b->i);
		b->i = NULL;
	}
	free(b->data);
	free(b);
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
	struct bitmap *b = bitmap;

	b->opaque = opaque;
	if (b->i != NULL) {
		freeimage(b->i);
		b->i = NULL;
	}
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
	struct bitmap *b = bitmap;

	return b->opaque;
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
	struct bitmap *b = bitmap;
	int i, c;

	c = ((b->width * 32 + 7) / 8) * b->height;
	for (i = 3; i < c; i += 4) {
		if (b->data[i] != 0xFF) {
			return false;
		}
	}
	return true;
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
	struct bitmap *b = bitmap;

	return b->data;
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
	struct bitmap *b = bitmap;
	int depth = 8 * BITMAP_BPP;

	return (b->width * depth + 8 - 1) / 8;
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
	struct bitmap *b = bitmap;

	return b->width;
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
	struct bitmap *b = bitmap;

	return b->height;
}

/**
 * The the *bytes* per pixel.
 *
 * \param bitmap The bitmap
 */
size_t
bitmap_get_bpp(void *bitmap)
{
	return BITMAP_BPP;
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
	struct bitmap *bm = bitmap;

	bm->modified = 1;
	if (bm->i != NULL) {
		freeimage(bm->i);
		bm->i = NULL;
	}
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
/* XXX: what is this used for ? */
	Image *i;
	int w, h;
	uint8_t *data;
	size_t sz;
	struct redraw_context ctx = {
		.interactive = false,
		.background_images = true,
		.plot = plan9_plotter_table,
	};
	w = content_get_width(content);
	h = content_get_height(content);
	i = getimage(bitmap, w, h);
	if (i == NULL) {
		return NSERROR_INVALID;
	}
	ctx.priv = i;
	content_scaled_redraw(content, w, h, &ctx);
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
