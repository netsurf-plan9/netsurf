#ifndef NETSURF_PLAN9_SCHEDULE_H
#define NETSURF_PLAN9_SCHEDULE_H

enum 
{ 
	SCHEDULE_PERIOD = 50,
};

void schedule_run(void);
nserror misc_schedule(int, void(*)(void*), void*);

#endif
