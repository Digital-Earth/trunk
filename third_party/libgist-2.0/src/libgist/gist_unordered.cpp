// gist_unordered.cc
// Copyright (c) 1997, Regents of the University of California
// $Id: gist_unordered.cpp,v 1.1 2004/12/15 21:33:44 Patrick Dooley Exp $

#include <assert.h>
#include "gist_p.h"

#ifdef __GNUG__
#pragma implementation "gist_unordered.h"
#endif
#include "gist_unordered.h"

void
gist_predcursor_t::add(
    const void*		data,
    int 		len)
{
    assert(numElems < maxElems);
    elems[numElems].key = data;
    elems[numElems].keyLen = len;
    numElems++;
}

void
gist_predcursor_t::add(
    const gist_p&	page)
{
    int cnt = page.nrecs();
    assert(numElems + cnt <= maxElems);
    for (int i = 0; i < cnt; i++) {
        elems[numElems].key = (void *) page.rec(i).key();
        elems[numElems].keyLen = page.rec(i).klen();
	numElems++;
    }
}

gist_predcursor_t::~gist_predcursor_t()
{
}

gist_predcursor_t::gist_predcursor_t() : numElems(0)
{
}

