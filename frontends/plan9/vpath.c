#include <stdlib.h>
#include <draw.h>
#include "vpath.h"

#define PINC 32

static void appendpt(struct vpath *l, Point p)
{
	if (l->np == 0) {
		l->p = malloc(PINC*sizeof(Point));
	} else if (l->np%PINC == 0) {
		l->p = realloc(l->p, (l->np+PINC)*sizeof(Point));
	}
	l->p[l->np++] = p;	
}

void vpath_move_to(struct vpath *vp, Point p)
{
	appendpt(vp, p);
}

void vpath_line_to(struct vpath *vp, Point p)
{
	appendpt(vp, p);
}

static int normsq(Point p)
{
	return p.x*p.x+p.y*p.y;
}

static int psdist(Point p, Point a, Point b)
{
	int num, den;

	p = subpt(p, a);
	b = subpt(b, a);
	num = p.x*b.x + p.y*b.y;
	if(num <= 0)
		return normsq(p);
	den = normsq(b);
	if(num >= den)
		return normsq(subpt(b, p));
	return normsq(subpt(divpt(mulpt(b, num), den), p));
}

static void
bpts1(struct vpath *l, Point p0, Point p1, Point p2, Point p3, int scale)
{
	Point p01, p12, p23, p012, p123, p0123;
	Point tp0, tp1, tp2, tp3;
	tp0=divpt(p0, scale);
	tp1=divpt(p1, scale);
	tp2=divpt(p2, scale);
	tp3=divpt(p3, scale);
	if(psdist(tp1, tp0, tp3)<=1 && psdist(tp2, tp0, tp3)<=1){
		appendpt(l, tp0);
		appendpt(l, tp1);
		appendpt(l, tp2);
	}
	else{
		/*
		 * if scale factor is getting too big for comfort,
		 * rescale now & concede the rounding error
		 */
		if(scale>(1<<12)){
			p0=tp0;
			p1=tp1;
			p2=tp2;
			p3=tp3;
			scale=1;
		}
		p01=addpt(p0, p1);
		p12=addpt(p1, p2);
		p23=addpt(p2, p3);
		p012=addpt(p01, p12);
		p123=addpt(p12, p23);
		p0123=addpt(p012, p123);
		bpts1(l, mulpt(p0, 8), mulpt(p01, 4), mulpt(p012, 2), p0123, scale*8);
		bpts1(l, p0123, mulpt(p123, 2), mulpt(p23, 4), mulpt(p3, 8), scale*8);
	}
}

void vpath_bezier(struct vpath *vp, Point p1, Point p2, Point p3)
{
	Point p0;

	p0 = vp->p[vp->np-1];
	bpts1(vp, p0, p1, p2, p3, 1);
	appendpt(vp, p3);
}

void vpath_close(struct vpath *vp)
{
	appendpt(vp, vp->p[0]);
}
