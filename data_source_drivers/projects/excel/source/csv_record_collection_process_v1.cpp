/******************************************************************************
csv_record_collection_process.cpp

begin      : 18/06/2013 12:00:45 PM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"

#define EXCEL_SOURCE

// local includes
#include "csv_record_collection_process_v1.h"

// pyxlib includes
#include "pyxis/data/record_collection.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/app_services.h"

#include <fstream>

// {B254DEB4-535A-4176-AB1D-56A76EC31E80}
PYXCOM_DEFINE_CLSID(CsvRecordCollectionProcess_v1, 
0xb254deb4, 0x535a, 0x4176, 0xab, 0x1d, 0x56, 0xa7, 0x6e, 0xc3, 0x1e, 0x80);

PYXCOM_CLASS_INTERFACES(CsvRecordCollectionProcess_v1, IProcess::iid, IRecordCollection::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CsvRecordCollectionProcess_v1, "CSV File Reader v1", "Read CSV file into a Record Collection.", "Hidden",
	IRecordCollection::iid, IRecord::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "CSV Path", "The input CSV file.");
IPROCESS_SPEC_END

namespace
{
	//! The unit test class
	Tester< CsvRecordCollectionProcess_v1 > gTester;	
}

/*!
The unit test method for the class.
*/
void CsvRecordCollectionProcess_v1::test()
{
	// Create a test process with a path
	boost::intrusive_ptr< IProcess > spProc(new CsvRecordCollectionProcess_v1);
	TEST_ASSERT(spProc);

	//TODO: make a unit test here
}

////////////////////////////////////////////////////////////////////////////////
// IRecordCollection
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
// CsvReader_v1
///////////////////////////////////////////
//
// CVS format we follow:
// 1) ',' is a sperator.
// 2) if a line ends with ',' - that mean there is an empty value after that ','
// 3) if a value contain ',' - it must be encapsualted with '"'
// 4) escaping '"' is double quote '""' - and it treaded as '"' inside a value
// 5) leading spaces and trailing spaces are included if value is not qouted ('"')
///////////////////////////////////////////

class CsvReader_v1
{
private:
	boost::filesystem::path m_file;
	std::ifstream m_fileStream;
	bool m_fileIsOpen;
	bool m_eof;
	std::string m_currentLine;

public:
	CsvReader_v1(boost::filesystem::path file) : m_file(file), m_fileIsOpen(false),m_eof(false)
	{
		//should be outside of the constructor
		openFile();
		getNextLine();		
	}

	~CsvReader_v1()
	{
		if (m_fileIsOpen)
		{
			m_fileStream.close();
		}
	}
	
public:
	void openFile()
	{
		if (!m_fileIsOpen)
		{
			m_fileStream.open(FileUtils::pathToString(m_file).c_str(), std::ios::in);	
			// open the file
			if (!m_fileStream.good())
			{
				PYXTHROW(PYXException,"Failed to open file: " << m_file);
			}
			m_fileIsOpen = true;
		}
	}

	bool endOfFile() const
	{
		return m_eof;
	}

private:
	std::string getNextLine()
	{
		if (!m_fileIsOpen)
		{
			PYXTHROW(PYXException,"file not opened");
		}

		//get the last line we read
		std::string strLine = m_currentLine;
		
		// try to read the next next line - to see if we got EOF.
		if (!std::getline(m_fileStream, m_currentLine))
		{
			m_eof = true;
		}
		
		//return the previous line we read
		return strLine;
	}

public:
	enum ReaderState
	{
		knNormalValue,
		knStringValue,
		knStringEnding
	};

	std::vector<std::string> readRecord()
	{
		if (!m_fileIsOpen)
		{
			PYXTHROW(PYXException,"file not opened");
		}

		std::vector<std::string> record;

		ReaderState state = knNormalValue;

		std::string line = getNextLine();
		std::string currentValue;
		
		for(auto & c : line)
		{
			switch(state)
			{
			case knNormalValue:
				switch(c)
				{
				case ',':
					record.push_back(currentValue);
					currentValue.clear();
					break;
				case '"':
					state = knStringValue;
					currentValue.clear(); //ignore everything before the '"'
					break;
				default:
					currentValue.push_back(c);
					break;
				}
				break;

			case knStringValue:
				switch(c)
				{				
				case '"':
					//we move to knStringEnding because this could double quote...
					state = knStringEnding;
					break;
				default:
					currentValue.push_back(c);
					break;
				}
				break;

			case knStringEnding:
				switch(c)
				{				
				case '"': //double "" - which is translted as single '"' value
					currentValue.push_back('"');
					state = knStringValue;
					break;
				case ',':
					record.push_back(currentValue);
					currentValue.clear();
					state = knNormalValue;
					break;

				default: // do nothing - this should be only white space
					break;
				}
				break;
			}
		}

		record.push_back(currentValue);

		return record;
	}
};

class CsvRecord_v1 : public IRecord
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
		PYXTHROW(PYXException,"Setting values not allowed for CsvRecord");
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
		PYXTHROW(PYXException,"Setting values not allowed for CsvRecord");
	}

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const
	{
		return m_values;
	}

	virtual void STDMETHODCALLTYPE setFieldValues(std::vector<PYXValue> const & vecValues)
	{
		PYXTHROW(PYXException,"Setting values not allowed for CsvRecord");
	}

	virtual void STDMETHODCALLTYPE addField(
		const std::string& strName,
		PYXFieldDefinition::eContextType nContext,
		PYXValue::eType nType,
		int nCount = 1,
		PYXValue value = PYXValue())
	{
		PYXTHROW(PYXException,"Setting values not allowed for CsvRecord");
	}


public:
	static boost::intrusive_ptr<IRecord> create(const std::vector<std::string> & values,const PYXPointer<PYXTableDefinition> definition)
	{
		auto record = boost::intrusive_ptr<CsvRecord_v1>(new CsvRecord_v1());
		record->m_definition = definition;

		for(int fieldIndex = 0; fieldIndex < definition->getFieldCount(); ++fieldIndex)
		{
			auto & field = definition->getFieldDefinition(fieldIndex);
			if (values[fieldIndex].empty())
			{
				//push null value
				record->m_values.push_back(PYXValue());
			}
			else if (field.getType() == PYXValue::knString)
			{
				//getTypeCompatibleValue() for type string is not thorwing exception
				record->m_values.push_back(PYXValue(values[fieldIndex]));
			}
			else
			{
				PYXValue value = field.getTypeCompatibleValue();
				value.setString(values[fieldIndex]);
				record->m_values.push_back(value);
			}
		}

		return record;
	}
};

class CsvRecordIterator_v1 : public RecordIterator
{
private:
	CsvReader_v1 m_reader;
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
			m_currentRecord = CsvRecord_v1::create(m_reader.readRecord(),m_definition);
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
	static PYXPointer<CsvRecordIterator_v1> create(const boost::filesystem::path & file,const PYXPointer<PYXTableDefinition> & definition)		
	{
		return PYXNEW(CsvRecordIterator_v1,file,definition);
	}

	CsvRecordIterator_v1(const boost::filesystem::path & file,const PYXPointer<PYXTableDefinition> & definition) : m_reader(file),m_definition(definition)
	{
		//skip headers
		m_reader.readRecord();

		//read first record
		next();
	}
};


PYXPointer<RecordIterator> STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::getIterator() const
{
	return CsvRecordIterator_v1::create(m_inputFile,m_recordDefinition);
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::getRecordDefinition() const
{
	return m_recordDefinition;
}

boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::getRecord(const std::string & strRecordID) const
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

std::map< std::string, std::string > STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::getAttributes() const
{
	std::map< std::string, std::string > mapAttr;	
	return mapAttr;
}

void STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::setAttributes(std::map< std::string, std::string > const & mapAttr)
{
	m_initState = knNeedsInit;
}

std::string STDMETHODCALLTYPE CsvRecordCollectionProcess_v1::getAttributeSchema() const
{
    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"CsvRecordCollectionProcess_v1\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus CsvRecordCollectionProcess_v1::initImpl()
{
	boost::intrusive_ptr<IPath> pathProcess = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IPath>();

	m_inputFile = FileUtils::stringToPath(pathProcess->getLocallyResolvedPath());	

	if (!extractRecordDefinitionFromCache() && !extractRecordDefinition())
	{
		return knFailedToInit;
	}
	
	return knInitialized;
}

bool CsvRecordCollectionProcess_v1::extractRecordDefinitionFromCache()
{
	if (!PipeUtils::isPipelineIdentityStable(this)) {
		return false;
	}

	auto storage = PYXProcessLocalStorage::create(getIdentity());

	std::auto_ptr<PYXConstWireBuffer> geomBuffer = storage->get("csv:definition");

	if (geomBuffer.get() != 0)
	{
		std::string definitionString;
		(*geomBuffer) >> definitionString;

		std::stringstream stream(definitionString);
		
		m_recordDefinition = PYXTableDefinition::create();
		stream >> *m_recordDefinition;

		return true;
	}

	return false;
}

bool CsvRecordCollectionProcess_v1::writeRecordDefinitionToCache()
{
	if (!PipeUtils::isPipelineIdentityStable(this)) {
		return false;
	}

	auto storage = PYXProcessLocalStorage::create(getIdentity());


	PYXStringWireBuffer buffer;
	
	std::stringstream stream;
	stream << *m_recordDefinition;
	std::string definitionString = stream.str();

	buffer << definitionString;

	storage->set("csv:definition",buffer);
	return true;
}

bool CsvRecordCollectionProcess_v1::extractRecordDefinition()
{
	CsvReader_v1 reader(m_inputFile);

	std::vector<std::string> fieldsNames = reader.readRecord();
	std::vector<PYXValue::eType> fieldTypes(fieldsNames.size(),PYXValue::knNull);	

	while(!reader.endOfFile())
	{
		std::vector<std::string> values = reader.readRecord();

		for(unsigned int i=0;i<fieldsNames.size();++i)
		{
			//if this is string value - nothing left to do
			if (fieldTypes[i]==PYXValue::knString) 
			{
				continue;
			}
			
			//if this is a char - make sure the value is 1 char long
			if (fieldTypes[i]==PYXValue::knChar)
			{
				if (values[i].size() > 1)
				{
					fieldTypes[i]=PYXValue::knString;
				}
				continue;
			}

			std::string trimmedValue = StringUtils::trim(values[i]);

			//empty value - aka - null, nothing to do here
			if (trimmedValue.empty())
			{
				continue;
			}
			
			//this is not a number
			if (!StringUtils::isNumeric(trimmedValue))
			{
				fieldTypes[i]=values[i].size()==1?PYXValue::knChar:PYXValue::knString;
				continue;
			}

			//if the type is double already - there is nothing to do
			if (fieldTypes[i]==PYXValue::knDouble)
			{
				continue;
			}

			bool isInteger = StringUtils::countDecimalPlaces(trimmedValue)==0;

			//try to set it as unsigned integer or a double
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
		m_recordDefinition->addFieldDefinition(fieldsNames[i],PYXFieldDefinition::knContextNone,fieldTypes[i]);
	}

	//store definition for next time.
	writeRecordDefinitionToCache();

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

CsvRecordCollectionProcess_v1::CsvRecordCollectionProcess_v1()
{
}

CsvRecordCollectionProcess_v1::~CsvRecordCollectionProcess_v1()
{
}

