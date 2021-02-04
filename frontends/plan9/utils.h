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

/*
 * send the given text through the plumber
 * send channel
 */
int send_to_plumber(const char *text);

/*
 * send a plumb message to the send channel
 * using action=showdata
 */
int send_data_to_plumber(char *dst, char *filename, char *data, int ndata);

/*
 * spawn a new netsurf process for the given url
 */
void exec_netsurf(const char *url);

void DBG(const char *format, ...);

#endif
