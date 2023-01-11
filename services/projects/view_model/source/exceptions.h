#pragma once
#ifndef VIEW_MODEL__EXCEPTIONS_H
#define VIEW_MODEL__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-10-20
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// pyxlib includes
#include "pyxis/utility/exception.h"

// standard includes
#include <string>

/*
Include all the exceptions for the visualization module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/

//! Thrown for any visualization error
class VIEW_MODEL_API PYXVisualizationException : public PYXException
{
public:
	//! Constructor
	PYXVisualizationException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A visualization error occurred.";}	
};

//! Thrown errors related to visualizing symbols.
class VIEW_MODEL_API PYXSymbolException : public PYXVisualizationException
{
public:
	//! Constructor
	PYXSymbolException(const std::string& strError) : 
	  PYXVisualizationException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A visualization symbol error occurred.";}	
};

//! Thrown errors related to managing pipelines of data
class VIEW_MODEL_API PYXVisualPipelineException : public PYXVisualizationException
{
public:
	//! Constructor
	PYXVisualPipelineException(const std::string& strError) : 
	  PYXVisualizationException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error in visual pipeline management occurred.";}	
};

//! Thrown errors related to visualizing coverages
class VIEW_MODEL_API NotVisualProcess: public PYXVisualPipelineException
{
public:
	//! Constructor
	NotVisualProcess(const std::string& strError) : 
	  PYXVisualPipelineException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Expected a coverage or feature collection for visualization pipeline";}	
};


#endif
