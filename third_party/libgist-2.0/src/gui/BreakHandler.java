// BreakHandler.java
// Copyright (c) 1998, Regents of the University of California
// $Header: /pyxis_dev/dev/third_party_code/libgist-2.0/src/gui/BreakHandler.java,v 1.1 2004/12/15 21:33:42 Patrick Dooley Exp $

/*
 * BreakHandler:
 *
 * Breakpoint handler routine. Return value indicates to libgist how to carry on.
 */

interface BreakHandler {

int CONTINUE = 0;
int STEP = 1; // stop at next break-worthy point in execution
int NEXT = 2; // stop at next entry routine (insert, fetch, remove)
int CANCEL = 3; // don't execute the entry routine that was just started

public int
breakHandler(BreakInfo info);

}
