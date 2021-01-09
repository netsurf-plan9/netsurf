#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <draw.h>
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "utils/sys_time.h"
#include "netsurf/misc.h"
#include "plan9/schedule.h"
#include "plan9/utils.h"

typedef struct Slist Slist;

struct Slist
{
	void (*cb)(void*);
	void *p;
	struct timeval tv;
	Slist *next;
};

static Slist *schedlist;

static nserror schedule_remove(void (*cb)(void*), void *p)
{
	Slist *cur, *prev, *del;

	if (schedlist == NULL) {
		return NSERROR_OK;
	}
	cur = schedlist;
	prev = NULL;
	while (cur != NULL) {
		if ((cur->cb == cb) && (cur->p == p)) {
			del = cur;
			cur = del->next;
			if (prev == NULL) {
				schedlist = cur;
			} else {
				prev->next = cur;
			}
			free(del);
		} else {
			prev = cur;
			cur = prev->next;
		}
	}
	return NSERROR_OK;
}

int schedule_run(void)
{
	Slist *cur, *prev, *del;
	struct timeval tv, nexttime, rettime;

	if (schedlist == NULL)
		return -1;
	cur = schedlist;
	prev = NULL;
	nexttime = cur->tv;
	gettimeofday(&tv, NULL);
	while (cur != NULL) {
		if (timercmp(&tv, &cur->tv, >)) {
			del = cur;
			if (prev == NULL) {
				schedlist = del->next;
			} else {
				prev->next = del->next;
			}
			del->cb(del->p);
			free(del);
			if (schedlist == NULL) /* no more callbacks */
				return -1;
			cur = schedlist;
			prev = NULL;
			nexttime = cur->tv;
		} else {
			if (timercmp(&nexttime, &cur->tv, >)) {
				nexttime = cur->tv;
			}
			prev = cur;
			cur = prev->next;
		}
	}
	timersub(&nexttime, &tv, &rettime);
	return (rettime.tv_sec*1000 + rettime.tv_usec/1000);
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
	struct timeval tv;

	err = schedule_remove(cb, p);
	if(t < 0)
		return err;
	tv.tv_sec  = t/1000;
	tv.tv_usec = (t%1000)*1000;
	s = calloc(1, sizeof(Slist));
	s->cb = cb;
	s->p  = p;
	gettimeofday(&s->tv, NULL);
	timeradd(&s->tv, &tv, &s->tv);
	s->next = schedlist;
	schedlist = s;
	return NSERROR_OK;
}
