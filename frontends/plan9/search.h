#ifndef NETSURF_PLAN9_SEARCH_H
#define NETSURF_PLAN9_SEARCH_H

void  search(struct gui_window*, const char*);
void  search_reset(struct gui_window*);
bool  search_has_next(void);
char* search_text(void);

#endif
