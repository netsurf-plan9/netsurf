#ifndef NETSURF_PLAN9_UTILS_H
#define NETSURF_PLAN9_UTILS_H

int send_to_plumber(const char *text);

void exec_netsurf(const char *cmd, const char *url);

void DBG(const char *format, ...);

#endif
