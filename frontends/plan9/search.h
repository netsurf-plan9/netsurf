#ifndef NETSURF_PLAN9_SEARCH_H
#define NETSURF_PLAN9_SEARCH_H

void  search(struct gui_window*, const char*);
void  search_reset(struct gui_window*);
bool  search_has_next(void);
bool  search_should_wrap(void);
char* search_text(void);

extern struct gui_search_table *plan9_search_table;

#endif
