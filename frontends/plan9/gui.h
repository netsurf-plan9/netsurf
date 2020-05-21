#ifndef NETSURF_PLAN9_GUI_H
#define NETSURF_PLAN9_GUI_H

struct gui_window* gui_window_create(struct browser_window *bw);
void gui_window_destroy(struct gui_window *gw);
void gui_window_redraw(struct gui_window *gw, Rectangle clipr);
void gui_window_resize(struct gui_window *gw);

#endif
