/******************************************************************************
constant_record.cpp

begin		: 2012-08-22
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/constant_record.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_utils.h"

//////////////////////////////////////////////////////////////////////////
// ConstantRecord
//////////////////////////////////////////////////////////////////////////

PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE ConstantRecord::getDefinition() const
{
	return m_definition;
}

PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE ConstantRecord::getDefinition()
{
	return m_definition;
}

PYXValue STDMETHODCALLTYPE ConstantRecord::getFieldValue( int nFieldIndex ) const
{
	return m_values[nFieldIndex];
}

void STDMETHODCALLTYPE ConstantRecord::setFieldValue( PYXValue value, int nFieldIndex )
{
	PYXTHROW(PYXException,"Can not change value of a constant record");
}

PYXValue STDMETHODCALLTYPE ConstantRecord::getFieldValueByName( const std::string& strName ) const
{
	int nField = getDefinition()->getFieldIndex(strName); 
	if (0 <= nField) 
	{ 
		return getFieldValue(nField); 
	} 
	return PYXValue();
}

void STDMETHODCALLTYPE ConstantRecord::setFieldValueByName( PYXValue value, const std::string& strName )
{
	PYXTHROW(PYXException,"Can not change value of a constant record");
}

std::vector<PYXValue> STDMETHODCALLTYPE ConstantRecord::getFieldValues() const
{
	return m_values;
}

void STDMETHODCALLTYPE ConstantRecord::setFieldValues( const std::vector<PYXValue>& vecValues )
{
	PYXTHROW(PYXException,"Can not change values of a constant record");
}

void STDMETHODCALLTYPE ConstantRecord::addField( const std::string& strName, PYXFieldDefinition::eContextType nContext, PYXValue::eType nType, int nCount /*= 1*/, PYXValue value /*= PYXValue( ) */ )
{
	PYXTHROW(PYXException,"Can not add field to a constant record");
}

