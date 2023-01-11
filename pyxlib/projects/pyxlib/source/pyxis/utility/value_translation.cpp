/******************************************************************************
value_translation.cpp

begin		: 2010-04-19
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/value_translation.h"

// local includes
#include "pyxis/utility/tester.h"

#pragma warning(disable: 4800)


template<typename T_INPUT,typename T_OUTPUT>
class SpecificCastValueTranslator : public CastValueTranslator
{
protected:
	PYXFieldDefinition m_inputDefinition;
	PYXFieldDefinition m_outputDefinition;

public:
	virtual const PYXFieldDefinition & getInputDefinition() const
	{
		return m_inputDefinition;
	}

	virtual PYXFieldDefinition & getInputDefinition()
	{
		return m_inputDefinition;
	}

	virtual const PYXFieldDefinition & getOutputDefinition() const
	{
		return m_outputDefinition;
	}

	virtual PYXFieldDefinition & getOutputDefinition()
	{
		return m_outputDefinition;
	}

	virtual PYXValue translate(const PYXValue & inputValue) const
	{
		PYXValue result = m_outputDefinition.getTypeCompatibleValue();
		T_INPUT inValue;
		inputValue.get(inValue);
		result.set(static_cast<T_OUTPUT>(inValue));
		return result;
	}

	virtual void translate(const PYXValue * inputValue,PYXValue * outputValue) const
	{
		T_INPUT inValue;
		inputValue->get(inValue);
		outputValue->set(static_cast<T_OUTPUT>(inValue));
	}

	virtual PYXValue translateBack(const PYXValue & outputValue) const
	{
		PYXValue result = m_inputDefinition.getTypeCompatibleValue();
		T_OUTPUT outValue;
		outputValue.get(outValue);
		result.set(static_cast<T_INPUT>(outValue));
		return result;
	}

	virtual void translateBack(PYXValue * inputValue,const PYXValue * outputValue) const
	{
		T_OUTPUT outValue;
		outputValue->get(outValue);
		inputValue->set(static_cast<T_INPUT>(outValue));
	}

public:	
	SpecificCastValueTranslator(const PYXFieldDefinition & inputDefinition,const PYXFieldDefinition & outputDefinition) : 
		m_inputDefinition(inputDefinition),
		m_outputDefinition(outputDefinition)
	{
	}

	virtual ~SpecificCastValueTranslator ()
	{
	}
};


PYXPointer<ISingleValueTranslator> CastValueTranslator::create(const PYXFieldDefinition & inputDefinition,PYXValue::eType outputType)
{
	PYXFieldDefinition outputDefinition(inputDefinition.getName(),inputDefinition.getContext(),outputType);

	switch(inputDefinition.getType())
	{
		case PYXValue::knUInt8:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint8_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;
							
		case PYXValue::knUInt16:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint16_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knUInt32:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<uint32_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knInt8:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int8_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knInt16:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int16_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knInt32:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<int32_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knBool:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<bool,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knChar:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<char,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knFloat:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<float,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knDouble:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificCastValueTranslator<double,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"cast to String is not supported");
			}
			break;

		case PYXValue::knString:
			PYXTHROW(PYXException,"cast to String is not supported");
	}

	PYXTHROW(PYXException,"Failed to create CastValueTransaltor");
}



template<typename T_INPUT,typename T_OUTPUT>
class SpecificReinterpretValueTranslator : public ReinterpretValueTranslator 
{
protected:
	PYXFieldDefinition m_inputDefinition;
	PYXFieldDefinition m_outputDefinition;

public:
	virtual const PYXFieldDefinition & getInputDefinition() const
	{
		return m_inputDefinition;
	}

	virtual PYXFieldDefinition & getInputDefinition()
	{
		return m_inputDefinition;
	}

	virtual const PYXFieldDefinition & getOutputDefinition() const
	{
		return m_outputDefinition;
	}

	virtual PYXFieldDefinition & getOutputDefinition()
	{
		return m_outputDefinition;
	}

	virtual PYXValue translate(const PYXValue & inputValue) const
	{
		PYXValue result = m_outputDefinition.getTypeCompatibleValue();
		T_INPUT inValue;
		inputValue.get(inValue);
		result.set(*(reinterpret_cast<T_OUTPUT*>(&inValue)));
		return result;
	}

	virtual void translate(const PYXValue * inputValue,PYXValue * outputValue) const
	{
		T_INPUT inValue;
		inputValue->get(inValue);
		outputValue->set(*(reinterpret_cast<T_OUTPUT*>(&inValue)));
	}

	virtual PYXValue translateBack(const PYXValue & outputValue) const
	{
		PYXValue result = m_inputDefinition.getTypeCompatibleValue();
		T_OUTPUT outValue;
		outputValue.get(outValue);
		result.set(*(reinterpret_cast<T_INPUT*>(&outValue)));
		return result;
	}

	virtual void translateBack(PYXValue * inputValue,const PYXValue * outputValue) const
	{
		T_OUTPUT outValue;
		outputValue->get(outValue);
		inputValue->set(*(reinterpret_cast<T_INPUT*>(&outValue)));
	}

public:
	
	SpecificReinterpretValueTranslator(const PYXFieldDefinition & inputDefinition,const PYXFieldDefinition & outputDefinition) : 
		m_inputDefinition(inputDefinition),
		m_outputDefinition(outputDefinition)
	{
	}

	virtual ~SpecificReinterpretValueTranslator()
	{
	}
};


PYXPointer<ISingleValueTranslator> ReinterpretValueTranslator::create(const PYXFieldDefinition & inputDefinition,PYXValue::eType outputType)
{
	PYXFieldDefinition outputDefinition(inputDefinition.getName(),inputDefinition.getContext(),outputType);

	switch(inputDefinition.getType())
	{
		case PYXValue::knUInt8:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint8_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;
							
		case PYXValue::knUInt16:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint16_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knUInt32:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<uint32_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knInt8:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int8_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knInt16:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int16_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knInt32:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<int32_t,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knBool:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<bool,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knChar:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<char,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knFloat:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<float,double>(inputDefinition,outputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knDouble:
			switch(outputType)
			{
				case PYXValue::knUInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,uint8_t>(inputDefinition,outputDefinition),false);
									
				case PYXValue::knUInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,uint16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knUInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,uint32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt8:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,int8_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt16:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,int16_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knInt32:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,int32_t>(inputDefinition,outputDefinition),false);

				case PYXValue::knBool:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,bool>(inputDefinition,outputDefinition),false);

				case PYXValue::knChar:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,char>(inputDefinition,outputDefinition),false);

				case PYXValue::knFloat:
					return PYXPointer<ISingleValueTranslator>(new SpecificReinterpretValueTranslator<double,float>(inputDefinition,outputDefinition),false);

				case PYXValue::knDouble:
					return PYXPointer<ISingleValueTranslator>(new NoChangeValueTranslator(inputDefinition),false);

				case PYXValue::knString:
					PYXTHROW(PYXException,"reinterpret  to String is not supported");
			}
			break;

		case PYXValue::knString:
			PYXTHROW(PYXException,"reinterpret to String is not supported");
	}

	PYXTHROW(PYXException,"Failed to create ReinterpretValueTranslator");
}
