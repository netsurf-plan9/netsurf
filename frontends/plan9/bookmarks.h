#ifndef NETSURF_PLAN9_BOOKMARKS_H
#define NETSURF_PLAN9_BOOKMARKS_H

void bookmarks_init(void);
void bookmarks_add(const char*, struct browser_window*);
void bookmarks_show(struct browser_window*);

#endif
