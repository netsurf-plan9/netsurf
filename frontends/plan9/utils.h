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
 * get the full rooted path of _filename_
 */
char* file_fullpath(char *filename);

/*
 * returns the file extension of _filename_
 */
char* file_ext(const char *filename);

/*
 * returns the mime-type of _path_ based
 * on its file extension
 */
const char* file_type(const char *path);

/*
 * check whether page(1) can open the given
 * mime-type
 */
bool page_accept_mimetype(char *mime);

/*
 * check whether _filename_ can be opened 
 * with page
 */
bool page_accept_file(char *filename);

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
