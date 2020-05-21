#ifndef NETSURF_PLAN9_SCHEDULE_H
#define NETSURF_PLAN9_SCHEDULE_H

enum 
{ 
	SCHEDULE_PERIOD = 50
};

void schedule_run(void);

extern struct gui_misc_table *plan9_misc_table;

#endif
