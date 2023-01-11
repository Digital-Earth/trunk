// AnalysisInfo.java
// Copyright (c) 1998, Regents of the University of California
// $Id: AnalysisInfo.java,v 1.1 2004/12/15 21:33:41 Patrick Dooley Exp $

import java.lang.String;

///////////////////////////////////////////////////////////////////////////////
// AnalysisInfo - info about state of analysis
//
// Description:
//	- combines parts of amdb_wkldprofile.statsTotals and 
//	  parts of amdb_analysis
//	- avoided subclasses here, because these fields need to be set in C,
//	  which would have made it too cumbersome
///////////////////////////////////////////////////////////////////////////////

public class AnalysisInfo
{

// amdb_analysis.actualInfo
String actualName;
boolean actualHasWkldStats;
boolean actualHasSplitStats;
boolean actualHasPenaltyStats;

// amdb_analysis.optInfo
String optName;
boolean optHasWkldStats;
boolean optHasSplitStats;
boolean optHasPenaltyStats;

// amdb_analysis.parameters
int randomRuns;
float targetUtil;

String queryFile; // amdb_analysis.queryFile
int numQueries; // amdb_wkldprofile.queries.size()
int numRetrieved; // amdb_wkldprofile.statsTotals.dataCoverage
int numItems; // amdb_treemap.itemMap.size()

}
