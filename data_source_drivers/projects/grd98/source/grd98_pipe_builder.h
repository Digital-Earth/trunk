#ifndef GRD98_PIPE_BUIDLER_H
#define GRD98_PIPE_BUILDER_H
/***************************************************************************
grd98_pipe_builder.h

begin		: 2008-04-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "module_grd98.h"
#include "pyxis/pipe/pipe_builder.h"

/*!
Builds a pipeline for GRD98 datasources injecting the required processes
in the pipeline chain to read & sample a GRD98 data source into Pyxis.
The pipeline generated is: FileProc->GRD98Reader->Sampler->Cache.
*/
//! Generates the required pipeline to read a GRD98 datasource in Pyxis.
class MODULE_GRD98_DECL GRD98PipeBuilder : public PipeBuilderBase
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown.

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeBuilder)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: //GRD98PipeBuilder.

	//! Default Constructor
	GRD98PipeBuilder();
	
	//! Destructor
	~GRD98PipeBuilder(){;}

	//! Build the catalog that describes the data source.
	virtual PYXPointer<const PYXCatalog> STDMETHODCALLTYPE buildCatalog(const std::string& strPath) const;

	//! Build a pipeline for the data set.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

private:

	//! Helper method to build an individual pipeline.
	boost::intrusive_ptr<IProcess> buildAPipeline(const boost::filesystem::path& path) const;
};

#endif