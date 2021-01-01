#ifndef NETSURF_PLAN9_VPATH_H
#define NETSURF_PLAN9_VPATH_H

struct vpath
{
	Point *p;
	int np;
	int pos;
};

void vpath_move_to(struct vpath*, Point);
void vpath_line_to(struct vpath*, Point);
void vpath_bezier(struct vpath*, Point, Point, Point);
void vpath_close(struct vpath*);

#endif
