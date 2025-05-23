// PenaltyStats.java
// Copyright (c) 1998, Regents of the University of California
// $Id: PenaltyStats.java,v 1.1 2004/12/15 21:33:41 Patrick Dooley Exp $


///////////////////////////////////////////////////////////////////////////////
// PenaltyStats -
// 	Java-equivalent of amdb_splitstats::FullPenaltyStats
//
// Description:
///////////////////////////////////////////////////////////////////////////////

public class PenaltyStats
{

float actualAddIos;
float optAddIos;
float actualExcCov;
float excCovDelta;
float clusterLoss;
float excCovOh;
float excCovSuccess;
String key;

PenaltyStats()
{
    reset();
}

void
reset()
{

    actualAddIos = 0.0f;
    optAddIos = 0.0f;
    actualExcCov = 0.0f;
    excCovDelta = 0.0f;
    clusterLoss = 0.0f;
    excCovOh = 0.0f;
    excCovSuccess = 0.0f;
    key = null;

}

}
