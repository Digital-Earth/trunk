/******************************************************************************
channel_selector_process.cpp

begin		: 2007-06-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "channel_selector_process.h"

// pyxlib includes.
#include "pyxis/data/value_tile.h"
#include "pyxis/procs/exceptions.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"

// {16B34600-CB76-4613-B3B4-7D1B3C4FB499}
PYXCOM_DEFINE_CLSID(ChannelSelectorProcess,
0x16b34600, 0xcb76, 0x4613, 0xb3, 0xb4, 0x7d, 0x1b, 0x3c, 0x4f, 0xb4, 0x99);

PYXCOM_CLASS_INTERFACES(
	ChannelSelectorProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ChannelSelectorProcess, "Channel Selector", "Selects which channel to view from input.", "Utility",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Multi-Channel input", "The process to select a field or channel from.")
IPROCESS_SPEC_END

const std::string ChannelSelectorProcess::kstrSelectedChannel = "Channel_Selected";

/*! 
Gets the attributes that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key) 
to string(values) and the map is retuned to be written out.

\return A STL map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> ChannelSelectorProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib.clear();
	attrib[kstrSelectedChannel] = intToString(m_nChannelSelected, 0);
	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when
the attributes have been altered in the pipe editor. The stl map
passed in is searched for key-value pairs and the values are parsed
out into their respective variables.

\param mapAttr A map of the attributes to be set in this process.
*/
void ChannelSelectorProcess::setAttributes(const std::map<std::string, std::string> &mapAttr)
{	
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kstrSelectedChannel);
	if (it != mapAttr.end())
	{ 
		m_nChannelSelected = atoi(const_cast<char*> (it->second.c_str()));
	}
	else
	{
		m_nChannelSelected = 0;
	}
}

PYXValue ChannelSelectorProcess::getCoverageValue(const PYXIcosIndex &index, int nFieldIndex) const
{
	if (nFieldIndex != 0)
	{
		PYXTHROW(PYXException, "Channel '" << nFieldIndex << "' invalid, the selected channel must be 0");
	}

	return m_spInputCov->getCoverageValue(index, m_nChannelSelected);
}

PYXPointer<PYXValueTile> ChannelSelectorProcess::getFieldTile(	const PYXIcosIndex& index,
																int nRes,
																int nFieldIndex	) const
{
	if (nFieldIndex != 0)
	{
		PYXTHROW(PYXException, "Channel '" << nFieldIndex << "' invalid, the selected channel must be 0");
	}

	return m_spInputCov->getFieldTile(index, nRes, m_nChannelSelected);
}

PYXCost STDMETHODCALLTYPE ChannelSelectorProcess::getFieldTileCost(	const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex ) const
{	
	if (nFieldIndex != 0)
	{
		PYXTHROW(PYXException, "Channel '" << nFieldIndex << "' invalid, the selected channel must be 0");
	}

	return m_spInputCov->getFieldTileCost(index,nRes,m_nChannelSelected);
}

IProcess::eInitStatus ChannelSelectorProcess::initImpl()
{
	m_strID = "Channel Selector: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	m_spInputCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	if (!m_spInputCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not acquire the input coverage.");
		return knFailedToInit;
	}

	// verify that the channel count is within the limits
	if (!m_spInputCov->getCoverageDefinition())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not acquire input coverage definition.");
		return knFailedToInit;
	}
	if (m_nChannelSelected < 0 ||
		m_spInputCov->getCoverageDefinition()->getFieldCount() < m_nChannelSelected)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("The selected field is not valid for the input coverage");
		return knFailedToInit;
	}

	// build the coverage definition based on the input field only
	m_spCovDefn = PYXTableDefinition::create();
	m_spCovDefn->addFieldDefinition(
		m_spInputCov->getCoverageDefinition()->getFieldDefinition(m_nChannelSelected));
	return knInitialized;
}

/*!
Creates the geometry for this coverage. Since this process
is a pass through process. The geometry of our input is
taken as our own geometry.
*/
void ChannelSelectorProcess::createGeometry() const
{
	assert(m_spInputCov && "Input coverage not initalized properly.");
	m_spGeom = m_spInputCov->getGeometry()->clone();
}
