#include <stdio.h>
#include <stdlib.h>
#include <draw.h>
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "netsurf/misc.h"
#include "plan9/schedule.h"
#include "plan9/utils.h"

typedef struct Slist Slist;

struct Slist
{
	void (*cb)(void*);
	void *p;
	int t;
	Slist *next;
};

static Slist* schedlist = NULL;

static nserror schedule_remove(void (*cb)(void*), void *p)
{
	Slist *cur, *prev, *tmp;

	prev = NULL;
	cur = schedlist;
	while(cur != NULL){
		if(cur->cb==cb && cur->p==p){
//			DBG("schedlist: removing scheduled callback %p (p=%p)", cb, p);
			tmp = cur;
			cur = cur->next;
			if(prev == NULL)
				schedlist = cur;
			else
				prev->next = cur;
			free(tmp);
		}else{
			prev = cur;
			cur = cur->next;
		}
	}
	return NSERROR_OK;
}

void schedule_run(void)
{
	static int nrun = 0;
	Slist *cur, *prev, *tmp;

	prev = NULL;
	cur = schedlist;
	while(cur != NULL){
//DBG("schedrun: callback %p (p=%p) planned in %d", cur->cb, cur->p, cur->t);
		cur->t -= SCHEDULE_PERIOD;
		if(cur->t <= 0){
			tmp = cur;
			if(prev == NULL)
				schedlist = cur->next;
			else
				prev->next = cur->next;
//DBG("schedrun: running callback %p (p=%p)", tmp->cb, tmp->p);
			tmp->cb(tmp->p);
			free(tmp);
			if(schedlist == NULL)
				return;
			cur = schedlist;
			prev = NULL;
		}else{
			prev = cur;
			cur = cur->next;
		}
	}
}

/**
 * Schedule a callback.
 *
 * \param t interval before the callback should be made in ms or
 *          negative value to remove any existing callback.
 * \param callback callback function
 * \param p user parameter passed to callback function
 * \return NSERROR_OK on sucess or appropriate error on faliure
 *
 * The callback function will be called as soon as possible
 * after the timeout has elapsed.
 *
 * Additional calls with the same callback and user parameter will
 * reset the callback time to the newly specified value.
 *
 */
nserror misc_schedule(int t, void cb(void *p), void *p)
{
	Slist *s;
	nserror err;
//	DBG("IN misc_schedule - t=%d cb=%p p=%p", t, cb, p);

	err = schedule_remove(cb, p);
	if(t<0)
		return err;
	s = calloc(1, sizeof(Slist));
	s->cb = cb;
	s->p  = p;
	s->t  = t;
	s->next = schedlist;
	schedlist = s;
//	DBG("OUT misc_schedule");
	return NSERROR_OK;
}

static struct gui_misc_table misc_table = {
	.schedule = misc_schedule,
};

struct gui_misc_table *plan9_misc_table = &misc_table;
