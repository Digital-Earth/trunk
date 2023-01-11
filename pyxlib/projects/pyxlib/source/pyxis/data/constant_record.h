#ifndef PYXIS__DATA__CONSTANT_RECORD_H
#define PYXIS__DATA__CONSTANT_RECORD_H
/******************************************************************************
constant_record.h

begin		: 2012-08-22
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/record.h"

#include <boost/function.hpp>

// standard includes
#include <string>

class PYXLIB_DECL ConstantRecord : public IRecord
{
public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	ConstantRecord(PYXPointer<PYXTableDefinition> definition,boost::function<PYXValue(int)> valueGenerator):
		m_definition(definition)
	{
		for(int i=0;i<m_definition->getFieldCount();++i)
		{
			m_values.push_back(valueGenerator(i));
		}
	}

	ConstantRecord(PYXPointer<PYXTableDefinition> definition,std::vector<PYXValue> values):
		m_definition(definition),
		m_values(values)
	{
	}

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition();

	virtual PYXValue STDMETHODCALLTYPE getFieldValue( int nFieldIndex ) const;

	virtual void STDMETHODCALLTYPE setFieldValue( PYXValue value, int nFieldIndex );

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName( const std::string& strName ) const;

	virtual void STDMETHODCALLTYPE setFieldValueByName( PYXValue value, const std::string& strName );

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const;

	virtual void STDMETHODCALLTYPE setFieldValues( const std::vector<PYXValue>& vecValues );

	virtual void STDMETHODCALLTYPE addField( const std::string& strName, PYXFieldDefinition::eContextType nContext, PYXValue::eType nType, int nCount = 1, PYXValue value = PYXValue( ) );

private:
	PYXPointer<PYXTableDefinition> m_definition;
	std::vector<PYXValue> m_values;
};

#endif // guard
