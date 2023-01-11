#ifndef PYXIS__CATALOG_H
#define PYXIS__CATALOG_H
/******************************************************************************
catalog.h

begin		: 2015-09-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/record.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/utility/object.h"

// standard includes

/*!
PYXDataSet describes a data set (typically a layer or set of bands) in a data source.
*/
//! Describes a data set in a data source.
class PYXLIB_DECL PYXDataSet : public IRecord
{
public:
	/*
	The following are metadata values which may be present in a data set and feature or coverage
	pipeline. The dimension values are intended to represent a dimension within a given catalog.
	For example, in a catalog containing data sets which represent a time series, each data set
	will contain a time dimension value which represents its position in the time series.
	*/

	//! Identifies an int32 which represents the number of seconds since the start of the Unix epoch
	static const std::string s_strPyxisDimensionTime;

	//! Identifies an int32 which represents the number of metres above ground level
	static const std::string s_strPyxisDimensionHeight;

	//! Identifies an int32 which represents the model number in a GRIB ensemble
	static const std::string s_strPyxisDimensionGRIBModel;

	//! Identifies a string which is the long name for the data set
	static const std::string s_strPyxisLongName;

	//! Identifies a string which is the short name for the data set
	static const std::string s_strPyxisShortName;

	//! Identifies a string that is the units of measurement for the data set
	static const std::string s_strPyxisUnits;

	//! Identifies a double which represents the number of features in the data set
	static const std::string s_strPyxisFeatureCount;

	//! Identifies a double which represents the number of pixels in the data set
	static const std::string s_strPyxisPixelCount;

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IRecord contains information about the data set, not its contents.

	IRECORD_IMPL();
	
public:

	//! Test method
	static void test();

	//! Create method
	static PYXPointer<PYXDataSet> create()
	{
		return PYXPointer<PYXDataSet> (new PYXDataSet);
	}

	//! Create method
	static PYXPointer<PYXDataSet> create(const std::string& strUri, const std::string& strName)
	{
		return PYXPointer<PYXDataSet> (new PYXDataSet(strUri, strName));
	}

	//! Copy create method
	static PYXPointer<PYXDataSet> create(const PYXDataSet& rhs)
	{
		return PYXPointer<PYXDataSet> (new PYXDataSet(rhs));
	}

	//! Clone method
	virtual PYXPointer<PYXDataSet> clone() const
	{
		return create(*this);
	}

	//! Destructor
	virtual ~PYXDataSet() {}

	//! Can the data set be loaded? (i.e. all required files are present)
	bool isLoadable() const { return m_vecMissingRequiredFilesAllOf.empty() && m_vecMissingRequiredFilesOneOf.empty(); }

	//! Get the uri
	const std::string& getUri() const { return m_strUri; }

	//! Set the uri
	void setUri(const std::string& strUri) { m_strUri = strUri; }

	//! Get the name
	const std::string& getName() const { return m_strName; }

	//! Set the name
	void setName(const std::string& strName) { m_strName = strName; }

	//! Set the layer name
	void setLayer(const std::string& strLayer) { m_strLayer = strLayer; }

	//! Get the layer name
	const std::string& getLayer() const { return m_strLayer; }

	//! Get the table definition that describes the content of the data set
	PYXPointer<const PYXTableDefinition> getContentDefinition() const {return m_pContentDefinition;}

	//! Get the table definition that describes the content of the data set
	PYXPointer<PYXTableDefinition> getContentDefinition() {return m_pContentDefinition;}

	//! Get any missing required files (all files are required)
	const std::vector<std::string>& getMissingRequiredFilesAllOf() const { return m_vecMissingRequiredFilesAllOf; }

	//! Set any missing required files (all files are required)
	void setMissingRequiredFilesAllOf(const std::vector<std::string>& vecMissingFiles) { m_vecMissingRequiredFilesAllOf = vecMissingFiles; }

	//! Get any missing required file (one file is required)
	const std::vector<std::string>& getMissingRequiredFilesOneOf() const { return m_vecMissingRequiredFilesOneOf; }

	//! Set any missing required files (one file is required)
	void setMissingRequiredFilesOneOf(const std::vector<std::string>& vecMissingFiles) { m_vecMissingRequiredFilesOneOf = vecMissingFiles; }

	//! Get any missing optional files
	const std::vector<std::string>& getMissingOptionalFiles() const { return m_vecMissingOptionalFiles; }

	//! Set any missing optional files
	void setMissingOptionalFiles(const std::vector<std::string>& vecMissingFiles) { m_vecMissingOptionalFiles = vecMissingFiles; }

	void getBoundingBox(PYXRect2DDouble & bbox1, PYXRect2DDouble & bbox2) const;

	void setBoundingBox(PYXGeometry & geometry);

protected:

	//! Default constructor.
	PYXDataSet();

	//! Constructor
	PYXDataSet(const std::string& strUri, const std::string& strName);

	//! Copy constructor
	PYXDataSet(const PYXDataSet& dataSet);

	//! Copy assignment
	PYXDataSet operator=(const PYXDataSet& dataSet);

private:

	//! The uri of the data set
	std::string m_strUri;

	//! The name of the data set
	std::string m_strName;

	//! The name of the layer within the data set
	std::string m_strLayer;
	
	//! The number of pixels in the data set or a negative number if not known
	double m_nPixelCount;

	//! The number of features in the data set or a negative number if not known
	double m_nFeatureCount;

	//! The table definition that describes the content of the data set
	PYXPointer<PYXTableDefinition> m_pContentDefinition;

	//! Missing required files (all of these must be present to load the data source)
	std::vector<std::string> m_vecMissingRequiredFilesAllOf;

	//! Missing required file (one of these must be present to load the data source)
	std::vector<std::string> m_vecMissingRequiredFilesOneOf;

	//! Any missing optional files (the data source can still be loaded if these are missing)
	std::vector<std::string> m_vecMissingOptionalFiles;

	//! bounding box for this dataset
	PYXRect2DDouble m_bbox1;
	PYXRect2DDouble m_bbox2;

	friend PYXLIB_DECL bool operator ==(const PYXDataSet& lhs, const PYXDataSet& rhs);
	friend PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXDataSet& defn);
	friend PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXDataSet& defn);

	friend class PYXCatalog;
};

//!The equality operator.
PYXLIB_DECL bool operator ==(const PYXDataSet& lhs, const PYXDataSet& rhs);

//! The inequality operator.
PYXLIB_DECL bool operator !=(const PYXDataSet& lhs, const PYXDataSet& rhs);

//! Stream operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXDataSet& defn);

//! Stream operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXDataSet& defn);


////////////////////////////////////////////////////////////////////////////////

/*!
PYXDataSetCatalog describes the data sets (typically layers) in a data source. Also
catalogs can have sub catalogs within them.
*/
//! Describes the data sets in a data source.
class PYXLIB_DECL PYXCatalog : public PYXObject
{
public:

	//! Test method
	static void test();

	//! Typedef for vector of data set definitions
	typedef std::vector< PYXPointer<PYXDataSet> > DataSetVector;

	//! Typedef for vector of catalog definitions
	typedef std::vector< PYXPointer<PYXCatalog> > CatalogVector;

	//! Create method
	static PYXPointer<PYXCatalog> create()
	{
		return PYXNEW(PYXCatalog);
	}

	//! Create method
	static PYXPointer<PYXCatalog> create(const std::string& strUri, const std::string& strName)
	{
		return PYXNEW(PYXCatalog, strUri, strName);
	}

	//! Copy create method
	static PYXPointer<PYXCatalog> create(const PYXCatalog& rhs)
	{
		return PYXNEW(PYXCatalog, rhs);
	}

	//! Clone method
	virtual PYXPointer<PYXCatalog> clone() const
	{
		return create(*this);
	}

	//! Destructor
	virtual ~PYXCatalog() {}

	//! Get the uri.
	const std::string& getUri() const { return m_strUri; }

	//! Set the uri
	void setUri(const std::string& strUri) { m_strUri = strUri; }

	//! Get the name.
	const std::string& getName() const { return m_strName; }

	//! Set the name
	void setName(const std::string& strName) { m_strName = strName; }

	//! Add a data set catalog entry
	int addDataSet(PYXPointer<PYXDataSet> pDataSet);

	//! Get the number of data sets in this catalog.
	int getDataSetCount() const {return static_cast<int>(m_vecDataSets.size());}

	//! Get a data set by index.
	PYXPointer<const PYXDataSet> getDataSet(int nDataSetIndex) const;

	//! Add a catalog
	int addSubCatalog(PYXPointer<PYXCatalog> pCatalog);

	//! Get the number of sub-catalogs in this catalog.
	int getSubCatalogCount() const {return static_cast<int>(m_vecSubCatalogs.size());}

	//! Get a sub-catalog by index.
	PYXPointer<const PYXCatalog> getSubCatalog(int nSubCatalogIndex) const;

protected:

	//! Default constructor.
	PYXCatalog() {}

	//! Constructor
	PYXCatalog(const std::string& strUri, const std::string& strName);

	//! Copy constructor
	PYXCatalog(const PYXCatalog& catalog);

	//! Copy assignment
	PYXCatalog operator=(const PYXCatalog& catalog);

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

private:

	//! The uri of the data set
	std::string m_strUri;

	//! The name of the data set
	std::string m_strName;

	//! Vector of data sets
	mutable DataSetVector m_vecDataSets;

	//! Vector of catalog definitions
	mutable CatalogVector m_vecSubCatalogs;

	friend PYXLIB_DECL bool operator ==(const PYXCatalog& lhs, const PYXCatalog& rhs);
	friend PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXCatalog& defn);
	friend PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXCatalog& defn);
};

//!The equality operator.
PYXLIB_DECL bool operator ==(const PYXCatalog& lhs, const PYXCatalog& rhs);

//! The inequality operator.
PYXLIB_DECL bool operator !=(const PYXCatalog& lhs, const PYXCatalog& rhs);

//! Stream operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXCatalog& defn);

//! Stream operator.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXCatalog& defn);

#endif // guard
