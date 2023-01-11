#ifndef FEATURE_PROCESSING_PROCS__CONSTANT_FEATURE_CALCULATOR_H
#define FEATURE_PROCESSING_PROCS__CONSTANT_FEATURE_CALCULATOR_H
/******************************************************************************
constant_feature_calculator.h

begin      : 08/20/2012 4:42:06 PM 
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/value.h"
#include "pyxis/procs/feature_calculator.h"

class MODULE_FEATURE_PROCESSING_PROCS_DECL ConstantFieldCalculator :  public ProcessImpl<ConstantFieldCalculator>, IFeatureCalculator 
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCalculator)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown const > STDMETHODCALLTYPE getOutput() const
	{
		return static_cast< IFeatureCalculator const * >(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr< PYXCOM_IUnknown > STDMETHODCALLTYPE getOutput()
	{
		return static_cast< IFeatureCalculator * >(this);
	}

	//! Get the schema to describe how the attributes should be edited.
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	/*!
	Get the attributes associated with  this process. 

	\return a map of standard string - standard string containing the attributes to be serialized.
	*/
	virtual std::map< std::string, std::string > STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes of this process. 
	virtual void STDMETHODCALLTYPE setAttributes(std::map< std::string, std::string > const & mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCalculator

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getOutputDefinition() const;

	virtual boost::intrusive_ptr<IRecord> STDMETHODCALLTYPE calculateValues( boost::intrusive_ptr<IFeature> spFeature ) const;

	virtual PYXValue STDMETHODCALLTYPE calculateValue( boost::intrusive_ptr<IFeature> spFeature,int fieldIndex ) const;


public: //XYBoundsRegionProc

	ConstantFieldCalculator();

// Members
private:

	std::string m_attributeName;
	PYXValue m_attributeValue;

	PYXPointer<PYXTableDefinition> m_outputDefinition;
	boost::intrusive_ptr<IRecord> m_outputRecord;

};

#endif
