#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2007-11-23
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
*****************************************************************************/

#include "module_analysis_procs.h"
#include "pyxis/utility/exceptions.h"

//! Generic analysis exception.
class MODULE_ANALYSIS_PROCS_DECL AnalysisException : public PYXException
{
public:
	//! Constructor
	AnalysisException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "An analysis exception occurred";
	}
};

//! Thrown when the feature rasterize cannot get a  feature collection from it's input.
class MODULE_ANALYSIS_PROCS_DECL NullFeatureCollectionException : 
	public AnalysisException
{
public:
	//! Constructor
	NullFeatureCollectionException(const std::string& strError) : 
	  AnalysisException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "Failed to get the feature collection.";
	}
};

#endif