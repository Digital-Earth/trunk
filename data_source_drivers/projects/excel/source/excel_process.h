#ifndef EXCEL__EXCEL_PROCESS_H
#define EXCEL__EXCEL_PROCESS_H

/******************************************************************************
excel_process.h

begin      : 08/25/2010 12:00:45 PM
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "excel.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/excel.h"

// local forward declarations
class ExcelFeature;
template <
	typename FeatureCollectionType,
	typename FeatureType > class AllFeatureIterator;

/*!
The Excel Process creates a feature collection to generate a point data from an 
Excel file. The Excel file is subject to the following conditions:

- The file must have a contiguous data block.
- Each row represents one point data feature.
- Each column must be of uniform type (integer, float, or string)
- Formula results are not processed, only raw data values.
- Data values of unknown type are ignored.
- Coordinates are specified in decimal degrees in WGS84.
- Empty cells within the data block are treated as 0.0 (double) or "" (string), not as NULL.
*/
//! A process for Excel files.
/*
TODO:
- Allow different coordinate converters
- Feature Styles?
- Enable testing with persistent test file
- allow a field to represent the icon
*/
class EXCEL_DECL ExcelProcess :
public ProcessImpl< ExcelProcess >,
public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	typedef AllFeatureIterator<
		ExcelProcess, ExcelFeature > AllFeatureIterator;

	//! Unit testing method
	static void test();

private:

	std::vector< FeatureStyle > m_vecStyles; 

	//! Mutex to serialize concurrent access by multiple threads.
	mutable boost::recursive_mutex m_mutex;

	//! A user-friendly name for the worksheet.
	std::string m_strName;

	//! The table name of the worksheet within the workbook.
	//! Not a user-friendly name.
	std::string m_strTableName;

	//! The row that holds the labels for the data source
	int m_nLabelRow;

	//! The column that holds the latitude information
	int m_nLatColumn;

	//! The column that holds the longitude information
	int m_nLonColumn;

	//! The first row of data
	int m_nDataRowStart;

	//! The last row of data
	int m_nDataRowEnd;

	//! The last row of data as it set as the input attributes
	int m_nDataRowEndAttribute;

	//! The first Column of data
	int m_nDataColumnStart;

	//! The last Column of data
	int m_nDataColumnEnd;

	//! The last Column of data  as it set as the input attributes
	int m_nDataColumnEndAttribute;

	//! The number of data rows selected for the process.
	size_t m_nRowCount;

	//! The excel file workbook object.
	Excel::IWorkbook::Pointer m_pWorkbook;

	//! The excel file workbook table object.
	Excel::IWorkbookTable::Pointer m_pWorkbookTable;

	//! The excel file workbook view object.
	Excel::IWorkbookView::Pointer m_pWorkbookView;

	//! The table definition for the features in the collection
	PYXPointer< PYXTableDefinition > m_spFeatureDefn;

	//! Convert from lat lon to PYXIS
	WGS84CoordConverter m_converter;

	//! The PYXIS resolution of the data source.
	int m_nResolution;

	//! Mapping of column to data type.
	std::map< size_t, PYXValue::eType > m_offsetTypeMap;

	//! A row of label values, to be reused for row queries.
	mutable std::vector< PYXValue > m_labelRow;

	//! A row of data values, to be reused for row queries.
	mutable std::vector< PYXValue > m_dataRow;

private:

	ExcelProcess(ExcelProcess const &);
	ExcelProcess & operator =(ExcelProcess);

public:

	//! Constructor
	ExcelProcess();

	size_t getFeatureCount() const;

	boost::intrusive_ptr< ExcelFeature > getFeature(size_t featureOffset) const;

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr< const PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput() const
	{
		return static_cast< const IFeatureCollection * >(this);
	}

	virtual boost::intrusive_ptr< PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput()
	{
		return static_cast< IFeatureCollection * >(this);
	}

	virtual std::map< std::string, std::string > STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map< std::string, std::string > & mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL(); 

public: // IFeatureCollection

	virtual PYXPointer< FeatureIterator > STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer< FeatureIterator > STDMETHODCALLTYPE getIterator(PYXGeometry const & geometry) const;

	virtual PYXPointer< const PYXTableDefinition > STDMETHODCALLTYPE getFeatureDefinition() const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_spFeatureDefn; 
	} 

	virtual PYXPointer< PYXTableDefinition > STDMETHODCALLTYPE getFeatureDefinition()
	{ 
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_spFeatureDefn; 
	} 

	virtual boost::intrusive_ptr< IFeature > STDMETHODCALLTYPE getFeature(std::string const & strFeatureID) const;

	virtual std::vector< FeatureStyle > STDMETHODCALLTYPE getFeatureStyles() const
	{ 
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_vecStyles; 
	} 

	virtual bool STDMETHODCALLTYPE canRasterize(void) const
	{
		return false;
	}

	IFEATURECOLLECTION_IMPL_HINTS();
};

#endif
