/******************************************************************************
process.cpp

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// {20FA48A2-ACCC-4c15-A2FB-05F9805DDE74}
PYXCOM_DEFINE_IID(IProcess, 
0x20fa48a2, 0xaccc, 0x4c15, 0xa2, 0xfb, 0x5, 0xf9, 0x80, 0x5d, 0xde, 0x74);

namespace
{

struct Testee
{
static void test()
{
	{
		ProcRef procref;
		TEST_ASSERT(StringUtils::toString(procref) == "{00000000-0000-0000-0000-000000000000}[0]");

		GUID procid = strToGuid("{12345678-ABCD-1234-ABCD-12345678ABCD}");
		int nProcVer = 1234;

		procref = ProcRef(procid, nProcVer);
		TEST_ASSERT(StringUtils::toString(procref) == "{12345678-ABCD-1234-ABCD-12345678ABCD}[1234]");

		procref = ProcRef();
		std::istringstream in("{12345678-ABCD-1234-ABCD-12345678ABCD}[1234]");
		in >> procref;
		TEST_ASSERT(procref.getProcID() == procid && procref.getProcVersion() == nProcVer);
	}
}
};

Tester<Testee> gTester;

}

bool PYXLIB_DECL verifySpec(boost::intrusive_ptr<IProcess> spProc)
{
	if (!spProc)
	{
		TRACE_ERROR("Can not verify spec on a null process, returning false.");
		return false;
	}
	TRACE_DEBUG("Examining specification for process: " << spProc->getProcName());

	PYXPointer<const ProcessSpec> spSpec = spProc->getSpec();
	assert(spSpec);

	// look at each parameter in the spec and compare to the actual
	for (int nParam = 0; nParam < spSpec->getParameterCount(); ++nParam)
	{
		PYXPointer<const ParameterSpec> spParamSpec = spSpec->getParameter(nParam);		
		PYXPointer<const Parameter> spParam = spProc->getParameter(nParam);

		// verify the value counts
		if (spParam->getValueCount() < spParamSpec->getMinOccurs())
		{
			TRACE_DEBUG("Too few inputs for parameter " << 
				spParamSpec->getName() << " in " << spProc);
			return false;
		}
		else if (	spParamSpec->getMaxOccurs() != -1 && 
					spParamSpec->getMaxOccurs() < spParam->getValueCount()	)
		{
			TRACE_DEBUG("Too many inputs for parameter " << 
				spParamSpec->getName() << " in " << spProc);
			return false;
		}

		// verify the interfaces of the values
		IID interfaceID = spParamSpec->getInterface();
		for (int nValue = 0; nValue < spParam->getValueCount(); ++nValue)
		{
			if (!spParam->getValue(nValue)->getSpec()->providesOutputType(interfaceID))
			{
				TRACE_DEBUG("Parameter value '" << nValue << 
					"' does not have an appropriate output type for parameter " <<
					spParamSpec->getName() << " in process " << spProc->getProcName());
				return false;
			}
		}
	}

	// all counts and interfaces are correct
	TRACE_DEBUG("This process meets its specification: " << spProc->getProcName());
	return true;
}


std::string ProcessProcessingEvent::Error = "Error";
std::string ProcessProcessingEvent::Processing = "Processing";
std::string ProcessProcessingEvent::Fetching = "Fetching";
std::string ProcessProcessingEvent::Downloading = "Downloading";



// boost includes
#include <boost/thread/recursive_mutex.hpp>

ProcessLifeTimeIndication::ProcessLifeTimeIndication(const IProcess & process)
{
    m_name = process.getProcName();
    m_procRef = ProcRef(process.getProcID(),process.getProcVersion());
    m_spec = process.getSpec();
}

const std::string & ProcessLifeTimeIndication::getProcName() const
{
    return m_name;
}

const ProcRef & ProcessLifeTimeIndication::getProcRef() const
{
    return m_procRef;
}

const PYXPointer<ProcessSpec> & ProcessLifeTimeIndication::getProcSpec() const
{
    return m_spec;
}




ProcessLifeTimeTracker & ProcessLifeTimeTracker::getInstance()
{
    static ProcessLifeTimeTracker s_trackerInstance;
    return s_trackerInstance;
}

void ProcessLifeTimeTracker::notifyProcessCreated(const IProcess & process) 
{
    boost::recursive_mutex::scoped_lock lock(m_mutex);
    m_trackedProcesses.insert(&process);
}

void ProcessLifeTimeTracker::notifyProcessDestoryed(const IProcess & process)
{
    boost::recursive_mutex::scoped_lock lock(m_mutex);
    m_trackedProcesses.erase(&process);    
}

int ProcessLifeTimeTracker::getTrackedProcessCount()
{
    boost::recursive_mutex::scoped_lock lock(m_mutex);
    return (int)m_trackedProcesses.size();
}
	
std::vector<PYXPointer<ProcessLifeTimeIndication>> ProcessLifeTimeTracker::getTrackedProcesses()
{
    boost::recursive_mutex::scoped_lock lock(m_mutex);
    std::vector<PYXPointer<ProcessLifeTimeIndication>> result;
    
    for(auto & process : m_trackedProcesses) {
        result.push_back(ProcessLifeTimeIndication::create(*process));
    }
    
    return result;
}