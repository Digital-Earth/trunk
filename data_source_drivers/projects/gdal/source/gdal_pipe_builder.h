#ifndef GDAL_PIPE_BUILDER_H
#define GDAL_PIPE_BUILDER_H
/***************************************************************************
gdal_pipe_builder.h

begin		: 2007-12-03
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "module_gdal.h"
#include "pyxis/pipe/pipe_builder.h"

// GDAL includes
#include "gdal_priv.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// forward declarations
class PYXSharedGDALDataSet;

/*!
Builds GDAL pipelines, for the files that can be opened through the GDAL file reader.
*/
//! GDal Pipeline builder.
class MODULE_GDAL_DECL GDALPipeBuilder : public PipeBuilderBase
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IPipeBuilder)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	
	//! Test method
	static void test();

	//! Default Constructor
	GDALPipeBuilder();
	
	//! Destructor
	~GDALPipeBuilder(){;}

	//! Determine if a specific data source can be read.
	virtual bool STDMETHODCALLTYPE isDataSourceSupported(const std::string& strPath, eCheckOptions options) const;

	//! Build a pipeline for the data set.
	virtual boost::intrusive_ptr<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

	//! Build the catalog that describes the data source.
	virtual PYXPointer<const PYXCatalog> STDMETHODCALLTYPE buildCatalog(const std::string& strPath) const;

private:

	//! Add the datasets for the GDAL dataset to a catalog.
	void addDataSets(
		PYXPointer<PYXCatalog> pCatalog,
		const std::string& strPath,
		PYXPointer<PYXSharedGDALDataSet> pGDALDataSet) const;

	//! Helper method to create a GDAL process.
	boost::intrusive_ptr<IProcess> createGdalProcess(PYXPointer<PYXDataSet> pDataSet) const;

//--Grayscale RGB specific functions--//
private:
	//! Helper method to check if a coverage is unsigned values
	bool hasSimpleGreyscaleValues(boost::intrusive_ptr<IProcess> spGDalProcess) const;

//--DEM specific functions--//
private:
	//! Helper method to check if a coverage is unsigned values
	bool hasUnsignedValues(boost::intrusive_ptr<IProcess> spGDalProcess) const;

	//! Helper method to check if a coverage is float values.
	bool isFloatValues(boost::intrusive_ptr<IProcess> spGDalProcess) const;

	//! Helper method to create a translation into signed values 
	boost::intrusive_ptr<IProcess> createUnsignedToSignedTranslationProcess(boost::intrusive_ptr<IProcess> spGDalProcess) const;

	//! Helper method to create a cast process into float values 
	boost::intrusive_ptr<IProcess> addCastValuesToFloatProcess(boost::intrusive_ptr<IProcess> spGDalProcess) const;	
};

#endif //end guard