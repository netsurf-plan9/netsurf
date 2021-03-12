#include <stdio.h>
#include <stdlib.h>
#include <draw.h>
#include <event.h>
#include "utils/errors.h"
#include "netsurf/plotters.h"
#include "plan9/window.h"
#include "plan9/plotter.h"
#include "plan9/vpath.h"
#include "plan9/bitmap.h"
#include "plan9/layout.h"
#include "plan9/utils.h"
#include "plan9/drawui/window.h"
#include "utils/utils.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_MALLOC(x,u) malloc(x)
#define STBIR_FREE(x,u) free(x)
#define STBIR_ASSERT(x) assert(x)
#include "stb_image_resize.h"

typedef struct Color Color;

struct Color {
	Image *i;
	uint32_t rgb;
};

static Color cache[64];
static Rectangle clipr;

/**
 * \brief Converts a netsurf colour type to a libdraw image
 *
 * \param c The color to convert
 * \return Image on success else NULL 
 */
static Image *getcolor(colour c)
{
	Image *m;
	int i, f;
	uint32_t n, r, g, b;

	r = c & 0xff;
	g = (c & 0xff00) >> 8;
	b = (c & 0xff0000) >> 16;
	n = (r << 24) | (g << 16) | (b << 8) | 0xff; 
	if (n == 0xffffffff)
		return display->white;
	if (n == 0x000000ff)
		return display->black;
	for(i = f = 0; i < sizeof(cache)/sizeof(cache[0]); i++){
		if(cache[i].i != NULL){
			if(cache[i].rgb == n)
				return cache[i].i;
		}else{
			f = i;
		}
	}
	if((m = allocimage(display, Rect(0,0,1,1), BGR24, 1, n)) == NULL)
		return display->black;
	freeimage(cache[f].i);
	cache[f].i = m;
	cache[f].rgb = n;

	return m;
}

/**
 * \brief Converts a bitmap to a libdraw image
 *
 * \param b The bitmap
 * \return Image on success else NULL
 */
Image* getimage(struct bitmap *b, int w, int h)
{
	Rectangle r;
	ulong chan;
	uint8_t *out;
	int iw, ih, c, i;
	double alpha;

	if (b == NULL)
		return NULL;
	iw = bitmap_get_width(b);
	ih = bitmap_get_height(b);
	if(w == 0)
		w = iw;
	if(h == 0)
		h = ih;
	if (b->i == NULL) {
		if (b->opaque){
			chan = XBGR32;
		}else{
			c = ((b->width * 32 + 8 - 1) / 8) * b->height;
			/* don't always trust the rest of the netsurf: test for opaque images */
			for (i = 0; i < c; i += 4)
				if(b->data[i+3] != 255)
					break;
			if(i >= c){ /* it's opaque, apparently */
				chan = XBGR32;
			}else{
				chan = ABGR32;
				/* premultiply alpha */
				c = ((b->width * 32 + 8 - 1) / 8) * b->height;
				for (i = 0; i < c; i += 4) {
					alpha = (double)b->data[i+3] / 255.0;
					b->data[i+0] = alpha * b->data[i+0];
					b->data[i+1] = alpha * b->data[i+1];
					b->data[i+2] = alpha * b->data[i+2];
				}
			}
		}
		if (iw != w || ih != h) {
			if((out = malloc(w*h*4)) == nil)
				sysfatal("memory");
			stbir_resize_uint8_generic(
				b->data, iw, ih, bitmap_get_rowstride(b),
				out, w, h, w*BITMAP_BPP,
				BITMAP_BPP, chan == ABGR32 ? 3 : -1, STBIR_FLAG_ALPHA_PREMULTIPLIED,
				STBIR_EDGE_CLAMP, STBIR_FILTER_MITCHELL, STBIR_COLORSPACE_LINEAR,
				NULL
			);
		} else {
			out = b->data;
		}
		r = Rect(0, 0, w, h);
		if ((b->i = allocimage(display, r, chan, 0, DNofill)) != NULL)
			loadimage(b->i, r, out, BITMAP_BPP * w * h);
		if(out != b->data)
			free(out);
	}
	return b->i;
}

/**
 * \brief Sets a clip rectangle for subsequent plot operations.
 *
 * \param ctx The current redraw context.
 * \param clip The rectangle to limit all subsequent plot
 *              operations within.
 * \return NSERROR_OK on success else error code.
 */
nserror 
plotter_clip(const struct redraw_context *ctx, const struct rect *clip)
{
	Image *b;
	Rectangle r;

	b = ctx->priv;
	if (b == NULL) {
		DBG("plotter_clip => B IS NULL !!!");
		return NSERROR_INVALID;
	}
	r = Rect(clip->x0, clip->y0, clip->x1, clip->y1);
	if (clip->x0 == clip->x1 && clip->x0 == 0)
		r = b->r;
	replclipr(b, 0, r);
	clipr = r;
	return NSERROR_OK;
}

/**
 * Plots an arc
 *
 * plot an arc segment around (x,y), anticlockwise from angle1
 *  to angle2. Angles are measured anticlockwise from
 *  horizontal, in degrees.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the arc plot.
 * \param x The x coordinate of the arc.
 * \param y The y coordinate of the arc.
 * \param radius The radius of the arc.
 * \param angle1 The start angle of the arc.
 * \param angle2 The finish angle of the arc.
 * \return NSERROR_OK on success else error code.
 */
nserror
plotter_arc(const struct redraw_context *ctx,
	const plot_style_t *pstyle,
	int x,
	int y,
	int radius,
	int angle1,
	int angle2)
{
	Image *b;
	Image *c;
	Point p;

	b = ctx->priv;
	if ((c = getcolor(pstyle->fill_colour)) == NULL)
		return NSERROR_NOMEM;
	p = Pt(x, y);
	if(pstyle->fill_type != PLOT_OP_TYPE_NONE)
		arc(b, p, radius, radius, 0, c, ZP, angle1, angle2);
	return NSERROR_OK;
}

/**
 * Plots a circle
 *
 * Plot a circle centered on (x,y), which is optionally filled.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the circle plot.
 * \param x The x coordinate of the circle.
 * \param y The y coordinate of the circle.
 * \param radius The radius of the circle.
 * \return NSERROR_OK on success else error code.
 */
nserror
plotter_disc(const struct redraw_context *ctx,
		const plot_style_t *pstyle,
		int x,
		int y,
		int radius)
{
	Image *b;
	Image *c;
	Point p;
	int t;

	b = ctx->priv;
	t = plot_style_fixed_to_int(pstyle->stroke_width / 2);
	p = Pt(x, y);
	if(pstyle->fill_type != PLOT_OP_TYPE_NONE){
		if((c = getcolor(pstyle->fill_colour)) == NULL)
			return NSERROR_NOMEM;
		fillellipse(b, p, radius, radius, c, ZP);
	}
	if(pstyle->stroke_type != PLOT_OP_TYPE_NONE){
		if((c = getcolor(pstyle->stroke_colour)) == NULL)
			return NSERROR_NOMEM;
		ellipse(b, p, radius, radius, t, c, ZP);
	}
	return NSERROR_OK;
}


/**
 * Plots a line
 *
 * plot a line from (x0,y0) to (x1,y1). Coordinates are at
 *  centre of line width/thickness.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the line plot.
 * \param line A rectangle defining the line to be drawn
 * \return NSERROR_OK on success else error code.
 */
nserror 
plotter_line(const struct redraw_context *ctx,
		const plot_style_t *pstyle,
		const struct rect *l)
{
	Image *b;
	Image *c;
	int t;
	Point p0, p1;

	b = ctx->priv;
	if ((c = getcolor(pstyle->stroke_colour)) == NULL)
		return NSERROR_NOMEM;
	t = plot_style_fixed_to_int(pstyle->stroke_width / 2);
	p0 = Pt(l->x0, l->y0);
	p1 = Pt(l->x1, l->y1);
	line(b, p0, p1, 0, 0, t, c, ZP);
	return NSERROR_OK;
}

/**
 * Plots a rectangle.
 *
 * The rectangle can be filled an outline or both controlled
 *  by the plot style The line can be solid, dotted or
 *  dashed. Top left corner at (x0,y0) and rectangle has given
 *  width and height.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the rectangle plot.
 * \param rect A rectangle defining the line to be drawn
 * \return NSERROR_OK on success else error code.
 */
nserror 
plotter_rectangle(const struct redraw_context *ctx,
		const plot_style_t *pstyle,
		const struct rect *rectangle)
{
	Image *b;
	Rectangle r;
	Image *c;
	int t;

	b = ctx->priv;
	t = plot_style_fixed_to_int(pstyle->stroke_width / 2);
	r = Rect(rectangle->x0, rectangle->y0, rectangle->x1, rectangle->y1);
	if (pstyle->fill_type != PLOT_OP_TYPE_NONE) {
		if ((c = getcolor(pstyle->fill_colour)) == NULL)
			return NSERROR_NOMEM;
		draw(b, r, c, nil, ZP);
	}
	if (pstyle->stroke_type != PLOT_OP_TYPE_NONE) {
		if ((c = getcolor(pstyle->stroke_colour)) == NULL)
			return NSERROR_NOMEM;
		border(b, r, t, c, ZP);
	}
	return NSERROR_OK;
}

/**
 * Plot a polygon
 *
 * Plots a filled polygon with straight lines between
 * points. The lines around the edge of the ploygon are not
 * plotted. The polygon is filled with the non-zero winding
 * rule.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the polygon plot.
 * \param p verticies of polygon
 * \param n number of verticies.
 * \return NSERROR_OK on success else error code.
 */
nserror
plotter_polygon(const struct redraw_context *ctx,
		const plot_style_t *pstyle,
		const int *p,
		unsigned int n)
{
	Image *b;
	Image *c;
	Point *pts;
	int np, i;

	b = ctx->priv;
	pts = calloc(n + 1, sizeof *p);
	if (pts == NULL) {
		return NSERROR_OK;
	}
	np = 1;
	pts[0] = Pt(p[0], p[1]);
	for(i = 1; i != n; i++) {
		pts[np++] = Pt(p[i * 2], p[i * 2 + 1]);
	}
	if ((c = getcolor(pstyle->fill_colour)) == NULL)
		return NSERROR_NOMEM;
	fillpoly(b, pts, np, 1, c, ZP);
	return NSERROR_OK;
}

static Point transformpt(float x, float y, float transform[6])
{
	Point p;

	p.x = x * transform[0] + y * transform[1] + transform[4];
	p.y = x * transform[2] + y * transform[3] + transform[5];
	return p;
}

/**
 * Plots a path.
 *
 * Path plot consisting of cubic Bezier curves. Line and fill colour is
 *  controlled by the plot style.
 *
 * \param ctx The current redraw context.
 * \param pstyle Style controlling the path plot.
 * \param p elements of path
 * \param n nunber of elements on path
 * \param transform A transform to apply to the path.
 * \return NSERROR_OK on success else error code.
 */
nserror plotter_path(
		const struct redraw_context *ctx,
		const plot_style_t *pstyle,
		const float *p,
		unsigned int n,
		const float transform[6])
{
	Image *b, *c;
	Point p0, p1, p2, p3;
	struct vpath vp;
	int i, w;

	if (n == 0) {
		return NSERROR_OK;
	}
	if (p[0] != PLOTTER_PATH_MOVE) {
		DBG("plotter_path: path not starting with a move");
		return NSERROR_INVALID;
	}
	vp.np = 0;
	vp.p = NULL;
	for(i = 0; i < n; ) {
		if (p[i] == PLOTTER_PATH_MOVE) {
			p0 = transformpt(p[i+1], p[i+2], transform);
			vpath_move_to(&vp, p0);
			i += 3;
		} else if (p[i] == PLOTTER_PATH_CLOSE) {
			vpath_close(&vp);
			i += 1;
		} else if (p[i] == PLOTTER_PATH_LINE) {
			p1 = transformpt(p[i+1], p[i+2], transform);
			vpath_line_to(&vp, p1);
			i += 3;
		} else if (p[i] == PLOTTER_PATH_BEZIER) {
			p1 = transformpt(p[i+1], p[i+2], transform);
			p2 = transformpt(p[i+3], p[i+4], transform);
			p3 = transformpt(p[i+5], p[i+6], transform);
			vpath_bezier(&vp, p1, p2, p3);
			i += 7;
		} else {
			return NSERROR_INVALID;
		}
	}
	b = ctx->priv;
	c = NULL;
	w = plot_style_fixed_to_int(pstyle->stroke_width / 2);
	if (pstyle->stroke_colour != NS_TRANSPARENT) {
		if ((c = getcolor(pstyle->stroke_colour)) == NULL)
			return NSERROR_NOMEM;
		poly(b, vp.p, vp.np, Enddisc, Enddisc, w, c, ZP);
	}
	if (pstyle->fill_colour != NS_TRANSPARENT) {
		if ((c = getcolor(pstyle->fill_colour)) == NULL)
			return NSERROR_NOMEM;
		fillpoly(b, vp.p, vp.np, 1, c, ZP);
	}
	return NSERROR_OK;
}

/**
 * Plot a bitmap
 *
 * Tiled plot of a bitmap image. (x,y) gives the top left
 * coordinate of an explicitly placed tile. From this tile the
 * image can repeat in all four directions -- up, down, left
 * and right -- to the extents given by the current clip
 * rectangle.
 *
 * The bitmap_flags say whether to tile in the x and y
 * directions. If not tiling in x or y directions, the single
 * image is plotted. The width and height give the dimensions
 * the image is to be scaled to.
 *
 * \param ctx The current redraw context.
 * \param bitmap The bitmap to plot
 * \param x The x coordinate to plot the bitmap
 * \param y The y coordiante to plot the bitmap
 * \param width The width of area to plot the bitmap into
 * \param height The height of area to plot the bitmap into
 * \param bg the background colour to alpha blend into
 * \param flags the flags controlling the type of plot operation
 * \return NSERROR_OK on success else error code.
 */
nserror
plotter_bitmap(const struct redraw_context *ctx,
		struct bitmap *bitmap,
		int x,
		int y,
		int width,
		int height,
		colour bg,
		bitmap_flags_t flags)
{
	Image *b;
	Rectangle r, cr;
	Point p;
	Image *i, *m;

	b = ctx->priv;
	if ((i = getimage(bitmap, width, height)) == NULL)
		return NSERROR_NOMEM;
	if (flags == BITMAPF_NONE) {
		r = rectaddpt(i->r, Pt(x, y));
		p = ZP;
	} else {
		r = clipr;
		cr = rectsubpt(clipr, clipr.min);
		p = Pt(clipr.min.x - x, clipr.min.y - y);
		replclipr(i, 1, rectaddpt(cr, p));
	}
	draw(b, r, i, 0, p);
	bitmap->i = i;
	return NSERROR_OK;
}


/**
 * Text plotting.
 *
 * \param ctx The current redraw context.
 * \param fstyle plot style for this text
 * \param x x coordinate
 * \param y y coordinate
 * \param text UTF-8 string to plot
 * \param length length of string, in bytes
 * \return NSERROR_OK on success else error code.
 */
nserror
plotter_text(const struct redraw_context *ctx,
		const plot_font_style_t *fstyle,
		int x,
		int y,
		const char *text,
		size_t length)
{
	Image *b;
	Point p;
	Image *c;
	Font *f;
        uint32_t r;
        size_t n;

	n = 0;
	b = ctx->priv;
	f = getfont(fstyle);
	p = Pt(x, y -f->ascent);
	if ((c = getcolor(fstyle->foreground)) == NULL)
		return NSERROR_NOMEM;
        while (n < length) {
                r = utf8_to_ucs4(text + n, length - n);
                n = utf8_next(text, length, n);
                p = runestringn(b, p, c, ZP, f, &r, 1);
        }

	return NSERROR_OK;
}

static struct plotter_table plotter_table =
{
	.clip = plotter_clip,
	.arc = plotter_arc,
	.disc = plotter_disc,
	.line = plotter_line,
	.rectangle = plotter_rectangle,
	.polygon = plotter_polygon,
	.path = plotter_path,
	.bitmap = plotter_bitmap,
	.text = plotter_text,
	.option_knockout = false
};

struct plotter_table *plan9_plotter_table = &plotter_table;
