// gist_query.h							-*- c++ -*-
// Copyright (c) 1998, Regents of the University of California
// $Id: gist_query.h,v 1.1 2004/12/15 21:30:20 Patrick Dooley Exp $

#ifndef GIST_QUERY_H
#define GIST_QUERY_H

#ifdef __GNUG__
//#pragma interface "gist_query.h"
#endif

/*
 * This is the root class of all query classes. The only reason for
 * its existence is that we need a virtual destructor for amdb.
 */

class gist_query_t {
public:

    gist_query_t();
    virtual ~gist_query_t();
};

#endif // GIST_QUERY_H

