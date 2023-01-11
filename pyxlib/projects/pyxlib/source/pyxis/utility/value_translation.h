#ifndef PYXIS__UTILITY__VALUE_TRANSLATION_H
#define PYXIS__UTILITY__VALUE_TRANSLATION_H
/******************************************************************************
xy_coverage_translator.cpp

begin		: 2010-04-19
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/object.h"
#include "pyxis/data/record.h"


class PYXLIB_DECL ISingleValueTranslator : public PYXObject
{
public:
	virtual const PYXFieldDefinition & getInputDefinition() const = 0;

	virtual PYXFieldDefinition & getInputDefinition() = 0;

	virtual const PYXFieldDefinition & getOutputDefinition() const = 0;

	virtual PYXFieldDefinition & getOutputDefinition() = 0;

	virtual PYXValue translate(const PYXValue & inputValue) const = 0;

	virtual void translate(const PYXValue * inputValue,PYXValue * outputValue) const = 0;

	virtual PYXValue translateBack(const PYXValue & outputValue) const = 0;

	virtual void translateBack(PYXValue * inputValue,const PYXValue * outputValue) const = 0;

protected:
	ISingleValueTranslator()
	{
	}

	virtual ~ISingleValueTranslator()
	{
	}
};

class PYXLIB_DECL NoChangeValueTranslator : public ISingleValueTranslator
{
protected:
	PYXFieldDefinition m_definition;

public:
	virtual const PYXFieldDefinition & getInputDefinition() const
	{
		return m_definition;
	}

	virtual PYXFieldDefinition & getInputDefinition()
	{
		return m_definition;
	}

	virtual const PYXFieldDefinition & getOutputDefinition() const
	{
		return m_definition;
	}

	virtual PYXFieldDefinition & getOutputDefinition()
	{
		return m_definition;
	}

	virtual PYXValue translate(const PYXValue & inputValue) const
	{
		return inputValue;
	}

	virtual void translate(const PYXValue * inputValue,PYXValue * outputValue) const
	{
		*outputValue = *inputValue;
	}

	virtual PYXValue translateBack(const PYXValue & outputValue) const
	{
		return outputValue;
	}

	virtual void translateBack(PYXValue * inputValue,const PYXValue * outputValue) const
	{
		*inputValue = *outputValue;
	}

public:
	static PYXPointer<NoChangeValueTranslator> create(const PYXFieldDefinition & definition)
	{
		return PYXNEW(NoChangeValueTranslator,definition);
	}

	NoChangeValueTranslator(const PYXFieldDefinition & definition) : m_definition(definition)
	{
	}

	virtual ~NoChangeValueTranslator()
	{
	}
};

class PYXLIB_DECL CastValueTranslator : public ISingleValueTranslator
{
public:
	static PYXPointer<ISingleValueTranslator> create(const PYXFieldDefinition & inputDefinition,PYXValue::eType outputType);	
};

class PYXLIB_DECL ReinterpretValueTranslator : public ISingleValueTranslator
{
public:
	static PYXPointer<ISingleValueTranslator> create(const PYXFieldDefinition & inputDefinition,PYXValue::eType outputType);
};


class PYXLIB_DECL IDefinitionTranslator : public PYXObject
{
public:
	virtual PYXPointer<const PYXTableDefinition> getInputDefinition() const = 0;

	virtual PYXPointer<PYXTableDefinition> getInputDefinition() = 0;

	virtual PYXPointer<const PYXTableDefinition> getOutputDefinition() const = 0;

	virtual PYXPointer<PYXTableDefinition> getOutputDefinition() = 0;

	//! translate the input value to the output value
	virtual PYXValue translateFieldByIndex(int nFieldIndex, const PYXValue & value) = 0;

	//! translate the input value to the output value using a given PYXValue objects
	virtual void translateFieldByIndex(int nFieldIndex, const PYXValue * inputValue, PYXValue * outputValue) = 0;

	//! translate the input value to the output value
	virtual PYXValue translateFieldByName(const std::string& strName, const PYXValue & value) = 0;

	//! translate all fields
	virtual std::vector<PYXValue> translateFields(const std::vector<PYXValue> & values) = 0;

	//! translate the output value back to the input value 
	virtual PYXValue translateBack(int nFieldIndex, const PYXValue & value) = 0;

	//! translate the output value back to the input value using a given PYXValue objects
	virtual void translateBackFieldByIndex(int nFieldIndex, PYXValue * inputValue, const PYXValue * outputValue) = 0;

	//! translate the input value to the output value
	virtual PYXValue translateBackFieldByName(const std::string& strName, const PYXValue & value) = 0;

	//! translate back all fields
	virtual std::vector<PYXValue> translateBackFields(const std::vector<PYXValue> & values) = 0;

protected:
	IDefinitionTranslator()
	{
	}

	virtual ~IDefinitionTranslator()
	{
	}
};

class PYXLIB_DECL SimpleDefinitionTranslator : public IDefinitionTranslator 
{
protected:
	PYXPointer<PYXTableDefinition> m_inputDefinition;
	PYXPointer<PYXTableDefinition> m_outputDefinition;
	std::vector<PYXPointer<ISingleValueTranslator>> m_valueTranslators;

public:	
	virtual PYXPointer<const PYXTableDefinition> getInputDefinition() const
	{
		return m_inputDefinition;
	}

	virtual PYXPointer<PYXTableDefinition> getInputDefinition()
	{
		return m_inputDefinition;
	}

	virtual PYXPointer<const PYXTableDefinition> getOutputDefinition() const
	{
		return m_outputDefinition;
	}

	virtual PYXPointer<PYXTableDefinition> getOutputDefinition()
	{
		return m_outputDefinition;
	}

	//! translate the input value to the output value
	virtual PYXValue translateFieldByIndex(int nFieldIndex, const PYXValue & value)
	{
		return m_valueTranslators[nFieldIndex]->translate(value);
	}

	//! translate the input value to the output value using a given PYXValue objects
	virtual void translateFieldByIndex(int nFieldIndex, const PYXValue * inputValue, PYXValue * outputValue)
	{	
		m_valueTranslators[nFieldIndex]->translate(inputValue,outputValue);
	}

	//! translate the input value to the output value
	virtual PYXValue translateFieldByName(const std::string& strName, const PYXValue & value)
	{	
		return m_valueTranslators[m_inputDefinition->getFieldIndex(strName)]->translate(value);
	}


	//! translate all fields
	virtual std::vector<PYXValue> translateFields(const std::vector<PYXValue> & values)
	{
		std::vector<PYXValue> result;

		for (size_t i=0;i<m_valueTranslators.size();i++)
		{
			result.push_back(m_valueTranslators[i]->translate(values[i]));
		}

		return result;
	}

	//! translate the output value back to the input value 
	virtual PYXValue translateBack(int nFieldIndex, const PYXValue & value)
	{
		return m_valueTranslators[nFieldIndex]->translateBack(value);
	}

	//! translate the output value back to the input value using a given PYXValue objects
	virtual void translateBackFieldByIndex(int nFieldIndex, PYXValue * inputValue, const PYXValue * outputValue)
	{
		m_valueTranslators[nFieldIndex]->translateBack(inputValue,outputValue);
	}

	//! translate the input value to the output value
	virtual PYXValue translateBackFieldByName(const std::string& strName, const PYXValue & value)
	{
		return m_valueTranslators[m_outputDefinition->getFieldIndex(strName)]->translateBack(value);
	}

	//! translate back all fields
	virtual std::vector<PYXValue> translateBackFields(const std::vector<PYXValue> & values)
	{
		std::vector<PYXValue> result;

		for (size_t i=0;i<m_valueTranslators.size();i++)
		{
			result.push_back(m_valueTranslators[i]->translate(values[i]));
		}

		return result;
	}

public:
	static PYXPointer<SimpleDefinitionTranslator> create(std::vector<PYXPointer<ISingleValueTranslator>> & translators) 
	{
		return PYXNEW(SimpleDefinitionTranslator,translators);
	}

	SimpleDefinitionTranslator(std::vector<PYXPointer<ISingleValueTranslator>> & translators) : m_valueTranslators(translators)
	{
		m_inputDefinition = PYXTableDefinition::create();
		m_outputDefinition = PYXTableDefinition::create();

		for (size_t i=0;i<m_valueTranslators.size();i++)
		{
			m_inputDefinition->addFieldDefinition(m_valueTranslators[i]->getInputDefinition());
			m_outputDefinition->addFieldDefinition(m_valueTranslators[i]->getOutputDefinition());
		}
	}

	virtual ~SimpleDefinitionTranslator()
	{
	}
};


#endif // guard
