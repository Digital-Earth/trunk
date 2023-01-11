#ifndef PYXIS__PIPE__PIPE_UTILS_H
#define PYXIS__PIPE__PIPE_UTILS_H
/******************************************************************************
pipe_utils.h

begin		: 2007-03-09
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/pipe/process.h"
#include "pyxis/utility/trace.h"

// boost includes
#include <boost/intrusive_ptr.hpp>

// standard includes
#include <functional>
#include <map>
#include <iosfwd>
#include <set>
#include <sstream>
#include <vector>

// forward declarations
struct IProcess;


class PYXLIB_DECL ProcessInitHelper
{
public:
	ProcessInitHelper(const CLSID & clsid);
	ProcessInitHelper(const std::string & clsid);

	ProcessInitHelper(const ProcessInitHelper & other);

	ProcessInitHelper & setAttribute(const std::string & key,const std::string & value);
	ProcessInitHelper & addInput(int nInputIndex,const boost::intrusive_ptr<IProcess> & process);
	ProcessInitHelper & setProcName(const std::string & name);
	ProcessInitHelper & setProcDescription(const std::string & description);
	ProcessInitHelper & borrowNameAndDescription(const boost::intrusive_ptr<IProcess> & process);
	ProcessInitHelper & setProcID(REFGUID procid);
	ProcessInitHelper & setProcVersion(int nVersion);

	boost::intrusive_ptr<IProcess> getProcess(bool initProc);

	operator boost::intrusive_ptr<IProcess> ()
	{
		return getProcess(false);
	}

private:
	boost::intrusive_ptr<IProcess> m_process;
	std::map<std::string,std::string> m_attributes;
	bool m_attributesBeenApplyed;
};

/*!
*/
//! PIPE utilities.
class PYXLIB_DECL PipeUtils
{
public:

	//! Removes non-root processes from the collection.
	static void pruneNonRoots(std::vector<boost::intrusive_ptr<IProcess> >* pVecProc);

	//! Clones a process.
	static boost::intrusive_ptr<IProcess> cloneProcess(boost::intrusive_ptr<IProcess> spProc);

	//! Clone an entire pipeline, initialze on request.
	static boost::intrusive_ptr<IProcess> clonePipeline(boost::intrusive_ptr<IProcess> spProc);

	//! Prepares a pipeline for modification.
	static boost::intrusive_ptr<IProcess> modifyPipeline(
		boost::intrusive_ptr<IProcess> spPipe,
		boost::intrusive_ptr<IProcess> spProc);	

	//! Prepares a pipeline for modification.
	static boost::intrusive_ptr<IProcess> modifyPipeline(
		boost::intrusive_ptr<IProcess> spPipe,
		boost::intrusive_ptr<IProcess> spProc,
		std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >& cloneMap);

	//! Shows a pipeline as a text tree.
	static std::ostream& pipeToTree(std::ostream& out, boost::intrusive_ptr<IProcess> spProc, std::string strPrefix = "");

	//! Shows a pipeline as a text tree string.
	static std::string pipeToTree(boost::intrusive_ptr<IProcess> spProc)
	{
		std::ostringstream out;
		pipeToTree(out, spProc);
		return out.str();
	}

	//! Search through a pipeline for a process in an error state.
	static boost::intrusive_ptr<IProcess> findFirstError(
		boost::intrusive_ptr<IProcess> spPipe,
		const std::string& strErrorID = "");

	//! Find all of the process within a pipeline that output a particular interface.
	static int findProcsOfType(
		boost::intrusive_ptr<IProcess> spPipe,
		IID interfaceID,
		std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> >& vecObjects);

	//! Find all of the process that are of a specific class
	static int findProcsOfClass(
		boost::intrusive_ptr<IProcess> spPipe,
		CLSID classID,
		std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> >& vecObjects);

	//! Check if the given ProcRef is a pramter of the given spProc
	static bool findAsParameter(boost::intrusive_ptr<IProcess> spProc, const ProcRef & procref);

	//! Check if the given ProcRef is a processes inside the given pipeline (recursive check)
	static bool doesPipelineContainProcRef(boost::intrusive_ptr<IProcess> spProc, const ProcRef & procref);

	/*
	This function traverses the pipeline and looks for IUrl processes and checks if they have checksums ready for all local resources.
	Moreover, this function also looks for IEmbeddedResourceHolder processes and checks the pipeline for IUrl process,
	to make sure all supported resources have checksums.

	*/
	//! Check if the given pipeline identity is stable (all supported IUrl resolve checksums). useful before publishing or importing pipelines	
	static bool isPipelineIdentityStable(boost::intrusive_ptr<IProcess> spPipe);

	static void waitUntilPipelineIdentityStable(boost::intrusive_ptr<IProcess> spPipe);

	/*
	A utility function help to create and initialize a process settings like: name,description,inputs and attributes
	*/
	//! A utility function help to create and initialize a process settings like: name,description,inputs and attributes
	static ProcessInitHelper createProcess(const CLSID & clsid);

	/*
	A utility function help to create and initialize a process settings like: name,description,inputs and attributes
	*/
	//! A utility function help to create and initialize a process settings like: name,description,inputs and attributes
	static ProcessInitHelper createProcess(const std::string & clsid);

	/*
	A utility function that returns the cache directory of the process,
	this function will not create the directory if it does not exist
	*/
	//! A utility function that returns the cache directory of the process, this function will not create the directory if it does not exist
	static std::string getProcessIdentityCacheDirectory(boost::intrusive_ptr<IProcess> process);

	/*!
	Performs a depth-first traversal of the processes within a pipeline, including
	the root. If a process appears more than once in a pipeline, it will be visited
	each time it appears. The function object must take a process as its sole
	argument and return a boolean indicating whether to abort the traversal. That is,
	consider it to be derived from
	std::unary_function<boost::intrusive_ptr<IProcess>, bool>.

	\param spProc		The root process of the pipeline.
	\param f			The function object to apply to each process.
	\param bPostOrder	Traversal is preorder if false (default), postorder if true.
	\return Whether the traversal was aborted.
	*/
	//! Visits a pipeline.
	template <typename F>
	static bool visitPipeline(	boost::intrusive_ptr<IProcess> spProc,
								F& f,
								bool bPostOrder = false)
	{
		if (!bPostOrder && f(spProc))
		{
			return true; // abort
		}
		
		for (int nParam = 0; nParam != spProc->getParameterCount(); ++nParam)
		{
			PYXPointer<Parameter> spParam = spProc->getParameter(nParam);
			for (int nValue = 0; nValue != spParam->getValueCount(); ++nValue)
			{
				if (visitPipeline(spParam->getValue(nValue), f, bPostOrder))
				{
					return true; // abort
				}
			}
		}
		return bPostOrder ? f(spProc) : false; // abort?
	}	

private:

	//! Clones a pipeline without initializing it.
	static boost::intrusive_ptr<IProcess> recursiveClonePipeline(boost::intrusive_ptr<IProcess> spProc,
		std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >* pCloneMap = 0);
};

template <typename TEST, typename T, typename F>
class IfVisitor : std::unary_function<boost::intrusive_ptr<IProcess>, bool>
{
	TEST m_test; // test function
	T m_t;       // true function
	F m_f;       // false function
public:
	IfVisitor(TEST test, T t, F f) : m_test(test), m_t(t), m_f(f) {}

	bool operator ()(boost::intrusive_ptr<IProcess> spProc)
	{
		return m_test(spProc) ? m_t(spProc) : m_f(spProc);
	}
};

template <typename TEST, typename T, typename F>
inline
IfVisitor<TEST, T, F> makeIfVisitor(TEST test, T t, F f)
{
	return IfVisitor<TEST, T, F>(test, t, f);
}

struct NoopVisitor : std::unary_function<boost::intrusive_ptr<IProcess>, bool>
{
	bool operator ()(boost::intrusive_ptr<IProcess> spProc)
	{
		return false;
	}
};

inline
NoopVisitor makeNoopVisitor()
{
	return NoopVisitor();
}

template <typename F>
class TraceVisitor : std::unary_function<boost::intrusive_ptr<IProcess>, bool>
{
	F m_f;
	bool m_bPre;
	bool m_bPost;
public:
	TraceVisitor(bool bPre = true, bool bPost = true) : m_bPre(bPre), m_bPost(bPost) {}
	TraceVisitor(F f, bool bPre = true, bool bPost = true) : m_f(f), m_bPre(bPre), m_bPost(bPost) {}

	bool operator ()(boost::intrusive_ptr<IProcess> spProc)
	{
		if (m_bPre)
		{
			TRACE_INFO("TraceVisitor >>> " << ProcRef(spProc) << " \"" << spProc->getProcName() << '"');
		}
		bool b = m_f(spProc);
		if (m_bPost)
		{
			TRACE_INFO("TraceVisitor <<< " << b << " from " << ProcRef(spProc) << " \"" << spProc->getProcName() << '"');
		}
		return b;
	}
};

template <typename F>
inline
TraceVisitor<F> makeTraceVisitor(F f)
{
	return TraceVisitor<F>(f);
}

template <typename F>
class UniqueVisitor : std::unary_function<boost::intrusive_ptr<IProcess>, bool>
{
	std::set<boost::intrusive_ptr<IProcess> > m_setProc;
	F m_f;
public:
	UniqueVisitor() {}
	UniqueVisitor(F f) : m_f(f) {}

	bool operator ()(boost::intrusive_ptr<IProcess> spProc)
	{
		if (m_setProc.find(spProc) != m_setProc.end())
		{
			return false;
		}
		
		m_setProc.insert(spProc);
		return m_f(spProc);
	}
};

template <typename F>
inline
UniqueVisitor<F> makeUniqueVisitor(F f)
{
	return UniqueVisitor<F>(f);
}

#endif // guard
