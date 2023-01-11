// gist_rtpred_base.cc						-*- c++ -*-
// Copyright (c) 1998, Regents of the University of California
// $Id: gist_rtpred.cpp,v 1.1 2004/12/15 21:33:45 Patrick Dooley Exp $

#ifdef __GNUG__
#pragma implementation "gist_rtpred.h"
#endif

#include "gist_defs.h"		// for assert()
#include "gist_rtpred.h"

rt_pred::rt_pred(int dim, double* c, bool d)
{
    assert(dim >= 0);
    dimension = dim;
    coord = c;
    dealloc = d;
}

rt_pred::~rt_pred()
{
    clear();
}

void
rt_pred::clear()
{
    if (dealloc && coord != NULL) {
	delete [] coord;
    }
    coord = NULL;
    dealloc = false;
}

void
rt_pred::set(double* c, bool d)
{
    clear();
    coord = c;
    dealloc = d;
}

bool
rt_pred::isEqual(const rt_pred& p, int n) const
{
    if (dimension != p.dimension) {
	return(false);
    }
    for (int i = 0; i < n; ++i) {
	if (coord[i] != p.coord[i]) {
	    return(false);
	}
    }
    return(true);
}
