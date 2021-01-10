#ifndef NETSURF_PLAN9_UTILS_H
#define NETSURF_PLAN9_UTILS_H

extern bool log_debug;

/* 
 * returns the full path of _filename_ 
 * in user configuration directory
 */
char *userdir_file(char *filename);

/*
 * read file and return its content
 * size is set to the content length
 */
char *read_file(char *path, int *size);

int send_to_plumber(const char *text);

void exec_netsurf(const char *cmd, const char *url);

void DBG(const char *format, ...);

#endif
