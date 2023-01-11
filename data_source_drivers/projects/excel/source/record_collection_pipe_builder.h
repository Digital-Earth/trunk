#ifndef RECORD_COLLECTION_PIPE_BUILDER_H
#define RECORD_COLLECTION_PIPE_BUILDER_H
/***************************************************************************
record_collection_pipe_builder.h

begin		: 08/07/2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "excel.h"
#include "pyxis/pipe/pipe_builder.h"

//TODO: Documentation.
class EXCEL_DECL RecordCollectionPipeBuilder : public PipeBuilderBase
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeBuilder)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:

	//! Default Constructor
	RecordCollectionPipeBuilder();

	//! Destructor
	~RecordCollectionPipeBuilder(){;}

	//! Build a pipeline for the data set.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

private:	
	//! Helper method to build an csv pipelines.
	void createCsvPipeline(std::vector<boost::intrusive_ptr<IProcess> > & result,const boost::filesystem::path &path) const;

	//! Helper method to build an excel pipelines.
	void createExcelPipelines(std::vector<boost::intrusive_ptr<IProcess> > & result,const boost::filesystem::path &path) const;
};

#endif // end guard