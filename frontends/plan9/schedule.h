#ifndef NETSURF_PLAN9_SCHEDULE_H
#define NETSURF_PLAN9_SCHEDULE_H

int schedule_run(void);
nserror misc_schedule(int, void(*)(void*), void*);

#endif
