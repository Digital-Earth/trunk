#ifndef GRIB_PIPE_BUILDER_H
#define GRIB_PIPE_BUILDER_H
/******************************************************************************
grib_pipe_builder.h

begin		: 2007-09-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

//local includes
#include "grib.h"

// pyxlib includes
#include "pyxis/pipe/pipe_builder.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

/*!
Automatically creates pipelines for GRIB file formats. Files processed through this 
pipebuilder are read in through the GRIB file reader process, then passed off to a default
sampling algorithm as specified in the WorldView properties file, currently Nearest Neighbour.
The output of the Nearest Neighbour Sampler is passed into a default grey scale colourizer,
each channel in the grib file receives it's own pipeline. These pipelines are then returned
to the manager to be imported into the library.
*/
//! Builds pipelines for Grib files.
class GRIB_DECL GRIBPipeBuilder : public PipeBuilderBase
{
	PYXCOM_DECLARE_CLASS();

public: //IUNKnown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeBuilder)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: //GRIBPipeBuilder
	
	//! Constants
	static const std::string kstrProcDescription;
	static const std::string kstrColourizerDefaultMin;
	static const std::string kstrColourizerDefaultMax;

	//! Default Constructor
	GRIBPipeBuilder();
	
	//! Destructor.
	~GRIBPipeBuilder(){;}

public: //IPipeBuilder

	//! Build a pipeline for the data set
	boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

private:

	//! Utility method to builds an individual pipeline for the specified path.
	boost::intrusive_ptr<IProcess> buildAPipeline(
		const boost::filesystem::path& path, int nRecord=0) const;

	//! Utility method to create a pipeline for each record.
	bool buildPipelineForEachChannel(const boost::filesystem::path& path,
		std::vector<boost::intrusive_ptr<IProcess> >* pVec) const;

private:

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_mutex;
};

#endif //end guard
