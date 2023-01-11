/******************************************************************************
record.cpp

begin		: 2007-02-28
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/record.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_utils.h"

// {D8EE8449-0249-4221-9417-17518A9C4DBC}
PYXCOM_DEFINE_IID(IRecord, 
0xd8ee8449, 0x249, 0x4221, 0x94, 0x17, 0x17, 0x51, 0x8a, 0x9c, 0x4d, 0xbc);

////////////////////////////////////////////////////////////////////////////////

//! Header string
const std::string kstrScopeField("PYXFieldDefinition");

//! Tester class
Tester<PYXFieldDefinition> gTesterField;

//! Test method
void PYXFieldDefinition::test()
{
	PYXFieldDefinition defn(
		"test with spaces",
		knContextRGB,
		PYXValue::knChar,
		3	);

	// test accessors
	TEST_ASSERT(defn.getName() == "test with spaces");
	TEST_ASSERT(defn.getContext() == knContextRGB);
	TEST_ASSERT(defn.getType() == PYXValue::knChar);
	TEST_ASSERT(defn.getCount() == 3);

	{
		// test copy constructor and equality operator
		PYXFieldDefinition defn2(defn);
		TEST_ASSERT(defn == defn2);
		TEST_ASSERT(!(defn != defn2));
	}

	{
		// test copy assignment and equality operator
		PYXFieldDefinition defn2;
		defn2 = defn;
		TEST_ASSERT(defn == defn2);
		TEST_ASSERT(!(defn != defn2));
	}

	{
		// test streaming
		std::stringstream ss;
		ss << defn;

		PYXFieldDefinition defn2;
		ss >> defn2;

		TEST_ASSERT(defn == defn2);
	}
}

/*!
Copy constructor.

\param defn	The definition to assign to this one.
*/
PYXFieldDefinition::PYXFieldDefinition(const PYXFieldDefinition& defn)
{
	m_strName = defn.m_strName;
	m_nContext = defn.m_nContext;
	m_nType = defn.m_nType;
	m_nCount = defn.m_nCount;
}

/*!
Copy assignment.

\param defn	The definition to assign to this one.
*/
PYXFieldDefinition PYXFieldDefinition::operator=(const PYXFieldDefinition& defn)
{
	if (this != &defn)
	{
		m_strName = defn.m_strName;
		m_nContext = defn.m_nContext;
		m_nType = defn.m_nType;
		m_nCount = defn.m_nCount;
	}

	return *this;
}

/*!
Checks to see if the field definitions are equal.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if the same, false otherwise.
*/
bool operator==(const PYXFieldDefinition& lhs, const PYXFieldDefinition& rhs)
{
	return (	(lhs.m_strName == rhs.m_strName) &&
				(lhs.m_nContext == rhs.m_nContext) &&
				(lhs.m_nType == rhs.m_nType) &&
				(lhs.m_nCount == rhs.m_nCount)	);
}

/*!
Checks to see if the field definitions are not equal.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if not equal, false if equal.
*/
bool operator!=(const PYXFieldDefinition& lhs, const PYXFieldDefinition& rhs) 
{
	return !(lhs == rhs);
}


/*!
Write a PYXFieldDefinition to a stream.
*/
std::ostream& operator <<(std::ostream& out, const PYXFieldDefinition& defn)
{
	// write header and version
	out << kstrScopeField << " " << "0.1" << std::endl;

	// write fields
	out << defn.m_strName << std::endl;
	out << defn.m_nContext << std::endl;
	out << defn.m_nType << std::endl;
	out << defn.m_nCount << std::endl;

	return out;
}

/*!
Read a PYXTableDefinition from a stream.
*/
std::istream& operator >>(std::istream& in, PYXFieldDefinition& defn)
{
	// read and check for valid header
	std::string strArg;
	in >> strArg;
	if (strArg != kstrScopeField)
	{
		PYXTHROW(PYXDefinitionException, "Invalid field definition.");
	}

	// read version
	std::string strVersion;
	getline(in, strVersion);		// ensure we read past the EOL

	// read fields
	getline(in, defn.m_strName);	// name may contain embedded spaces etc.

	int nContext;
	in >> nContext;
	defn.m_nContext = static_cast<PYXFieldDefinition::eContextType>(nContext);

	int nType;
	in >> nType;
	defn.m_nType = static_cast<PYXValue::eType>(nType);

	in >> defn.m_nCount;

	// consume trailing newline
	in.ignore(1, '\n');

	return in;
}

////////////////////////////////////////////////////////////////////////////////

//! Header string
const std::string kstrScopeTable("PYXTableDefinition");

//! Tester class
Tester<PYXTableDefinition> gTesterTable;

//! Test method
void PYXTableDefinition::test()
{
	PYXFieldDefinition fieldDefn1(
		"1",
		PYXFieldDefinition::knContextRGB,
		PYXValue::knChar,
		3	);

	// test getFieldCount(), getFieldDefinition() and getFieldIndex()
	PYXTableDefinition defn;
	{
		TEST_ASSERT(defn.getFieldCount() == 0);

		defn.addFieldDefinition(fieldDefn1);
		TEST_ASSERT(defn.getFieldCount() == 1);
		TEST_ASSERT(defn.getFieldDefinition(0) == fieldDefn1);

		PYXFieldDefinition fieldDefn2(
			"2",
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1	);
		defn.addFieldDefinition(
			fieldDefn2.getName(),
			fieldDefn2.getContext(),
			fieldDefn2.getType(),
			fieldDefn2.getCount()	);
		TEST_ASSERT(defn.getFieldCount() == 2);
		TEST_ASSERT(defn.getFieldDefinition(1) == fieldDefn2);

		TEST_ASSERT(defn.getFieldIndex("1") == 0);
		TEST_ASSERT(defn.getFieldIndex("2") == 1);
	}

	{
		// test copy constructor and equality operator
		PYXTableDefinition defn2(defn);
		TEST_ASSERT(defn == defn2);
		TEST_ASSERT(!(defn != defn2));
	}

	{
		// test copy assignment and equality operator
		PYXTableDefinition defn2;
		defn2 = defn;
		TEST_ASSERT(defn == defn2);
		TEST_ASSERT(!(defn != defn2));
	}

	{
		// test streaming
		std::stringstream ss;
		ss << defn;

		PYXTableDefinition defn2;
		ss >> defn2;

		TEST_ASSERT(defn == defn2);
	}
}

/*!
Copy constructor

\param defn	The definition to assign to this one.
*/
PYXTableDefinition::PYXTableDefinition(const PYXTableDefinition& defn)
{
	m_vecFieldDefns = defn.m_vecFieldDefns;
}

/*!
Copy assignment.

\param defn	The definition to assign to this one.
*/
PYXTableDefinition PYXTableDefinition::operator=(const PYXTableDefinition& defn)
{
	if (this != &defn)
	{
		m_vecFieldDefns = defn.m_vecFieldDefns;
	}

	return *this;
}

/*!
Add a field definition.

\param	strName		The name of the field.
\param	nContext	How the field is to be interpreted.
\param	nType		The expected data type of the field.
\param	nCount		The number of values for a single field (i.e. 3 for RGB)

\return	The index of the field.
*/
int PYXTableDefinition::addFieldDefinition(	const std::string& strName,
											PYXFieldDefinition::eContextType nContext,
											PYXValue::eType nType,
											int nCount	)
{
	assert((getFieldIndex(strName) < 0) && "Field already exists.");
	m_vecFieldDefns.push_back(PYXFieldDefinition(strName, nContext, nType, nCount));

	return static_cast<int>(m_vecFieldDefns.size() - 1);
}

/*!
Add a field definition.

\param	fieldDefn	The field definition.

\return	The index of the field.
*/
int PYXTableDefinition::addFieldDefinition(const PYXFieldDefinition& fieldDefn)
{
	assert((getFieldIndex(fieldDefn.getName()) < 0) && "Field already exists.");
	m_vecFieldDefns.push_back(fieldDefn);

	return static_cast<int>(m_vecFieldDefns.size() - 1);
}

/*!
Get a field definition by index. Asserts if the index is out of range.

\param	nFieldIndex	The index.

\return	The field definition.
*/
const PYXFieldDefinition& PYXTableDefinition::getFieldDefinition(int nFieldIndex) const
{
	assert(nFieldIndex >= 0 && "Invalid argument");
	assert(nFieldIndex < static_cast<int>(m_vecFieldDefns.size()) && "Invalid argument.");
	if (m_vecFieldDefns[nFieldIndex].getName().empty())
	{
		m_vecFieldDefns[nFieldIndex].m_strName = intToString(nFieldIndex, 3);
	}
	return m_vecFieldDefns[nFieldIndex];
}

/*!
Get a field index by name.

\param	strName	The name

\return	The field index or -1 if the field was not found.
*/
int PYXTableDefinition::getFieldIndex(const std::string& strName) const
{
	FieldDefinitionVector::const_iterator it = m_vecFieldDefns.begin();
	for (; it != m_vecFieldDefns.end(); ++it)
	{
		if (it->getName() == strName)
		{
			return (static_cast<int>(it - m_vecFieldDefns.begin()));
		}
	}

	return (-1);
}

/*!
Checks to see if the table definitions are equal. The table definitions are
considered to be equal if the contain the same field definitions in the same
order.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if the same, false otherwise.
*/
bool operator==(const PYXTableDefinition& lhs, const PYXTableDefinition& rhs)
{
	return (lhs.m_vecFieldDefns == rhs.m_vecFieldDefns);		
}

/*!
Checks to see if the table definitions are not equal.

\param	lhs		The left hand side.
\param	rhs		The right hand side.

\return	true if not equal, false if equal.
*/
bool operator !=(const PYXTableDefinition& lhs, const PYXTableDefinition& rhs) 
{
	return !(lhs == rhs);
}

/*!
Write a PYXTableDefinition to a stream.
*/
std::ostream& operator <<(std::ostream& out, const PYXTableDefinition& defn)
{
	// write header
	out << kstrScopeTable << " " << "0.1" << std::endl;

	int nFieldCount = defn.getFieldCount();

	// write number of field definitions
	out << nFieldCount << std::endl;

	// write field definitions
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		out << defn.getFieldDefinition(nField) << std::endl;
	}

	return out;
}

/*!
Read a PYXTableDefinition from a stream.
*/
std::istream& operator >>(std::istream& in, PYXTableDefinition& defn)
{
	// empty any previous field definitions
	defn.m_vecFieldDefns.clear();

	// read and check for valid header
	std::string strArg;
	in >> strArg;
	if (strArg != kstrScopeTable)
	{
		PYXTHROW(PYXDefinitionException, "Invalid table definition.");
	}

	// read version
	std::string strVersion;
	in >> strVersion;

	// read number of field definitions
	int nFieldCount;
	in >> nFieldCount;

	// consume trailing newline
	in.ignore(1, '\n');

	// read field definitions
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		PYXFieldDefinition fieldDefn;

		in >> fieldDefn;
		defn.addFieldDefinition(fieldDefn);

		// consume trailing newline
		in.ignore(1, '\n');
	}

	return in;
}

/*!
Get a vector containing field names in the order they appear in the definition.

\return	A vector of strings.
*/
std::vector<std::string> PYXTableDefinition::getFieldNames() const
{
	std::vector<std::string> vecNames;

	int nFieldCount = getFieldCount();
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		vecNames.push_back(getFieldDefinition(nField).getName());
	}

	return vecNames;
}


//! get all the fields of a record as XML.
std::string RecordTools::getFieldsAsXml(const IRecord& record) 
{
    std::string strReturn; 

    if ((record.getDefinition().get() != 0) && 
        record.getDefinition()->getFieldCount() > 0) 
    { 
	    strReturn = "<record>"; 
        for (int i=0; i < record.getDefinition()->getFieldCount(); i++) 
        { 
			strReturn += "\n\t<field>\n";
			strReturn += "\t\t<name>";
			strReturn += XMLUtils::toSafeXMLText(record.getDefinition()->getFieldDefinition(i).getName(),true);
			strReturn += "</name>\n";
            PYXValue pv = record.getFieldValue(i); 
            for (int index = 0; index < pv.getArraySize(); index++) 
            { 
			    strReturn += "\t\t<value>";
				strReturn += XMLUtils::toSafeXMLText(pv.getString(index), true);
				strReturn += "</value>\n"; 
            } 
			strReturn += "\t</field>";
        } 
	    strReturn += "</record>"; 
    } 
	return strReturn; 
}
