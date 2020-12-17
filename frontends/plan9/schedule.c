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
	Slist *prev;
};

static Slist *schedlist;

static nserror schedule_remove(void (*cb)(void*), void *p)
{
	Slist *s, *next;

	for(s = schedlist; s != NULL; s = next){
		next = s->next;
		if(s->cb==cb && s->p==p){
			if(next != NULL)
				next->prev = s->prev;
			if(s->prev != NULL)
				s->prev->next = next;
			else
				schedlist = next;
			free(s);
		}
	}
	return NSERROR_OK;
}

void schedule_run(void)
{
	Slist *s, *next, *old;

	old = schedlist;
	schedlist = NULL;

	for(s = old; s != NULL; s = next){
		next = s->next;
		if(s->t <= SCHEDULE_PERIOD){
			if(next != NULL)
				next->prev = s->prev;
			if(s->prev != NULL)
				s->prev->next = next;
			else
				old = next;
			s->cb(s->p);
			free(s);
		}else{
			s->t -= SCHEDULE_PERIOD;
		}
	}

	/* newly added tasks: put them into scheduler again */
	if (old != NULL) {
		for (s = schedlist; s != NULL && s->next != NULL; s = s->next);
		if (s != NULL) {
			s->next = old;
			old->prev = s;
		} else {
			schedlist = old;
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

	err = schedule_remove(cb, p);
	if(t < 0)
		return err;
	s = calloc(1, sizeof(Slist));
	s->cb = cb;
	s->p  = p;
	s->t  = t;
	s->next = schedlist;
	if(schedlist != NULL)
		schedlist->prev = s;
	schedlist = s;
	return NSERROR_OK;
}

static struct gui_misc_table misc_table = {
	.schedule = misc_schedule,
};

struct gui_misc_table *plan9_misc_table = &misc_table;
