#ifndef DRAWUI_DATA_H
#define DRAWUI_DATA_H

void data_init(void);

/* colors */
extern Image *bg_color;
extern Image *fg_color;
extern Image *scroll_bg_color;
extern Image *focus_color;
extern Image *back_button_focus_color;
extern Image *forward_button_focus_color;
extern Image *stop_button_focus_color;
extern Image *reload_button_focus_color;

/* icons */
extern Image *back_button_icon;
extern Image *forward_button_icon;
extern Image *stop_button_icon;
extern Image *reload_button_icon;
extern Image *tick;

/* cursors */
extern Cursor linkcursor;
extern Cursor waitcursor;
extern Cursor caretcursor;
extern Cursor crosscursor;
extern Cursor helpcursor;
extern Cursor *cornercursors[9];

#endif
