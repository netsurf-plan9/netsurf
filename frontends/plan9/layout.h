#ifndef NETSURF_PLAN9_LAYOUT_H
#define NETSURF_PLAN9_LAYOUT_H

/**
 * \brief Convert a netsurf font description to a Plan 9 font
 *
 * \param fstyle The font style description
 * \return A matching font on success or the default system font
 */
Font* getfont(const struct plot_font_style *fstyle);

extern struct gui_layout_table *plan9_layout_table;

#endif
