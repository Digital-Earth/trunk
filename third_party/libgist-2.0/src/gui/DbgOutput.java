// DbgOutput.java
// Copyright (c) 1998, Regents of the University of California
// $Id: DbgOutput.java,v 1.1 2004/12/15 21:33:42 Patrick Dooley Exp $

import java.io.PrintStream;

public class DbgOutput
{

private static PrintStream _out;
private static int _dbgLevel = 0; // no debugging output is the default

static void
setDbgLevel(int level)
{
    _dbgLevel = level;
}


DbgOutput()
{
}

static void
println(int level, String str)
{
    if (level <= _dbgLevel) {
        _out.println(str);
    }
}

static void
toStdout()
{
    _out = System.out;
}




}
