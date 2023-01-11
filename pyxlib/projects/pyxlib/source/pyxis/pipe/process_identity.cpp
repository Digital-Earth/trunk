/******************************************************************************
process_identity.cpp

begin		: 2008-06-02
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process_identity.h"

// pyxlib includes
#include "pyxis/pipe/parameter.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_utils.h"

//! Tester class
Tester<ProcessIdentity> gTester;

//! Test method
void ProcessIdentity::test()
{
	// TODO: Add tests.
}

ProcessIdentity::ProcessIdentity(const IID& clsID) :
	m_strXMLClassID(XMLUtils::toSafeXMLText(guidToStr(clsID)))
{
}

ProcessIdentity::~ProcessIdentity()
{
}

void ProcessIdentity::setData(const std::string& strData)
{
	m_strXMLData = XMLUtils::toSafeXMLText(strData);
}

void ProcessIdentity::setAttributes(
	const std::map<std::string, std::string>& mapAttributes)
{
	std::string strXMLAttributes;
	if (!mapAttributes.empty())
	{
		const std::map<std::string, std::string>::const_iterator itEnd = mapAttributes.end();
		for (std::map<std::string, std::string>::const_iterator it = mapAttributes.begin();
			it != itEnd; ++it)
		{
			std::string strXMLAttribute = "<attribute name=\"";
			strXMLAttribute += XMLUtils::toSafeXMLText(it->first, true);
			strXMLAttribute += "\">";
			strXMLAttribute += XMLUtils::toSafeXMLText(it->second);
			strXMLAttribute += "</attribute>\n";

			strXMLAttributes += strXMLAttribute;
		}
	}
	m_strXMLAttributes = strXMLAttributes;
}

/*!
Sets the checksum, if it is not empty, such that it may be included in the xml string 
for the identity. 
param strData	The checksum to be included in the identity. 
*/
void ProcessIdentity::setChecksum(const std::string& strData)
{
	if (!strData.empty())
	{
		m_strChecksum ="<checksum type=\"SHA256\">" + strData + "</checksum>\n";
	}
}

void ProcessIdentity::addInput(const Parameter& paramInput, bool bIsValueOrderSignificant)
{
	std::vector<std::string> vecValues;
	const int nValueCount = paramInput.getValueCount();
	for (int nv = 0; nv < nValueCount; ++nv)
	{
		boost::intrusive_ptr<IProcess> spValue = paramInput.getValue(nv);
		assert(spValue);

		vecValues.push_back(spValue->getIdentity());
	}

	if (!vecValues.empty())
	{
		if (!bIsValueOrderSignificant)
		{
			// Sort the values alphabetically.
			std::sort(vecValues.begin(), vecValues.end());
		}

		std::string strXMLValues;
		const std::vector<std::string>::const_iterator itEnd = vecValues.end();
		for (std::vector<std::string>::const_iterator it = vecValues.begin();
			it != itEnd; ++it)
		{
			strXMLValues += *it;
		}

		if (!strXMLValues.empty())
		{
			std::string strXMLInput = "<input>\n";
			strXMLInput += strXMLValues;
			strXMLInput += "</input>\n";

			m_strXMLInputs += strXMLInput;
		}
	}
}

std::string ProcessIdentity::operator ()()
{
	std::string strXML;

	strXML += "<identity clsID=\"";
	strXML += m_strXMLClassID;
	strXML += "\">\n";

	if (!m_strXMLData.empty())
	{
		strXML += "<data>\n";
		strXML += m_strXMLData;
		strXML += "</data>\n";
	}

	if (!m_strXMLAttributes.empty())
	{
		strXML += "<attributes>\n";
		strXML += m_strXMLAttributes;
		strXML += "</attributes>\n";
	}

	if (!m_strXMLInputs.empty())
	{
		strXML += "<inputs>\n";
		strXML += m_strXMLInputs;
		strXML += "</inputs>\n";
	}

	if (!m_strChecksum.empty())
	{
		strXML += m_strChecksum;
	}

	strXML += "</identity>\n";

	return strXML;
}
