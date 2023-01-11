#ifndef EXCEL__EXCEL_RECORD_COLLECTION_PROCESS_H
#define EXCEL__EXCEL_RECORD_COLLECTION_PROCESS_H

/******************************************************************************
excel_record_collection_process.h

begin      : 18/06/2013 12:00:45 PM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "excel.h"

// pyxlib includes
#include "pyxis/data/record_collection.h"
#include "pyxis/pipe/process.h"

#include "pyxis/utility/file_utils.h"

/*!
The Excel Process creates a recrod collection from a single table in an Excel file. 

The Excel table file is subject to the following conditions:
- first row is been consider as fields names (no title detecation yet)
- The file must have a contiguous data block.
- Each row represents one point data feature.
- Each column must be of uniform type (integer, float, or string)
- Data values of unknown type are ignored.

*/
class EXCEL_DECL ExcelRecordCollectionProcess : public ProcessImpl< ExcelRecordCollectionProcess  >, public IRecordCollection
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown
	 
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IRecord)
		IUNKNOWN_QI_CASE(IRecordCollection)		
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();

public: // IRecordCollection

	virtual PYXPointer<RecordIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getRecordDefinition() const;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE getRecord(const std::string & strRecordID) const;

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IRecordCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IRecordCollection*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public:
	ExcelRecordCollectionProcess ();

	virtual ~ExcelRecordCollectionProcess ();

public:
	static void test(); 


private:
	bool extractRecordDefinition();

private:
	PYXPointer<PYXTableDefinition> m_recordDefinition;
	boost::filesystem::path m_inputFile;
	std::string m_tableName;
};

#endif
