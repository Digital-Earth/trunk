#ifndef PYXIS__PIPE__PIPE_MANAGER_H
#define PYXIS__PIPE__PIPE_MANAGER_H
/******************************************************************************
pipe_manager.h

begin		: 2007-01-29
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/pyxcom.h"

// boost includes
#include <boost/intrusive_ptr.hpp>
#include <boost/filesystem/path.hpp>

// standard includes
#include <iosfwd>
#include <map>
#include <vector>

// forward declarations
struct IProcess;
class ProcRef;
class Notifier;

/*!
*/
//! Process resolver.
class PYXLIB_DECL ProcessResolver : public PYXObject
{
public:

	//! Resolves a process.
	virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procref) { return boost::intrusive_ptr<IProcess>(); }

	//! Notification that a process was resolved.
	virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProc) { return spProc; }

	//! Notification that a process was finalized.
	virtual void notifyFinalize(boost::intrusive_ptr<IProcess> spProc) {}
};

/*!
*/
//! PIPE manager.
class PYXLIB_DECL PipeManager
{
public:

	//! Returns a resolved process from any known resolution system.
	static boost::intrusive_ptr<IProcess> getProcess(
		const ProcRef& procref, bool bInitialize = true);

	//! Notification that a process is being finalized.
	static void notifyFinalize(boost::intrusive_ptr<IProcess> spProc);

	//! an event which gets fired if a process is wrapped in a proxy. 
	static Notifier* getWrappedProcessNotifier();

	//! an event which gets when a process notify about processing event. 
	static Notifier* getProcessProcessingNotifier();


	//! Adds a process resolver to the chain of responsibility.
	static void pushProcessResolver(PYXPointer<ProcessResolver> spProcResolver)
	{
		assert(spProcResolver != 0);
		m_vecProcResolver.push_back(spProcResolver);
	}

	//! Removes a process resolver to the chain of responsibility.
	static PYXPointer<ProcessResolver> popProcessResolver()
	{
		assert(!m_vecProcResolver.empty());
		PYXPointer<ProcessResolver> spProcResolver = m_vecProcResolver.back();
		m_vecProcResolver.pop_back();		

		return spProcResolver;
	}

	static void clearProcessResolvers()
	{
		std::vector<PYXPointer<ProcessResolver> > temp;
		//remove all ProcessReoslver without destorying them...
		m_vecProcResolver.swap(temp);
		
		//Now, when the PipeManager has no resolvers anymore. let the resolvers destory themself.
		//NOTE: just clearing m_vecProcResolver cause a failure.
		//      The resolvers were in the middle of desotrying thier map while the process manager notify them that process where desotried 
	}

	// NOTE: THIS CODE MAY BE REQUIRED FOR IMPLEMENTING BATCH INSERTS OF 
	// PROCESSES.
#if 0
	/*! Resolves and returns a vector of processes present in a vector of 
	pipelines.
	*/
	static std::vector< boost::intrusive_ptr<IProcess> > resolvePipelines(
		const std::vector< std::vector< boost::intrusive_ptr<IProcess> > >& 
		vecPipelinesToImport);
#endif

	//! Imports a pipeline from memory.
	static std::vector<boost::intrusive_ptr<IProcess> > import(boost::intrusive_ptr<IProcess> spProc);

	//! Imports a pipeline from memory, before which it increments process 
	//! versions until the ProcRefs in the CloneMap are unique.
	static std::vector<boost::intrusive_ptr<IProcess> > import(
		boost::intrusive_ptr<IProcess> spProc, 
		const std::map<boost::intrusive_ptr<IProcess>, 
		boost::intrusive_ptr<IProcess> >& cloneMap);

	//! Imports a pipeline from a file.
	static std::vector<boost::intrusive_ptr<IProcess> > import(const std::string& strFileURI);

	//! Imports a pipeline from a string.
	static std::vector<boost::intrusive_ptr<IProcess> > importStr(const std::string& strXMLPipeline);

	//! release memory if possible;
	static void releaseMemoryIfPossible();

	//! Returns the current process path.
	static boost::filesystem::path getProcessPath();

public:

	//! Writes a process to a stream.
	static void writeProcess(std::ostream& out, boost::intrusive_ptr<IProcess> spProc);

	//! Writes a process to a file.
	static void writeProcessToFile(const std::string& strURI, boost::intrusive_ptr<IProcess> spProc);

	//! Writes a process to a string.
	static void writeProcessToString(std::string* pStr, boost::intrusive_ptr<IProcess> spProc);

	//! Writes a pipeline to a stream.
	static void writePipeline(std::ostream& out, boost::intrusive_ptr<IProcess> spPipe);

	//! Writes a pipeline to a file.
	static void writePipelineToFile(const std::string& strURI, boost::intrusive_ptr<IProcess> spPipe);

	//! Writes a pipeline to a string.
	static void writePipelineToString(std::string* pStr, boost::intrusive_ptr<IProcess> spPipe);


	//! Reads a process from a stream.
	static boost::intrusive_ptr<IProcess> readProcess(std::istream& in);

	//! Reads a process from a file.
	static boost::intrusive_ptr<IProcess> readProcessFromFile(const std::string& strURI);

	//! Reads a process from a string.
	static boost::intrusive_ptr<IProcess> readProcessFromString(const char* pStr, int nStrSize);

	//! Reads a process from a string.
	static boost::intrusive_ptr<IProcess> readProcessFromString(const std::string & xmlString);

	//! Reads a pipeline from a stream.
	static boost::intrusive_ptr<IProcess> readPipeline(std::istream& in);

	//! Reads a pipeline from a file.
	static boost::intrusive_ptr<IProcess> readPipelineFromFile(const std::string& strURI);

	//! Reads a pipeline from a string.
	static boost::intrusive_ptr<IProcess> readPipelineFromString(const char* pStr, int nStrSize);

	//! Reads a pipeline from a string.
	static boost::intrusive_ptr<IProcess> readPipelineFromString(const std::string & xmlString);

	static bool exists();

private:

	//! For initialization.
	static void initialize();

	//! For uninitialization.
	static void uninitialize();
	
private:
	
	//! Chain of responsibility for resolving processes.
	static std::vector<PYXPointer<ProcessResolver> > m_vecProcResolver;

	//! A notifier that fires when a process has been wrapped in a proxy.
	static Notifier* m_pWrappedProcessNotifier;

	//! A notifier that fires when a process notify about processing.
	static Notifier* m_pProcessProcessingNotifier;

	//! a bool inditcate if the pipe manager still exists in memory
	static bool s_managerExists;

private:

	//! For initialization and uninitialization.
	friend class PYXLibInstance;
};

#endif // guard
