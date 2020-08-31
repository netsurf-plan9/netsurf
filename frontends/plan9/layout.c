#include <draw.h>
#include <ctype.h>
#include "utils/nsoption.h"
#include "netsurf/inttypes.h"
#include "netsurf/layout.h"
#include "netsurf/plot_style.h"
#include "utils/utf8.h"
#include "plan9/layout.h"
#include "plan9/utils.h"

/* from mothra */
struct font_definitions {
	char *name;
	Font *font;
} fontlist[4][4] = {
	"lucidasans/unicode.7", 0,
	"lucidasans/unicode.8", 0,
	"lucidasans/unicode.10", 0,
	"lucidasans/unicode.13", 0,

	"lucidasans/italicunicode.7", 0,
	"lucidasans/italicunicode.8", 0,
	"lucidasans/italicunicode.10", 0,
	"lucidasans/italicunicode.13", 0,

	"lucidasans/boldunicode.7", 0,
	"lucidasans/boldunicode.8", 0,
	"lucidasans/boldunicode.10", 0,
	"lucidasans/boldunicode.13", 0,

	"lucidasans/typeunicode.7", 0,
	"pelm/unicode.8", 0,
	"lucidasans/typeunicode.12", 0,
	"lucidasans/typeunicode.16", 0,
};

enum
{
	FONT_SIZE_SMALL = 0,
	FONT_SIZE_MEDIUM,
	FONT_SIZE_LARGE,
	FONT_SIZE_HUGE,
};

enum
{
	FONT_NORMAL = 0,
	FONT_ITALIC,
	FONT_BOLD,
	FONT_MONO
};

/**
 * \brief Convert a netsurf font description to a Plan 9 font
 *
 * \param fstyle The font style description
 * \return A matching font on success or the default system font
 */
Font* getfont(const plot_font_style_t *fstyle)
{
	int s, t, size;
	char buf[255];

	size = fstyle->size / PLOT_STYLE_SCALE;
	if(size <= 10)
		s = FONT_SIZE_SMALL;
	else if(size <= 12)
		s = FONT_SIZE_MEDIUM;
	else if(size <= 14)
		s = FONT_SIZE_LARGE;
	else
		s = FONT_SIZE_HUGE;

	if(fstyle->family == PLOT_FONT_FAMILY_MONOSPACE)
		t = FONT_MONO;
	if(fstyle->weight >= 500)
		t = FONT_BOLD;
	else if(fstyle->flags == FONTF_ITALIC || fstyle->flags == FONTF_OBLIQUE)
		t = FONT_ITALIC;
	else
		t = FONT_NORMAL;

	if(fontlist[t][s].font == 0) {
		snprintf(buf, sizeof buf, "/lib/font/bit/%s.font", fontlist[t][s].name);
		fontlist[t][s].font = openfont(display, buf);
		if(fontlist[t][s].font==0)
			fontlist[t][s].font=font;
	}
	return fontlist[t][s].font;
}

/**
 * Measure the width of a string.
 *
 * \param[in] fstyle plot style for this text
 * \param[in] string UTF-8 string to measure
 * \param[in] length length of string, in bytes
 * \param[out] width updated to width of string[0..length)
 * \return NSERROR_OK and width updated or appropriate error
 *          code on faliure
 */
nserror
layout_width(const struct plot_font_style *fstyle, const char *s, size_t length, int *width)
{
	Font *f;
        uint32_t ucs4;
        size_t nxtchr = 0;

	f = getfont(fstyle);
	*width = 0;
        while (nxtchr < length) {
                ucs4 = utf8_to_ucs4(s + nxtchr, length - nxtchr);
                nxtchr = utf8_next(s, length, nxtchr);

                *width += runestringnwidth(f, &ucs4, 1);
        }
	return NSERROR_OK;
}


/**
 * Find the position in a string where an x coordinate falls.
 *
 * \param[in] fstyle style for this text
 * \param[in] string UTF-8 string to measure
 * \param[in] length length of string, in bytes
 * \param[in] x coordinate to search for
 * \param[out] char_offset updated to offset in string of actual_x, [0..length]
 * \param[out] actual_x updated to x coordinate of character closest to x
 * \return NSERROR_OK and char_offset and actual_x updated or appropriate error code on faliure
 */
nserror 
layout_position(const struct plot_font_style *fstyle, const char *s, size_t length, int x, size_t *char_offset, int *actual_x)
{
	Font *f;
        uint32_t ucs4;
        size_t nxtchr = 0;
        int prev_x = 0;

	f = getfont(fstyle);
        *actual_x = 0;
        while (nxtchr < length) {
                ucs4 = utf8_to_ucs4(s + nxtchr, length - nxtchr);

                *actual_x += runestringnwidth(f, &ucs4, 1);
                if (*actual_x > x)
                        break;

                prev_x = *actual_x;
                nxtchr = utf8_next(s, length, nxtchr);
        }

        /* choose nearest of previous and last x */
        if (abs(*actual_x - x) > abs(prev_x - x))
                *actual_x = prev_x;

        *char_offset = nxtchr;
	return NSERROR_OK;}


/**
 * Find where to split a string to make it fit a width.
 *
 * \param[in] fstyle       style for this text
 * \param[in] string       UTF-8 string to measure
 * \param[in] length       length of string, in bytes
 * \param[in] x            width available
 * \param[out] char_offset updated to offset in string of actual_x, [1..length]
 * \param[out] actual_x updated to x coordinate of character closest to x
 * \return NSERROR_OK or appropriate error code on faliure
 *
 * On exit, char_offset indicates first character after split point.
 *
 * \note char_offset of 0 must never be returned.
 *
 *   Returns:
 *     char_offset giving split point closest to x, where actual_x <= x
 *   else
 *     char_offset giving split point closest to x, where actual_x > x
 *
 * Returning char_offset == length means no split possible
 */
nserror
layout_split(const struct plot_font_style *fstyle, const char *s, size_t length, int x, size_t *char_offset, int *actual_x)
{
	Font *f;
	uint32_t ucs4;
	size_t nxtchr = 0;
	int last_space_x = 0;
	int last_space_idx = 0;

	f = getfont(fstyle);
	*actual_x = 0;
	while (nxtchr < length) {
		ucs4 = utf8_to_ucs4(s + nxtchr, length - nxtchr);
		
		if (ucs4 == ' ' || ucs4 == '\t') {
			last_space_x = *actual_x;
			last_space_idx = nxtchr;
		}

		*actual_x += runestringnwidth(f, &ucs4, 1);
		if (*actual_x > x && last_space_idx != 0) {
			*actual_x = last_space_x;
			*char_offset = last_space_idx;
			return NSERROR_OK;
		}
		
		nxtchr = utf8_next(s, length, nxtchr);
	}
	*char_offset = length;
	return NSERROR_OK;
}

static struct gui_layout_table layout_table = {
	.width = layout_width,
	.position = layout_position,
	.split = layout_split,
};

struct gui_layout_table *plan9_layout_table = &layout_table;
