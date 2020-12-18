#ifndef NETSURF_PLAN9_CLIPBOARD_H
#define NETSURF_PLAN9_CLIPBOARD_H

void plan9_paste(char **buffer, size_t *length);
void plan9_snarf(const char *buffer, size_t length);

extern struct gui_clipboard_table *plan9_clipboard_table;

#endif
