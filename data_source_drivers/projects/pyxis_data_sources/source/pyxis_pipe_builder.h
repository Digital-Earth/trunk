#ifndef PYXIS_PIPE_BUILDER_H
#define PYXIS_PIPE_BUILDER_H
/******************************************************************************
pyxis_pipe_builder.h

begin		: 2007-08-09
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "module_pyxis_coverages.h"

// pyxlib includes
#include "pyxis/pipe/pipe_builder.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
Pyxis file pipeline builder. The first pipeline builder designed to handle 
the construction of ppl files into Pipelines, to be imported into the library.
*/
//! Builds ppl files into Pipelines.
class MODULE_PYXIS_COVERAGES_DECL PyxisPipeBuilder : public PipeBuilderBase
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeBuilder)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IPipeBuilder

	//! Build a pipeline for the data set.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

public:

	//! Test method.
	static void test();

	//! Default Constructor
	PyxisPipeBuilder();

	//! Destructor
	~PyxisPipeBuilder(){;}

private:

 	//! A mutex for thread safety
	mutable boost::recursive_mutex m_mutex;
};

#endif //guard