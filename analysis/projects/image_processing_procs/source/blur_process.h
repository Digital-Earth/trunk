#ifndef BLUR_PROCESS_H
#define BLUR_PROCESS_H
/******************************************************************************
Blur_Process

begin		: 2006-08-22
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_image_processing_procs.h"

//pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

/*!
BlurProcess, is a filter which implements a blur algorithm This process takes a 
single channel coverage as input and turns it into n channels creating a multi 
channel datasource. Each channel of this coverage is at a different sdpectrum
resolution. The changes in resolution are accomplished through a series of daisy
chained, zoom in and zoom out filters, along with cache filters at appropriate 
places to increase performance.
*/
//! Blurs the input coverage into multiple channels each channel more blurred then the last.
class MODULE_IMAGE_PROCESSING_PROCS_DECL BlurProcess : public ProcessImpl<BlurProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public: //PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( BlurProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const
	{
		return m_spCovDefn;
	}

	//! Get the field definitions for the coverage in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition()
	{
		return m_spCovDefn;
	}

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const
	{
		return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"channelType\">"
			"<xs:restriction base=\"xs:int\">"
				"<xs:minInclusive value=\"0\" />"
				"<xs:maxExclusive value=\"40\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"BlurProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Number_Of_Steps\" type=\"channelType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Number of Steps</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;
public:
	
	//! Constants
	static const std::string kstrSteps;
	
	//! Unit test method
	static void test();

	//! Default constructor.
	BlurProcess() : m_nSteps(1){;}

protected:
	//! Destructor.
	virtual ~BlurProcess(){;}
	
protected: //CoverageBase

	//! Create the geometry of this coverage.
	void createGeometry() const;

protected:

	//! Disable copy constructor
	BlurProcess(const BlurProcess&);

	//! Disable copy assignment
	void operator =(const BlurProcess&);

private:

	//! Creates meta data, representing this as multichannel coverage.
	void createMetaData();

	//! Creates the internal pipeline
	void setUpFilteringPipeline();

	//! A vector of output caches.
	std::vector<boost::intrusive_ptr<
		ICoverage> > m_vecOutputCaches;

	//! The number of steps, times to zoom out & in.
	int m_nSteps;

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spInputCoverage;

};

#endif	// end guard
