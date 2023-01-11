#ifndef OGR_PIPE_BUILDER_H
#define OGR_PIPE_BUILDER_H
/***************************************************************************
ogr_pipe_builder.h

begin		: 2007-12-07
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "module_gdal.h"
#include "ogr_process.h"
#include "pyxis/pipe/pipe_builder.h"

//TODO: Documentation.
class MODULE_GDAL_DECL OgrPipeBuilder : public PipeBuilderBase
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
	OgrPipeBuilder();

	//! Destructor
	~OgrPipeBuilder(){;}

	//! Determine if a specific data source can be read.
	virtual bool STDMETHODCALLTYPE isDataSourceSupported(const std::string& strPath, eCheckOptions options) const;

	//! Build the catalog that describes the data source.
	virtual PYXPointer<const PYXCatalog> STDMETHODCALLTYPE buildCatalog(const std::string& strPath) const;

	//! Build a pipeline for the data set.
	virtual PYXPointer<IProcess> STDMETHODCALLTYPE buildPipeline(PYXPointer<PYXDataSet> pDataSet) const;

private:

	//! list of all files that should appear if this is a VMAP directory
	std::vector<std::string> m_vmapNeededFiles;

	//! Check if path is an ESRI File Geodatabase
	bool isESRIFileGeodatabase(const std::string& strPath) const;

	//! Check if directory is a VMAP directory
	bool isVMAP0Directory(const std::string& strPath) const;

	//! Helper method to build an individual OGRProcess from a path.
	boost::intrusive_ptr<OGRProcess> OgrPipeBuilder::createOGRProcess(PYXPointer<PYXDataSet> pDataSet) const;

	//! Add the datasets for an AutoCAD data source to a catalog.
	void addAutoCADDataSets(
		PYXPointer<PYXSharedGDALDataSet> pOGRDataSource,
		PYXPointer<PYXCatalog> pCatalog,
		const std::string& strPath,
		const std::string& strLeaf	) const;
};
#endif // end guard