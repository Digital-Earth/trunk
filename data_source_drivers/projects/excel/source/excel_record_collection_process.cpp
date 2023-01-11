/******************************************************************************
excel_record_collection_process.h

begin      : 18/06/2013 12:00:45 PM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"

#define EXCEL_SOURCE

// local includes
#include "excel_record_collection_process.h"

// pyxlib includes
#include "pyxis/data/record_collection.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/excel.h"

#include <fstream>

// {4F5B6B1C-67C7-40A6-8E98-2FF511D38AAF}
PYXCOM_DEFINE_CLSID(ExcelRecordCollectionProcess , 
0x4f5b6b1c, 0x67c7, 0x40a6, 0x8e, 0x98, 0x2f, 0xf5, 0x11, 0xd3, 0x8a, 0xaf);

PYXCOM_CLASS_INTERFACES(ExcelRecordCollectionProcess , IProcess::iid, IRecordCollection::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ExcelRecordCollectionProcess , "Excel File Reader", "Read Excel file into a Record Collection.", "Reader",
	IRecordCollection::iid, IRecord::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "Excel Path", "The input Excel file.");
IPROCESS_SPEC_END

namespace
{
	//! The unit test class
	Tester< ExcelRecordCollectionProcess  > gTester;	
}

/*!
The unit test method for the class.
*/
void ExcelRecordCollectionProcess ::test()
{
	// Create a test process with a path
	boost::intrusive_ptr< IProcess > spProc(new ExcelRecordCollectionProcess );
	TEST_ASSERT(spProc);

	//TODO: make a unit test here
}

////////////////////////////////////////////////////////////////////////////////
// IRecordCollection
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
// ExcelReader
///////////////////////////////////////////

class ExcelReader
{
private:
	size_t m_rowIndex;
	Excel::IWorkbook::Pointer m_workbook;
	Excel::IWorkbookTable::Pointer m_table;
	Excel::IWorkbookView::Pointer m_view;

public:
	ExcelReader(boost::filesystem::path file,const std::string & tableName) : m_rowIndex(0)
	{
		auto implementation = Excel::IExcel::Implementation();
		m_workbook = implementation->CreateWorkbook(FileUtils::pathToString(file));
		m_table = m_workbook->CreateTable(tableName);
		m_view = m_table->CreateDefaultView();
	}

	~ExcelReader()
	{		
	}
	
public:
	bool endOfFile() const
	{
		return !(m_rowIndex < m_view->GetDataRowCount());
	}

public:
	std::vector<PYXValue> readHeadingRow()
	{
		std::vector<PYXValue> values;
		m_view->GetHeadingRow(values);
		return values;
	}


	std::vector<PYXValue> readRecord()
	{
		std::vector<PYXValue> values;
		m_view->GetDataRow(m_rowIndex,values);		
		m_rowIndex++;
		return values;
	}
};

class ExcelRecord : public IRecord
{
private:
	PYXPointer<PYXTableDefinition> m_definition;
	std::vector<PYXValue> m_values;	

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN		
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IRecord

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const
	{
		return m_definition;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition()
	{
		return m_definition;
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const
	{
		assert(m_values.size() <= boost::integer_traits< int >::const_max);
		if (0 <= nFieldIndex && nFieldIndex < static_cast<int>(m_values.size()))
		{
			return m_values[nFieldIndex];
		}
		return PYXValue();
	}

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex)
	{
		PYXTHROW(PYXException,"Setting values not allowed for ExcelRecord");
	}

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(std::string const & strName) const
	{
		int nFieldIndex = getDefinition()->getFieldIndex(strName);
		PYXValue value;
		if (0 <= nFieldIndex)
		{
			value = getFieldValue(nFieldIndex);
		}
		return value;
	}

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, std::string const & strName)
	{
		PYXTHROW(PYXException,"Setting values not allowed for ExcelRecord");
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_values;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(std::vector<PYXValue> const & vecValues)
	{
		PYXTHROW(PYXException,"Setting values not allowed for ExcelRecord");
	}

	virtual void STDMETHODCALLTYPE addField(
		const std::string& strName,
		PYXFieldDefinition::eContextType nContext,
		PYXValue::eType nType,
		int nCount = 1,
		PYXValue value = PYXValue())
	{
		PYXTHROW(PYXException,"Setting values not allowed for ExcelRecord");
	}


public:
	static boost::intrusive_ptr<IRecord> create(const std::vector<PYXValue> & values,const PYXPointer<PYXTableDefinition> definition)
	{
		auto record = boost::intrusive_ptr<ExcelRecord>(new ExcelRecord());
		record->m_definition = definition;

		for(int fieldIndex = 0; fieldIndex < definition->getFieldCount(); ++fieldIndex)
		{
			auto & field = definition->getFieldDefinition(fieldIndex);
			if (values[fieldIndex].isNull())
			{
				//push null value
				record->m_values.push_back(PYXValue());
			}
			else if (field.getType() == PYXValue::knString)
			{
				//getTypeCompatibleValue() for type string is not thorwing exception
				record->m_values.push_back(PYXValue(values[fieldIndex].getString()));
			}
			else
			{
				PYXValue value = field.getTypeCompatibleValue();
				value.setString(values[fieldIndex].getString());
				record->m_values.push_back(value);
			}
		}

		return record;
	}
};

class ExcelRecordIterator : public RecordIterator
{
private:
	ExcelReader m_reader;
	PYXPointer<PYXTableDefinition> m_definition;
	boost::intrusive_ptr<IRecord> m_currentRecord;

public:

	virtual bool end() const 
	{
		return !m_currentRecord;
	}

	virtual void next()
	{
		if (!m_reader.endOfFile())
		{
			m_currentRecord = ExcelRecord::create(m_reader.readRecord(),m_definition);
		}
		else
		{
			m_currentRecord.reset();
		}
	}

	virtual boost::intrusive_ptr<IRecord> getRecord() const 
	{
		return m_currentRecord;
	}

public:
	static PYXPointer<ExcelRecordIterator> create(const boost::filesystem::path & file,const std::string & table,const PYXPointer<PYXTableDefinition> & definition)		
	{
		return PYXNEW(ExcelRecordIterator,file,table,definition);
	}

	ExcelRecordIterator(const boost::filesystem::path & file,const std::string & table, const PYXPointer<PYXTableDefinition> & definition) : m_reader(file,table),m_definition(definition)
	{
		//read first record
		next();
	}
};


PYXPointer<RecordIterator> STDMETHODCALLTYPE ExcelRecordCollectionProcess ::getIterator() const
{
	return ExcelRecordIterator::create(m_inputFile,m_tableName,m_recordDefinition);
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE ExcelRecordCollectionProcess ::getRecordDefinition() const
{
	return m_recordDefinition;
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE ExcelRecordCollectionProcess ::getRecord(const std::string & strRecordID) const
{
	PYXPointer<RecordIterator> iterator = getIterator();
	int recordId= StringUtils::fromString<int>(strRecordID);
	int currentRecord = 0;
	while(!iterator->end() && currentRecord < recordId)
	{
		iterator->next();
	}

	if (iterator->end())
	{
		PYXTHROW(PYXException,"Record ID is out of range");
	}

	return iterator->getRecord();
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map< std::string, std::string > STDMETHODCALLTYPE ExcelRecordCollectionProcess ::getAttributes() const
{
	std::map< std::string, std::string > mapAttr;	

	mapAttr["Table"] = m_tableName;
	return mapAttr;
}

void STDMETHODCALLTYPE ExcelRecordCollectionProcess ::setAttributes(std::map< std::string, std::string > const & mapAttr)
{
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Table",m_tableName);
}

std::string STDMETHODCALLTYPE ExcelRecordCollectionProcess ::getAttributeSchema() const
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"ExcelRecordCollectionProcess \">"
		  "<xs:complexType>"
			"<xs:sequence>"
			
			 "<xs:element name=\"Table\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Table Name</friendlyName>"
					"<description>The table name for the worksheet.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus ExcelRecordCollectionProcess ::initImpl()
{
	boost::intrusive_ptr<IPath> pathProcess = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IPath>();

	m_inputFile = FileUtils::stringToPath(pathProcess->getLocallyResolvedPath());	

	if (!extractRecordDefinition())
	{
		return knFailedToInit;
	}
	
	return knInitialized;
}


bool ExcelRecordCollectionProcess ::extractRecordDefinition()
{
	ExcelReader reader(m_inputFile,m_tableName);

	std::vector<PYXValue> fieldsNames = reader.readHeadingRow();
	std::vector<PYXValue::eType> fieldTypes(fieldsNames.size(),PYXValue::knNull);	

	while(!reader.endOfFile())
	{
		std::vector<PYXValue> values = reader.readRecord();

		for(unsigned int i=0;i < fieldsNames.size(); ++i)
		{
			auto value = values[i].getString();
			//if this is string value - nothing left to do
			if (fieldTypes[i]==PYXValue::knString) 
			{
				continue;
			}
			
			//if this is a char - make sure the value is 1 char long
			if (fieldTypes[i]==PYXValue::knChar)
			{
				if (value.size() > 1)
				{
					fieldTypes[i]=PYXValue::knString;
				}
				continue;
			}

			std::string trimmedValue = StringUtils::trim(value);

			//empty value - aka - null, nothing to do here
			if (trimmedValue.empty())
			{
				continue;
			}
			
			//this is not a number
			if (!StringUtils::isNumeric(trimmedValue))
			{
				fieldTypes[i]=value.size()==1? PYXValue::knChar : PYXValue::knString;
				continue;
			}

			//if the type is double already - there is nothing to do
			if (fieldTypes[i]==PYXValue::knDouble)
			{
				continue;
			}

			bool isInteger = StringUtils::countDecimalPlaces(trimmedValue)==0;

			//try to set it as unsinged integer or a double
			fieldTypes[i]=isInteger? PYXValue::knInt32 : PYXValue::knDouble;
		}
	}

	m_recordDefinition = PYXTableDefinition::create();

	for(unsigned int i=0;i<fieldsNames.size();++i)
	{
		//if we werent able to detect the value type 
		if (fieldTypes[i] == PYXValue::knNull)
		{
			//make it string as default
			fieldTypes[i] = PYXValue::knString;	
		}
		m_recordDefinition->addFieldDefinition(fieldsNames[i].getString(),PYXFieldDefinition::knContextNone,fieldTypes[i]);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

ExcelRecordCollectionProcess ::ExcelRecordCollectionProcess ()
{
}

ExcelRecordCollectionProcess ::~ExcelRecordCollectionProcess ()
{
}

