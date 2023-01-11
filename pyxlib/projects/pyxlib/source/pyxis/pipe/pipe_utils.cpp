/******************************************************************************
pipe_utils.cpp

begin		: 2007-03-09
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"

// pyxlib includes
#include "pyxis/pipe/exceptions.h"
#include "pyxis/pipe/parameter.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/pipe/pipe_utils.h"

#include "pyxis/utility/app_services.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/thread_pool.h"

#include "pyxis/procs/url.h"
#include "pyxis/procs/embedded_resource_holder.h"

// boost includes
#include <boost/filesystem/path.hpp>

// standard includes
#include <iostream>
#include <set>

namespace
{

typedef std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> > CloneMap;

/*!
Assumes this process is a root. Recursively removes all input processes
from the set. Does not remove this process from the set.

\param pSetProc		Collection of processes. (Must not be null.)
\param spProc		Process whose inputs will be pruned.
*/
void recursivePrune(std::set<boost::intrusive_ptr<IProcess> >* pSetProc, boost::intrusive_ptr<IProcess> spProc)
{
	for (int n = 0; n != spProc->getParameterCount(); ++n)
	{
		PYXPointer<Parameter> spParam = spProc->getParameter(n);
		for (int nv = 0; nv != spParam->getValueCount(); ++nv)
		{
			boost::intrusive_ptr<IProcess> spVal = spParam->getValue(nv);
			// NOTE subtle issue here. We are removing this process from the
			// set, but it *can't* be our current iterator because we don't
			// allow cycles in pipelines.
			pSetProc->erase(spVal);
			recursivePrune(pSetProc, spVal);
		}
	}
}

}

/*!
\param pVecProc		Collection of processes. (Must not be null.)
*/
void PipeUtils::pruneNonRoots(std::vector<boost::intrusive_ptr<IProcess> >* pVecProc)
{
	assert(pVecProc);

	std::set<boost::intrusive_ptr<IProcess> > setProc(pVecProc->begin(), pVecProc->end());

	for (std::set<boost::intrusive_ptr<IProcess> >::iterator it = setProc.begin();
		it != setProc.end(); ++it)
	{
		// NOTE subtle issue here. We have a current iterator which will
		// *not* be invalidated since we don't allow cycles in pipelines.
		recursivePrune(&setProc, *it);
	}

	pVecProc->assign(setProc.begin(), setProc.end());
}

/*!
The clone is a shallow copy of the original, not under process management. If
an init is attempted and fails, an exception is thrown.

\param spProc		The process to clone.
\return The clone of the process.
*/
boost::intrusive_ptr<IProcess> PipeUtils::cloneProcess(boost::intrusive_ptr<IProcess> spProc)
{
	assert(spProc);

	boost::intrusive_ptr<IProcess> spClone;
	PYXCOM_HRESULT hr = PYXCOMCreateInstance(spProc->getSpec()->getClass(), 0, IProcess::iid, (void**) &spClone);
	if (PYXCOM_FAILED(hr))
	{
		PYXTHROW(	PYXException,
					"Process clsid '" << spProc->getSpec()->getClass() << "' not found");
	}
	spClone->setFinalize(false);

	spClone->setProcID(spProc->getProcID());
	spClone->setProcVersion(spProc->getProcVersion());

	spClone->setProcName(spProc->getProcName());
	spClone->setProcDescription(spProc->getProcDescription());

	spClone->setAttributes(spProc->getAttributes());
	spClone->setData(spProc->getData());

	{
		std::vector<PYXPointer<Parameter> > vecParam;
		for (int nParam = 0; nParam != spProc->getParameterCount(); ++nParam)
		{
			PYXPointer<Parameter> spParam = spProc->getParameter(nParam);
			vecParam.push_back(Parameter::create(spParam->getSpec(), spParam->getValues()));
		}
		spClone->setParameters(vecParam);
	}

	return spClone;
}

/*!
Clone a pipeline of processes without putting them under process control. When 
specified the processes are also initialized. Any errors during process
initialization cause an exception to be thrown.

\param spProc		The pipeline of processes to clone.
*/
boost::intrusive_ptr<IProcess> PipeUtils::clonePipeline(boost::intrusive_ptr<IProcess> spProc)
{
	boost::intrusive_ptr<IProcess> spReturnProc = recursiveClonePipeline(spProc);
	return spReturnProc;
}

/*!
The clone is a deep copy of the original, not under process management. The process is
not initialized.

\param spProc		The process to clone.
\param pCloneMap	Internal variable used for recursion, just ignore it please.

\return The clone of the process.
*/
boost::intrusive_ptr<IProcess> PipeUtils::recursiveClonePipeline(
	boost::intrusive_ptr<IProcess> spProc,
	std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >* pCloneMap	)
{
	assert(spProc);

	typedef std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> > CloneMap;
	CloneMap stackCloneMap;
	if (!pCloneMap)
	{
		pCloneMap = &stackCloneMap;
	}

	boost::intrusive_ptr<IProcess> spClone;
	PYXCOM_HRESULT hr = PYXCOMCreateInstance(spProc->getSpec()->getClass(), 0, IProcess::iid, (void**) &spClone);
	if (PYXCOM_FAILED(hr))
	{
		PYXTHROW(	PYXException,
					"Process clsid '" << spProc->getSpec()->getClass() << "' not found");
	}
	spClone->setFinalize(false);

	spClone->setProcID(spProc->getProcID());
	spClone->setProcVersion(spProc->getProcVersion());

	spClone->setProcName(spProc->getProcName());
	spClone->setProcDescription(spProc->getProcDescription());

	spClone->setAttributes(spProc->getAttributes());
	spClone->setData(spProc->getData());

	{
		std::vector<PYXPointer<Parameter> > vecParam;
		for (int nParam = 0; nParam != spProc->getParameterCount(); ++nParam)
		{
			PYXPointer<Parameter> spParam = spProc->getParameter(nParam);
			std::vector<boost::intrusive_ptr<IProcess> > vecValue;
			for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
			{
				boost::intrusive_ptr<IProcess> spValue = spParam->getValue(nValue);
				CloneMap::iterator it = pCloneMap->find(spValue);
				if (it == pCloneMap->end())
				{
					// Recursion.
					it = pCloneMap->insert(std::make_pair(spValue, recursiveClonePipeline(spValue, pCloneMap))).first;
				}
				vecValue.push_back(it->second);
			}
			vecParam.push_back(Parameter::create(spParam->getSpec(), vecValue));
		}
		spClone->setParameters(vecParam);
	}
	return spClone;
}

/*!
Delegates the modifying of a pipeline to another version of modifyPipeline().  
This method exists for callers who don't want to provide a CloneMap as an 
argument.

\param spPipe	The pipeline containing the process to modify.
\param spProc	The process to modify.

\return	The modified pipeline, or the original if the process to modify is
		not in the original pipeline.
*/
boost::intrusive_ptr<IProcess> PipeUtils::modifyPipeline(
	boost::intrusive_ptr<IProcess> spPipe,
	boost::intrusive_ptr<IProcess> spProc)
{
	CloneMap cloneMap;
	return modifyPipeline(spPipe, spProc, cloneMap);
}

/*!
Prepares a pipeline for modification. In most cases, you should probably use
this method instead of the lower-level clone methods.

The pipe is the root of a pipeline that may or may not contain the process
intended for modification. If it does, it is cloned and its version is
incremented. Also, any parent processes through the pipeline, up to and
including the root process, are also cloned and their version incremented.
None of these processes are initialized, so be sure to do so before use.

\param spPipe		The pipeline containing the process to modify.
\param spProc		The process to modify.
\param cloneMap		Internal variable used for recursion, just ignore it please.  
					Is a map of original->cloned processes, containing all the 
					processes from spProc upwards (i.e. all its parents).

\return The modified pipeline, or the original if the process to modify is
		not in the original pipeline.
*/
boost::intrusive_ptr<IProcess> PipeUtils::modifyPipeline(
	boost::intrusive_ptr<IProcess> spPipe,
	boost::intrusive_ptr<IProcess> spProc,
	std::map<boost::intrusive_ptr<IProcess>, 
	boost::intrusive_ptr<IProcess> >& cloneMap)
{
	assert(spPipe && spProc);

	boost::intrusive_ptr<IProcess> spPipeMod = spPipe;

	if (spPipe == spProc)
	{
		if (cloneMap.find(spPipe) == cloneMap.end())
		{
			spPipeMod = cloneProcess(spPipe);
			spPipeMod->setProcVersion(spPipeMod->getProcVersion() + 1);
			cloneMap[spPipe] = spPipeMod;
		}
		else
		{
			// Already cloned
			return cloneMap[spPipe];
		}
	}

	for (int nParam = 0; nParam != spPipe->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spPipe->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			boost::intrusive_ptr<IProcess> spValue = spParam->getValue(nValue);
			boost::intrusive_ptr<IProcess> spValueMod = 
				modifyPipeline(spValue, spProc, cloneMap);
			if (spValueMod != spValue)
			{
				if (spPipeMod == spPipe)
				{
					if (cloneMap.find(spPipe) == cloneMap.end())
					{
						spPipeMod = cloneProcess(spPipe);
						spPipeMod->setProcVersion(spPipeMod->getProcVersion() + 1);
						cloneMap[spPipe] = spPipeMod;
					}
					else
					{
						// Already cloned
						spPipeMod = cloneMap[spPipe];
					}
				}
				spPipeMod->getParameter(nParam)->setValue(nValue, spValueMod);
			}
		}
	}

	return spPipeMod;
}

/*!
Shows a pipeline in a form similar to the DOS tree command.

Does not print the prefix before the root process, so if you want it there, you'll have
to do that manually.

\code
<I>{3D7ACB32-7529-44F1-82C5-2D2F6F1CB6BC}[8] "channel combiner"
\-<input coverages>
  +-<I>{550E3DFA-144D-48F1-BAFB-EC790943692D}[8] "first of rgb blender or elev clut"
  | \-<input coverages>
  |   +-<I>{B1184281-E2E3-48B1-BC1D-ADA93A2FD835}[5] "rgb blender"
  |   | \-<input coverages>
  |   |   +-<I>{FDFC5B7F-07CD-4604-9645-D76AB73B6FFE}[1] "Resolution filter"
  |   |   | \-<input coverage>
  |   |   |   \-<I>{24D14E69-9A38-4491-A220-FDD95779B798}[1] "Blue Marble Sampler"
  |   |   |     \-<Input Coverage>
  |   |   |       \-<I>{58B51A67-1413-442E-9301-507F7253A8C3}[1] "Blue Marble"
  |   |   |         \-<SRS>
  |   |   \-<I>{CD907C33-212C-4CCE-B8E4-A5E682AB73D8}[1] "Resolution filter"
  |   |     \-<input coverage>
  |   |       \-{E1680D5C-CE4F-457D-9E0E-D2DD39213356}[1] "Natural Earth Reloaded"
  |   \-<I>{DB55F30B-6E5C-44A8-ACBE-C9B570909FE8}[4] "elev colourizer"
  |     \-<input coverage>
  |       \-<I>{BCF61FBB-46CE-4218-B66C-B59770AB1F2E}[4] "elev blender"
  |         \-<input coverages>
  |           \-<I>{60EEE1E9-ACAD-4095-B081-4EF23F0B7829}[1] "Resolution filter"
  |             \-<input coverage>
  |               \-<I>{11E1B6DE-C126-4C57-A873-310128253BD4}[1] "World Elevation Sampler"
  |                 \-<Input Coverage>
  |                   \-<I>{2F42C2B7-8D83-4E6C-9885-7EBE54F5C383}[1] "World Elevation"
  \-<I>{BCF61FBB-46CE-4218-B66C-B59770AB1F2E}[4] "elev blender"
    \-<input coverages>
      \-<I>{60EEE1E9-ACAD-4095-B081-4EF23F0B7829}[1] "Resolution filter"
        \-<input coverage>
          \-<I>{11E1B6DE-C126-4C57-A873-310128253BD4}[1] "World Elevation Sampler"
            \-<Input Coverage>
              \-<I>{2F42C2B7-8D83-4E6C-9885-7EBE54F5C383}[1] "World Elevation"
\endcode

The codes that immediately preceed each process identifier hold the initialization 
state. The possible codes are:
	<I> = initialized
	<X> = failed to initialize
	<*> = needs to be initialized
	<S> = specification error
Should any of the processes be in a failure state (spec or error) an error 
description will be added on the following line.

\param out			The stream.
\param spProc		The pipeline process.
\param strPrefix	A prefix to use for every line.
\return The stream.
*/
std::ostream& PipeUtils::pipeToTree(std::ostream& out, boost::intrusive_ptr<IProcess> spProc, std::string strPrefix)
{
	assert(spProc);

	// determine the init state of the process for printing
	std::string strInitState = "<U>";
	if (spProc->getInitState() == IProcess::knInitialized) {strInitState = "<I>";}
	else if (spProc->getInitState() == IProcess::knFailedToInit) {strInitState = "<X>";}
	else if (spProc->getInitState() == IProcess::knNeedsInit) {strInitState = "<*>";}
	else if (spProc->getInitState() == IProcess::knDoesNotMeetSpec) {strInitState = "<S>";}

	// print out the process
	out << strInitState << ProcRef(spProc) << " \"" << spProc->getProcName() << "\"\n";

	// print out the error (if any)
	if (spProc->getInitError())
	{
		out << strPrefix << "<!Error!> \"" << spProc->getInitError()->getError() << "\"\n";
	}

	// add child nodes recursively
	int nPC = spProc->getParameterCount();
	for (int nP = 0; nP != nPC; ++nP)
	{
		bool bLastP = (nP == (nPC - 1));
		PYXPointer<Parameter> spParam = spProc->getParameter(nP);
		out << strPrefix << (bLastP ? "\\-" : "+-") << '<' << spParam->getSpec()->getName() << ">\n";

		strPrefix.append(bLastP ? "  " : "| ");

		int nVC = spParam->getValueCount();
		for (int nV = 0; nV != nVC; ++nV)
		{
			bool bLastV = (nV == (nVC - 1));
			boost::intrusive_ptr<IProcess> spValue = spParam->getValue(nV);

			out << strPrefix << (bLastV ? "\\-" : "+-");

			strPrefix.append(bLastV ? "  " : "| ");
			pipeToTree(out, spValue, strPrefix);
			strPrefix.resize(strPrefix.size() - 2);
		}

		strPrefix.resize(strPrefix.size() - 2);
	}

	return out;
}

/*! 
Search through a pipeline to find the first process that matches the desired 
error type. This error type is the same value that would be returned from an 
IProcessInitError. If the type is left blank then the first process with any 
error will be returned. The pipeline is examined from its leaves to its roots
in the same manner that processes are initialized.
NOTE: A process that is in the knNeedsInit is not considered to be in an 
error state.

\param spPipe		The pipeline to be examined.
\param strErrorID	The unique error identifer to be searched for. An empty
					string will match any error type.

\return The first process that matches the error type or an empty pointer
		if none is found.
*/
boost::intrusive_ptr<IProcess> PipeUtils::findFirstError(
	boost::intrusive_ptr<IProcess> spPipe,
	const std::string& strErrorID)
{
	if (!spPipe)
	{
		assert(false && "Empty pipeline or process passed to findFirstError");
		return boost::intrusive_ptr<IProcess>();
	}

	// examine each of the children
	for (int nParam = 0; nParam != spPipe->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spPipe->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			// call recursively on children in the same order as initialization
			boost::intrusive_ptr<IProcess> spErrorProc =
				findFirstError(spParam->getValue(nValue), strErrorID);
			if (spErrorProc)
			{
				return spErrorProc;
			}
		}
	}

	// if we got this far, none of the children are in an error state, check the current node
	boost::intrusive_ptr<const IProcessInitError> spError = spPipe->getInitError();
	if (spError && (strErrorID == "" || strErrorID == spError->getErrorID()))
	{
		return spPipe;
	}

	// no errors found at this point or below.
	return boost::intrusive_ptr<IProcess>();
}

/*! 
Search through a pipeline to find all of the processes who's output implements
a particular interface. The processes are added into the passed in vector. Even
if a process appears multiple times in pipeline it will only appear once
in the list of processes. If the process that is passed in is not initialized 
this method may return a false negative. A process is not required by policy
to have its output available until the process can be initialized.

\param spPipe		The pipeline to be examined. This pipeline should be 
					initialized.
\param interfaceID	The interface ID to look for
\param vecObjects	The processes that match the interface.

\return The total number of matching unique processes that were found.
*/
int PipeUtils::findProcsOfType(
	boost::intrusive_ptr<IProcess> spPipe,
	IID interfaceID,
	std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> >& vecObjects)
{
	int nCount = 0;
	if (!spPipe)
	{
		assert(false && "Empty pipeline or process passed to findFirstError");
		return 0;
	}
	else if (spPipe->getInitState() != IProcess::knInitialized)
	{
		TRACE_INFO("Examining uninitialized pipeline: '" << spPipe->getProcName() << 
			"'. findProcsOfType() may result in false negatives.");
	}

	// examine each of the children
	for (int nParam = 0; nParam != spPipe->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spPipe->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			// call recursively on children in the same order as initialization
			nCount += findProcsOfType(spParam->getValue(nValue), interfaceID, vecObjects);
		}
	}

	// all of the children have been processed, check the current node
	// if the pipeline is not initialized it may not have output.
	boost::intrusive_ptr<PYXCOM_IUnknown> spUnknown;
	if (spPipe->getOutput())
	{
		spPipe->getOutput()->QueryInterface(interfaceID, (void**) &spUnknown);
		if (spUnknown)
		{
			// only add the process to the list if it is unique
			if (std::find(vecObjects.begin(), vecObjects.end(), spUnknown) == vecObjects.end())
			{
				vecObjects.push_back(spUnknown);
				++nCount;
			}
		}
	}
	else
	{
		TRACE_DEBUG("No Output for: " << spPipe->getProcName());
	}

	// no errors found at this point or below.
	return nCount;
}

/*! 
Search through a pipeline to find all of the processes that thier spec is a specific class
The processes are added into the passed in vector. Even if a process appears multiple times in pipeline 
it will only appear once in the list of processes. 

\param spPipe		The pipeline to be examined. This pipeline should be 
					initialized.
\param interfaceID	The interface ID to look for
\param vecObjects	The processes that match the interface.

\return The total number of matching unique processes that were found.
*/

int PipeUtils::findProcsOfClass(
		boost::intrusive_ptr<IProcess> spPipe,
		CLSID classID,
		std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> >& vecObjects)
{
	int nCount = 0;
	if (!spPipe)
	{
		assert(false && "Empty pipeline or process passed to findFirstError");
		return 0;
	}
	
	// examine each of the children
	for (int nParam = 0; nParam != spPipe->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spPipe->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			// call recursively on children in the same order as initialization
			nCount += findProcsOfClass(spParam->getValue(nValue), classID, vecObjects);
		}
	}

	// all of the children have been processed, check the current node	
	boost::intrusive_ptr<PYXCOM_IUnknown> spUnknown;

	if (spPipe->getSpec()->getClass() == classID)
	{
		spUnknown = spPipe->QueryInterface<PYXCOM_IUnknown>();

		if (spUnknown)
		{
			// only add the process to the list if it is unique
			if (std::find(vecObjects.begin(), vecObjects.end(), spUnknown) == vecObjects.end())
			{
				vecObjects.push_back(spUnknown);
				++nCount;
			}
		}
	}	

	// no errors found at this point or below.
	return nCount;
}



//! Check if the given ProcRef is a pramter of the given spProc
bool PipeUtils::findAsParameter(boost::intrusive_ptr<IProcess> spProc, const ProcRef & procref)
{
	// examine each of the children
	for (int nParam = 0; nParam != spProc->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spProc->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			if (ProcRef(spParam->getValue(nValue)) == procref)
			{
				return true;
			}
		}
	}

	return false;
}

//! Check if the given ProcRef is a processes inside the given pipeline (recursive check)
bool PipeUtils::doesPipelineContainProcRef(boost::intrusive_ptr<IProcess> spProc, const ProcRef & procref)
{
	// check the root
	if (ProcRef(spProc) == procref)
	{
		return true;
	}

	// examine each of the children
	for (int nParam = 0; nParam != spProc->getParameterCount(); ++nParam)
	{
		PYXPointer<Parameter> spParam = spProc->getParameter(nParam);
		for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
		{
			if (doesPipelineContainProcRef(spParam->getValue(nValue),procref))
			{
				return true;
			}
		}
	}

	return false;
}

void PipeUtils::waitUntilPipelineIdentityStable(boost::intrusive_ptr<IProcess> spPipe)
{
	while (!PipeUtils::isPipelineIdentityStable(spPipe))
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}

bool PipeUtils::isPipelineIdentityStable(boost::intrusive_ptr<IProcess> spPipe)
{
	// We need to find all processes that refer to local files
    // and make sure that the manifest is available for them.
	std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> > procs;

	findProcsOfType(spPipe,IUrl::iid,procs);

	for(unsigned int i=0;i<procs.size();i++)
	{
		boost::intrusive_ptr<IUrl> url = procs[i]->QueryInterface<IUrl>();

		if (url)
		{
			//force initialization of the url pipeline as it harmless to do so and it give the right indentitiy
			auto proc = procs[i]->QueryInterface<IProcess>();

			if (proc && proc->getInitState() == IProcess::knNeedsInit)
			{
				proc->initProc(true);
			}

			if (url->isLocalFile() && url->getManifest().size() == 0)
			{
				// we found a local file process with no manifest yet.
				return false;
			}
		}
	}

	procs.clear();

	// second, find all procs that have embedded resource and check if we can publish their resource also
	findProcsOfType(spPipe,IEmbeddedResourceHolder::iid,procs);

	for(unsigned int i=0;i<procs.size();i++)
	{
		boost::intrusive_ptr<IEmbeddedResourceHolder> resources = procs[i]->QueryInterface<IEmbeddedResourceHolder>();

		if (resources)
		{
			// go over all internal pipelines
			for(int r=0;r<resources->getEmbeddedResourceCount();r++)
			{
				boost::intrusive_ptr<IProcess> process = resources->getEmbeddedResource(r);

				if (process)
				{
					// check that pipeline has a stable identity
					if (!isPipelineIdentityStable(process))
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

ProcessInitHelper PipeUtils::createProcess( const CLSID & clsid )
{
	return ProcessInitHelper(clsid);
}

ProcessInitHelper PipeUtils::createProcess( const std::string & clsid )
{
	return ProcessInitHelper(clsid);
}

std::string PipeUtils::getProcessIdentityCacheDirectory(boost::intrusive_ptr<IProcess> process)
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path pathCache = cache.getPath(process->getIdentity(), false);

	return pathCache.string();
}



//////////////////////////////////////////////////////////////////////////
// ProcessInitHelper 
//////////////////////////////////////////////////////////////////////////

ProcessInitHelper & ProcessInitHelper::setAttribute( const std::string & key,const std::string & value )
{
	m_attributesBeenApplyed = false;
	m_attributes[key] = value;
	return *this;
}

ProcessInitHelper & ProcessInitHelper::addInput( int nInputIndex,const boost::intrusive_ptr<IProcess> & process )
{
	m_process->getParameter(nInputIndex)->addValue(process);
	return *this;
}

ProcessInitHelper & ProcessInitHelper::setProcName( const std::string & name )
{
	m_process->setProcName(name);
	return *this;
}

ProcessInitHelper & ProcessInitHelper::setProcDescription( const std::string & description )
{
	m_process->setProcDescription(description);
	return *this;
}

ProcessInitHelper & ProcessInitHelper::borrowNameAndDescription( const boost::intrusive_ptr<IProcess> & process )
{
	m_process->setProcName(process->getProcName());
	m_process->setProcDescription(process->getProcDescription());
	return *this;
}

ProcessInitHelper & ProcessInitHelper::setProcID( REFGUID procid )
{
	m_process->setProcID(procid);
	return *this;
}

ProcessInitHelper & ProcessInitHelper::setProcVersion( int nVersion )
{
	m_process->setProcVersion(nVersion);
	return *this;
}

boost::intrusive_ptr<IProcess> ProcessInitHelper::getProcess( bool initProc )
{
	if (!m_attributesBeenApplyed)
	{
		m_process->setAttributes(m_attributes);
		m_attributesBeenApplyed = true;
	}

	if (initProc)
	{
		m_process->initProc(true);
	}

	return m_process;
}

ProcessInitHelper::ProcessInitHelper( const CLSID & clsid ) : m_attributesBeenApplyed(false)
{
	m_process = PYXCOMCreateInstance<IProcess>(clsid);
}

ProcessInitHelper::ProcessInitHelper(const std::string & clsid) : m_attributesBeenApplyed(false)
{
	m_process = PYXCOMCreateInstance<IProcess>(strToGuid(clsid));
}

ProcessInitHelper::ProcessInitHelper(const ProcessInitHelper & other) : m_process(other.m_process), m_attributesBeenApplyed(other.m_attributesBeenApplyed)
{
	//[shatzi]: this is not safe because now we have two ProcessInitHelper that modifying the same process.
	//However, we have to do this to allow a function return a ProcessInitHelper, until we apply move semantics
}