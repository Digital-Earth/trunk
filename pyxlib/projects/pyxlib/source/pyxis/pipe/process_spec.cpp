/******************************************************************************
process_spec.cpp

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process_spec.h"

#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/sxs.h"

void ProcessSpec::write(std::ostream& out, const ProcessSpec& procSpec)
{
	boost::intrusive_ptr<ISXS> spObject = new DefaultSXSObject();
	spObject->setSXSName("proc_spec");
	
	spObject->setSXSAttribute("clsid", guidToStr(procSpec.getClass()));
	spObject->setSXSAttribute("name", procSpec.getName());
	spObject->setSXSAttribute("desc", procSpec.getDescription());

	// write out each of the parameters
	for (int nParam = 0; nParam != procSpec.getParameterCount(); ++nParam)
	{
		PYXPointer<ParameterSpec> spParamSpec = procSpec.getParameter(nParam);
		boost::intrusive_ptr<ISXS> spChild = new DefaultSXSObject();
		spChild->setSXSName("param");
		spChild->setSXSAttribute("iid", guidToStr(spParamSpec->getInterface()));
		spChild->setSXSAttribute("min", StringUtils::toString(spParamSpec->getMinOccurs()));
		spChild->setSXSAttribute("max", StringUtils::toString(spParamSpec->getMaxOccurs()));
		spChild->setSXSAttribute("name", spParamSpec->getName());
		spChild->setSXSAttribute("desc", spParamSpec->getDescription());
		spObject->addSXSChild(spChild);
	}

	// write out each of the supported output interfaces
	std::vector<IID> vecOutputIID = procSpec.getOutputInterfaces();
	for (size_t nInterface = 0; nInterface < vecOutputIID.size(); ++nInterface)
	{
		boost::intrusive_ptr<ISXS> spChild = new DefaultSXSObject();
		spChild->setSXSName("interface");
		spChild->setSXSAttribute("iid", guidToStr(vecOutputIID[nInterface]));
		spObject->addSXSChild(spChild);
	}
	SXSParser::getDefaultParser().writeObject(out, spObject);
}

std::string ProcessSpec::write(const ProcessSpec& procSpec)
{
	std::stringstream out;
	write(out, procSpec);
	return out.str();
}

void ProcessSpec::read(const std::string& xmlDefinition, ProcessSpec& procSpec)
{
	std::stringstream in(xmlDefinition);
	read(in, procSpec);
}

void ProcessSpec::read(std::istream& in, ProcessSpec& procSpec)
{
	boost::intrusive_ptr<ISXS> spObject;
	SXSParser::getDefaultParser().readObject(in, spObject);

	assert(spObject->getSXSName() == "proc_spec");
	assert(spObject->hasSXSAttribute("clsid"));
	assert(spObject->hasSXSAttribute("name"));
	assert(spObject->hasSXSAttribute("desc"));

	// write each of the parameters to the 
	std::vector<PYXPointer<ParameterSpec> > vecParamSpec;
	std::vector<IID> vecIID;
	for (int nChild = 0; nChild != spObject->getNumberOfSXSChildren(); ++nChild)
	{
		boost::intrusive_ptr<ISXS> spChild = spObject->getSXSChild(nChild);
		if (spChild->getSXSName() == "param")
		{
			assert(spChild->hasSXSAttribute("iid"));
			assert(spChild->hasSXSAttribute("min"));
			assert(spChild->hasSXSAttribute("max"));
			assert(spChild->hasSXSAttribute("name"));
			assert(spChild->hasSXSAttribute("desc"));
			vecParamSpec.push_back(ParameterSpec::create(
				strToGuid(spChild->getSXSAttribute("iid")),
				StringUtils::fromString<int>(spChild->getSXSAttribute("min")),
				StringUtils::fromString<int>(spChild->getSXSAttribute("max")),
				spChild->getSXSAttribute("name"),
				spChild->getSXSAttribute("desc")));
		}
		else if (spChild->getSXSName() == "interface")
		{
			assert(spChild->hasSXSAttribute("iid"));
			vecIID.push_back(strToGuid(spChild->getSXSAttribute("iid")));
		}
	}

	procSpec.setClass(strToGuid(spObject->getSXSAttribute("clsid")));
	procSpec.setParameterSpecs(vecParamSpec);
	assert(vecIID.size() != 0 && "No ouutput IID's specified for parameter spec");
	procSpec.setOutputIIDs(vecIID);
	procSpec.setName(spObject->getSXSAttribute("name"));
	procSpec.setDescription(spObject->getSXSAttribute("desc"));
}
