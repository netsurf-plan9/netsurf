#ifndef DRAWUI_WINDOW_H
#define DRAWUI_WINDOW_H

#include <draw.h>
#include <event.h>

typedef void(*mouse_callback)(Mouse, void*);
typedef void(*keyboard_callback)(int, void*);

struct dwindow;

struct    dwindow *dwindow_create(Rectangle r);
void      dwindow_destroy(struct dwindow *window);
void      dwindow_resize(struct dwindow *window, Rectangle r);
Rectangle dwindow_get_view_rect(struct dwindow *window);
Rectangle dwindow_rect_in_view_rect(struct dwindow *window, Rectangle r);
Point     dwindow_point_in_view_rect(struct dwindow *window, Point p);
void	  dwindow_set_icon(struct dwindow *window, Image *icon);
void      dwindow_set_title(struct dwindow *window, const char *text);
void      dwindow_set_url(struct dwindow *window, const char *text);
void      dwindow_set_status(struct dwindow *window, const char *text);
void      dwindow_set_extents(struct dwindow *window, int x, int y);
int       dwindow_try_scroll(struct dwindow *window, int sx, int sy);
int       dwindow_get_scroll_x(struct dwindow *window);
int       dwindow_get_scroll_y(struct dwindow *window);
int       dwindow_get_extent_y(struct dwindow *window);
void      dwindow_set_scroll(struct dwindow *window, int sx, int sy);
void      dwindow_set_back_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_forward_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_stop_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_reload_button_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_entry_activated_callback(struct dwindow *window, void(*cb)(char*, void*), void *data);
void      dwindow_set_scrollbar_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_browser_mouse_callback(struct dwindow *window, mouse_callback cb, void *data);
void      dwindow_set_browser_keyboard_callback(struct dwindow *window, keyboard_callback cb, void *data);
void      dwindow_draw(struct dwindow *window);
void      dwindow_mouse_event(struct dwindow *window, Event e);
void      dwindow_keyboard_event(struct dwindow *window, Event e);
void	  dwindow_focus_url_bar(struct dwindow *dwindow);

#endif
