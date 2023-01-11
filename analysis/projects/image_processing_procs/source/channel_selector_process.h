#ifndef CHANNEL_SELECTOR_PROCESS_H
#define CHANNEL_SELECTOR_PROCESS_H
/******************************************************************************
Channel_Selector_Process.h

begin		: 2007-06-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_image_processing_procs.h"

// pyxlib includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"



/*!
Allows data to be visualized from a coverage that outputs multiple channels of data.
By selecting a different channel of data, data from that channel of this process's 
input will be visaulized. This is to be used when visualizating data with a metadata
field count greater then zero. While this process can be used with an input process
that only generates one field of data it then becomes redundat.
*/
//! Allows a channel to be visualized from a multi-channel input.
class MODULE_IMAGE_PROCESSING_PROCS_DECL ChannelSelectorProcess :
	 public ProcessImpl<ChannelSelectorProcess>, public CoverageBase
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

	IUNKNOWN_DEFAULT_CAST( ChannelSelectorProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output type of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	//! Get the output type of this process.
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

	//! Get the attributes associated with this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes associated with this process.
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
		  "</xs:restriction>"
		"</xs:simpleType>"
		  "<xs:element name=\"ChannelSelectorProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Channel_Selected\" type=\"channelType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Channel Selected</friendlyName>"
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

	//! Get a coverage value.
	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;
public: // ChannelSelectorProcess

	//! Constants.
	static const std::string kstrSelectedChannel;

	ChannelSelectorProcess():m_nChannelSelected(0){;}

protected:
	virtual ~ChannelSelectorProcess(){;}

protected: //CoverageBase
	
	//! Create the coverage's geometry.
	virtual void createGeometry() const;

private:

	//! The selected channel. The channel which we ask our inputs for data at.
	int m_nChannelSelected;

	/*!
	 A pointer to the input coverage, to avoid Querying the 
	 interface everytime we want it.
	*/
	boost::intrusive_ptr<ICoverage> m_spInputCov;
};

#endif //end guard