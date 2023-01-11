/******************************************************************************
pipe_manager.cpp

begin		: 2007-01-29
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/pipe_manager.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/pipe/parameter.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/xml_utils.h"

// xerces includes
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

// boost includes
#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <set>

XERCES_CPP_NAMESPACE_USE

	namespace
{

	//! Current process path (empty if no import).
	boost::filesystem::path  processPath;

	//! Mutex.
	boost::recursive_mutex mutex;

	//! Struct used for writing process pipeline XML to a stream.
	struct WriteInfo
	{
		WriteInfo(std::ostream& out) : out(out) {}
		std::ostream& out;
		std::set<GUID> setID;
	};

	/*!
	Writes this process (only).
	*/
	void nonrecursiveWriteProcess(WriteInfo& wi, boost::intrusive_ptr<IProcess> spProc)
	{
		// Write process.
		wi.out << "<process\n";
		wi.out << "  id=\"" << spProc->getProcID() << "\"\n";
		wi.out << "  version=\"" << spProc->getProcVersion() << "\"\n";
		wi.out << "  clsid=\"" << spProc->getSpec()->getClass() << "\"\n";
		wi.out << "  name=\"" << XMLUtils::toSafeXMLText(spProc->getProcName(), true) << "\"\n";
		wi.out << "  description=\"" << XMLUtils::toSafeXMLText(spProc->getProcDescription(), true) << "\"";

		std::map<std::string, std::string> mapAttr = spProc->getAttributes();
		if (!mapAttr.empty())
		{
			for (std::map<std::string, std::string>::iterator it = mapAttr.begin();
				it != mapAttr.end(); ++it)
			{
				// TODO should ensure attribute name is valid
				wi.out << "\n  " << XMLUtils::toSafeXMLText(it->first );
				wi.out << "=\"" << XMLUtils::toSafeXMLText(it->second, true) << "\"";
			}
		}

		wi.out << ">\n";

		std::string strData = spProc->getData();
		if (!strData.empty())
		{
			// TODO still not sure what to do with CDATA,
			// thinking it should NOT be escaped but SHOULD be UTF-8
			// Best solution is to encode as Base64 which is ASCII and doesn't need escapes.
			wi.out << "  <data><![CDATA[" << strData << "]]></data>\n";
		}

		// TODO see if there are any params at all and skip below section if none

		if (0 < spProc->getParameterCount())
		{
			wi.out << "  <parameter_list>\n";

			for (int n = 0; n != spProc->getParameterCount(); ++n)
			{
				PYXPointer<Parameter> spParam = spProc->getParameter(n);

				wi.out << "    <parameter>\n";
				for (int nv = 0; nv != spParam->getValueCount(); ++nv)
				{
					wi.out << "      <process_ref idref=\"" << spParam->getValue(nv)->getProcID() << "\""
						<< " version=\"" << spParam->getValue(nv)->getProcVersion() << "\"/>\n";
				}

				wi.out << "    </parameter>\n";
			}

			wi.out << "  </parameter_list>\n";
		}

		wi.out << "</process>\n";
	}

	/*!
	Writes this process (and all its parameter processes first).
	*/
	void recursiveWriteProcess(WriteInfo& wi, boost::intrusive_ptr<IProcess> spProc)
	{
		if (wi.setID.find(spProc->getProcID()) != wi.setID.end())
		{
			// Process already written.
			return;
		}

		for (int n = 0; n != spProc->getParameterCount(); ++n)
		{
			PYXPointer<Parameter> spParam = spProc->getParameter(n);
			for (int nv = 0; nv != spParam->getValueCount(); ++nv)
			{
				recursiveWriteProcess(wi, spParam->getValue(nv));
			}
		}

		nonrecursiveWriteProcess(wi, spProc);

		// Record that process was written.
		wi.setID.insert(spProc->getProcID());
	}

	/*!
	SAX2 handler for reading pipeline file.
	*/
	class PipeHandler : public DefaultHandler
	{
	public:

		/*!
		\param bResolveProcess	Whether to resolve processes. If false, it's not
		going to try to resolve a process to see if it's
		already loaded. We set it to false when called from
		the local disk process resolver so that we don't
		recurse infinitely.
		*/
		//! Constructor
		PipeHandler(bool bResolveProcess = false) :
			m_bResolveProcess(bResolveProcess)
		{
		}

	public:

		virtual void startDocument()
		{
			m_mapProc.clear();
			m_vecProc.clear();
			m_nParam = 0;
			m_vecValue.clear();
			m_bData = false;
			m_strData.clear();
		}

		virtual void startElement(
			const XMLCh* const uri,
			const XMLCh* const localname,
			const XMLCh* const qname,
			const Attributes& attrs		)
		{
			HRESULT hr;
			char* name = XMLString::transcode(localname);
			if (strcmp(name, "process") == 0)
			{
				std::map<std::string, std::string> mapAttr = XMLUtils::getAttributes(attrs);
				boost::intrusive_ptr<IProcess> spProc;
				{
					assert(mapAttr.find("clsid") != mapAttr.end());
					hr = PYXCOMCreateInstance(strToGuid(mapAttr["clsid"]), 0, IProcess::iid, (void**) &spProc);
					if (FAILED(hr))
					{
						PYXTHROW(	PYXException,
							"Process clsid'" << mapAttr["clsid"] << "' not found");
					}
					mapAttr.erase("clsid");
				}
				{
					assert(mapAttr.find("id") != mapAttr.end());
					spProc->setProcID(strToGuid(mapAttr["id"]));
					mapAttr.erase("id");
				}
				{
					assert(mapAttr.find("version") != mapAttr.end());
					spProc->setProcVersion(atoi(mapAttr["version"].c_str()));
					mapAttr.erase("version");
				}
				{
					assert(mapAttr.find("name") != mapAttr.end());
					spProc->setProcName(mapAttr["name"]);
					mapAttr.erase("name");
				}
				{
					assert(mapAttr.find("description") != mapAttr.end());
					spProc->setProcDescription(mapAttr["description"]);
					mapAttr.erase("description");
				}
				spProc->setAttributes(mapAttr);
				m_vecProc.push_back(spProc);

				// TODO to support process parameters by value, we'd just need to
				// also keep a stack of parameter lists, and add the process there too.
			}
			else if (strcmp(name, "parameter_list") == 0)
			{
				m_nParam = 0;
			}
			else if (strcmp(name, "parameter") == 0)
			{
				assert(m_vecValue.empty());
				m_vecValue.clear();
			}
			else if (strcmp(name, "process_ref") == 0)
			{
				std::map<std::string, std::string> mapAttr = XMLUtils::getAttributes(attrs);
				assert(mapAttr.find("idref") != mapAttr.end());
				assert(mapAttr.find("version") != mapAttr.end());
				ProcRef procref(strToGuid(mapAttr["idref"]), atoi(mapAttr["version"].c_str()));
				boost::intrusive_ptr<IProcess> spResolvedProc = PipeManager::getProcess(procref,false);
				if (!spResolvedProc)
				{
					spResolvedProc = m_mapProc[procref]; // note may alter map
				}
				if (!spResolvedProc)
				{
					PYXTHROW(	PYXException,
						"Referenced process '" << procref << "' not found");
				}
				m_vecValue.push_back(spResolvedProc);
			}
			else if (strcmp(name, "data") == 0)
			{
				assert(!m_bData && m_strData.empty());
				m_bData = true;
			}
			XMLString::release(&name);
		}

		virtual void endElement(
			const XMLCh* const uri,
			const XMLCh* const localname,
			const XMLCh* const qname	)
		{
			char* name = XMLString::transcode(localname);
			if (strcmp(name, "process") == 0)
			{
				assert(!m_vecProc.empty());
				boost::intrusive_ptr<IProcess> spProc;
				if (m_bResolveProcess &&
					(spProc = PipeManager::getProcess(ProcRef(m_vecProc.back()),false)))
				{
					// Existing process, so junk this procid and let it go away.

					//NOTE: this may cause the PipeManager to currupt his MemoryResolver. because the it save the process by ProcRef which we change. [shatzi]
					GUID procid;
					PYXCOMCreateGuid(&procid);
					m_vecProc.back()->setProcID(procid);
				}
				else
				{
					// No existing process, so use this one.
					spProc = m_vecProc.back();
				}
				m_mapProc[ProcRef(spProc)] = spProc;
				m_vecProc.pop_back();
			}
			else if (strcmp(name, "parameter") == 0)
			{
				m_vecProc.back()->getParameter(m_nParam)->setValues(m_vecValue);
				m_vecValue.clear();
				++m_nParam;
			}
			else if (strcmp(name, "data") == 0)
			{
				assert(m_bData);
				m_vecProc.back()->setData(m_strData);
				m_strData.clear();
				m_bData = false;
			}
			XMLString::release(&name);
		}

		virtual void characters(
			const XMLCh* const chars,
			const XMLSize_t length	)
		{
			if (m_bData)
			{
				m_strData.append(chars, chars + length);
			}
		}

		void fatalError(const SAXParseException& e)
		{
			std::string strSystemId(
				e.getSystemId(), e.getSystemId() + XMLString::stringLen(e.getSystemId()));
			std::string strMessage(
				e.getMessage(), e.getMessage() + XMLString::stringLen(e.getMessage()));
			TRACE_ERROR("Fatal error during parse:"
				<< " " << strSystemId
				<< " line " << e.getLineNumber()
				<< " col " << e.getColumnNumber()
				<< " " << strMessage);
		}

	public:

		//! Index to all parsed processes.
		std::map<ProcRef, boost::intrusive_ptr<IProcess> > m_mapProc;

		//! Stack of processes being parsed. (Currently only one deep.)
		std::vector<boost::intrusive_ptr<IProcess> > m_vecProc;

		//! Index of parameter currently being parsed.
		int m_nParam;

		//! Values for the parameter currently being parsed.
		std::vector<boost::intrusive_ptr<IProcess> > m_vecValue;

		//! Whether data is being processed.
		bool m_bData;

		//! The data currently being parsed.
		std::string m_strData;

		//! Whether to resolve processes.
		bool m_bResolveProcess;
	};

	//! The internal directory for storing pipeline files.
	const std::string kstrProcDir = "procs";

	//! The file extention used for pipelines
	const std::string kstrPipelineFileExt = ".ppl";

	/*!
	*/
	//! Resolves processes from a local disk cache.
	class LocalDiskProcessResolver : public ProcessResolver
	{
	public:

		static PYXPointer<LocalDiskProcessResolver> create()
		{
			return PYXNEW(LocalDiskProcessResolver);
		}

	private: 

		LocalDiskProcessResolver()
		{
			// create the process directory if it does not exist.
			boost::filesystem::path procsPath = AppServices::getWorkingPath() /
				kstrProcDir;

			if (!FileUtils::exists(procsPath))
			{
				try
				{
					boost::filesystem::create_directory(procsPath);
				}
				catch (...)
				{
					TRACE_ERROR("Unable to create process directory.");
				}
			}
		}

	public:

		virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procref)
		{
			boost::intrusive_ptr<IProcess> spProc;

			boost::filesystem::path pplFile = AppServices::getWorkingPath() /
				kstrProcDir /
				FileUtils::stringToPath(StringUtils::toString(procref) + kstrPipelineFileExt);

			if (!FileUtils::exists(pplFile) || FileUtils::isDirectory(pplFile))
			{
				return spProc;
			}

			spProc = PipeManager::readProcessFromFile(FileUtils::pathToString(pplFile));

			return spProc;
		}

		virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProc)
		{
			TRACE_NOTIFY("Notify Resolve in LocalDiskProcessResolver");
			boost::filesystem::path pplFile = AppServices::getWorkingPath() /
				kstrProcDir /
				FileUtils::stringToPath(StringUtils::toString(ProcRef(spProc)) + kstrPipelineFileExt);

			assert(!exists(pplFile));

			PipeManager::writeProcessToFile(FileUtils::pathToString(pplFile), spProc);

			return spProc;
		}
	};

	/*!
	*/
	//! Doesn't resolve processes but does provide diagnostic information.
	class DiagnosticProcessResolver : public ProcessResolver
	{
	public:

		static PYXPointer<DiagnosticProcessResolver> create()
		{
			return PYXNEW(DiagnosticProcessResolver);
		}

	public:

		//! Resolves a process.
		virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procref)
		{
			TRACE_INFO("Process " << procref << " requested");
			return boost::intrusive_ptr<IProcess>();
		}

		virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProc)
		{
			TRACE_INFO("Process " << ProcRef(spProc) << " resolved");
			return spProc;
		}

		virtual void notifyFinalize(boost::intrusive_ptr<IProcess> spProc)
		{
			TRACE_INFO("Process " << ProcRef(spProc) << " finalized");
		}
	};


	class WeakMemoryPointerProcessResolver : public ProcessResolver
	{
	public:
		typedef std::set<boost::intrusive_ptr<IProcess>> SmartPointerReferenceSet;

		static PYXPointer<WeakMemoryPointerProcessResolver> create()		
		{
			return PYXNEW(WeakMemoryPointerProcessResolver);			
		}

		WeakMemoryPointerProcessResolver()
		{
		}

		~WeakMemoryPointerProcessResolver()
		{
		}

	protected:	
		SmartPointerReferenceSet m_setProc;

		//! Remove all Process that this class is the only owner. means, thier ref count equals to 1.
		// return true if a process has been removed
		bool tryRemoveSingleOwnedProcess()
		{
			SmartPointerReferenceSet toRemove;
			for(SmartPointerReferenceSet::iterator it = m_setProc.begin();it != m_setProc.end();++it)
			{
				//this is not thread safe. and sometimes a thread would do Release or AddRef between the two calls.
				//this doesn't matter. if we got refCount == 1 - we are the only one that have pointer to the porcess. which is good and we can delete the object.
				//if we got refCount > 1, don't worry. we would get it next time...
				(*it)->AddRef();
				PYXCOM_ULONG refCount = (*it)->Release();

				//NOTE: this wouldn't work if we have two WeakMemoryPointerProcessResolver - life sucks.
				if (refCount == 1)
				{				
					toRemove.insert(*it);
				}
			}

			//remove all our pointers from thoses process. 
			//When thoses process would get destoried - they would notify the PipeManager... a mess, but work
			for(SmartPointerReferenceSet::iterator it = toRemove.begin(); it != toRemove.end(); ++it)
			{
				//this is our lame implementation of weak pointers
				m_setProc.erase(*it);
			}

			return !toRemove.empty();
		}

		//! Remove all Process that this class is the only owner. means, thier ref count equals to 1.
		void removeSingleOwnedProcess()
		{
			//contiune to remove single owned process until we no loner found any
			while(tryRemoveSingleOwnedProcess()) {};
		}

	public:
		virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procref)
		{
			//try to locate
			boost::intrusive_ptr<IProcess> spProc;

			for(SmartPointerReferenceSet::iterator it = m_setProc.begin();it != m_setProc.end();++it)
			{
				if (procref == ProcRef(*it))
				{
					spProc = *it;
				}
			}

			//We're choosing to have a "relatively quick" clean up of objects, which fires when new objects are created.
			removeSingleOwnedProcess();

			return spProc;
		}

		virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProc)
		{				
			//cache the process		
			m_setProc.insert(spProc);

			//We're choosing to have a "relatively quick" clean up of objects, which fires when new objects are created.
			removeSingleOwnedProcess();

			return spProc;
		}

		virtual void notifyFinalize(boost::intrusive_ptr<IProcess> spProc)
		{		
			m_setProc.erase(spProc);

			//We're choosing to have a "relatively quick" clean up of objects, which fires when new objects are created, or destoryed.
			removeSingleOwnedProcess();
		}


	};

	/*!
	*/
	//! Resolves processes from a map.
	template <typename MAP>
	class MemoryProcessResolver : public ProcessResolver
	{
	private:
		explicit MemoryProcessResolver(MAP& mapProc) :
		m_mapProc(mapProc),
			m_bDoCaching(true)
		{
		}

	public:

		static PYXPointer<MemoryProcessResolver> create(
			MAP& mapProc)
		{
			return PYXNEW(MemoryProcessResolver,
				mapProc);
		}

	public:

		virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procref)
		{
			boost::intrusive_ptr<IProcess> spProc;
			MAP::iterator it = m_mapProc.find(procref);
			if (it != m_mapProc.end())
			{
				spProc = it->second;
			}
			return spProc;
		}

		virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProc)
		{
			TRACE_NOTIFY("Notify Resolve in Memory Process resolver");
			if (m_bDoCaching)
			{
				m_mapProc[ProcRef(spProc)] = spProc.get();
			}
			return spProc;
		}

		virtual void notifyFinalize(boost::intrusive_ptr<IProcess> spProc)
		{
			//TRACE_INFO("Removing from weak memory map: " << ProcRef(spProc));
			m_mapProc.erase(ProcRef(spProc));
		}

	public:
		//! Turn on memory caching behaviour.
		void EnableCache() { m_bDoCaching = true; }

		//! Turn off memory caching behaviour.
		void DisableCache() { m_bDoCaching = false; }

	private:

		//! Process map. Could be weak (raw) or strong (intrusive) pointer.
		MAP& m_mapProc;

		//! true if we are to add processes that come through the NotifyResolve into our memory cache.
		bool m_bDoCaching;
	};

	//! Helper for creating memory process resolvers.
	template <typename MAP>
	PYXPointer<MemoryProcessResolver<MAP> >
		createMemoryProcessResolver(MAP& mapProc)
	{
		return MemoryProcessResolver<MAP>::create(mapProc);
	}

}

//! Chain of responsibility for resolving processes.
std::vector<PYXPointer<ProcessResolver> > PipeManager::m_vecProcResolver;

//! The notifier object for the PipeManager.
Notifier* PipeManager::m_pWrappedProcessNotifier = 0;
Notifier* PipeManager::m_pProcessProcessingNotifier = 0;
bool PipeManager::s_managerExists = false;
/*!
Must be called exactly once near program start.
*/
void PipeManager::initialize()
{
	m_pWrappedProcessNotifier = new Notifier("Pipe Manager");
	m_pProcessProcessingNotifier = new Notifier("Process Processing");

	// Establish process resolver chain of responsibility.
	pushProcessResolver(WeakMemoryPointerProcessResolver::create());
	//	pushProcessResolver(ProxyProcessResolver::create());
	//	pushProcessResolver(LocalDiskProcessResolver::create());

	s_managerExists = true;
}

bool PipeManager::exists()
{
	return s_managerExists;
}

/*!
Must be called exactly once near program end.
*/
void PipeManager::uninitialize()
{
	releaseMemoryIfPossible();

    auto stillAliveProcs = ProcessLifeTimeTracker::getInstance().getTrackedProcesses();
    if (!stillAliveProcs.empty()) 
    {
        TRACE_ERROR("PipeManager is been uninitialized while before all process been destroyed.");
        for(auto proc : stillAliveProcs) 
        {
            TRACE_INFO("Process " << proc->getProcName() << " : " << proc->getProcRef() << " still in memory");
        }
    }
    
	s_managerExists = false;

	delete m_pWrappedProcessNotifier;
	delete m_pProcessProcessingNotifier;

	clearProcessResolvers();	
}

void PipeManager::releaseMemoryIfPossible()
{
	//BIG NOTE: WeakPointerReferenceResolver is only update it self when resolve/notifyResolve/notifyFinialized is get called.
	//          Therefore, we could get to this point and still have few process alive because the WeakPointerReferenceResolver keep a pointer to them
	//          getProcess(ProcRef()) will ensure we WeakPointerReferenceResolver to release the last processes alive.
	getProcess(ProcRef(GUID(),1));
}

/*!
Uses chain of responsibility to resolve process or pipeline of processes from its ID.
When directed to do so, an attempt is made to initialize the pipeline. The pipeline
or process is not gauranteed to be initialized when it is returned and the caller 
should check the initialization state before performing any actions on the process 
output.

\param procref		The process to resolve.
\param bInitialize	A directive to initialize the pipeline or not.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::getProcess(
	const ProcRef& procref,
	bool bInitialize)
{
	assert(procref != ProcRef());
	boost::intrusive_ptr<IProcess> spProc;

	if (!s_managerExists)
	{
		return NULL;
	}

	{
		boost::recursive_mutex::scoped_lock lock(mutex);

		try
		{
			for (std::vector<PYXPointer<ProcessResolver> >::iterator it = m_vecProcResolver.begin();
				it != m_vecProcResolver.end(); ++it)
			{
				spProc = (*it)->resolve(procref);
				if (spProc)
				{
					while (m_vecProcResolver.begin() < it)
					{
						--it;
						try
						{
							spProc = (*it)->notifyResolve(spProc);
						}
						catch (...)
						{
							TRACE_ERROR("Unknown error while notifying resolution of process '" << 
								spProc);
						}
					}
					break;
				}
			}
		}
		catch (...)
		{
			TRACE_ERROR("Unknown error while resolving process '" << procref << "'");
		}
	}

	if (spProc && bInitialize && spProc->getInitState() != IProcess::knInitialized)
	{
		spProc->initProc(true);
	}
	return spProc;
}

/*!
Notifies chain of responsibility that process instance is being finalized.
*/
void PipeManager::notifyFinalize(boost::intrusive_ptr<IProcess> spProc)
{
	if (!s_managerExists)
	{
		return;
	}

	boost::recursive_mutex::scoped_lock lock(mutex);

	assert(spProc);
	try
	{
		for (std::vector<PYXPointer<ProcessResolver> >::iterator it = m_vecProcResolver.begin();
			it != m_vecProcResolver.end(); ++it)
		{
			(*it)->notifyFinalize(spProc);
		}
	}
	catch (...)
	{
		TRACE_ERROR("Unknown error while notifying finalization of process '" << spProc->getProcID() << "'");
	}
}

//! Gets a notifier that fires when a process has been wrapped in a proxy.
Notifier* PipeManager::getWrappedProcessNotifier()
{
	assert(m_pWrappedProcessNotifier);
	return m_pWrappedProcessNotifier;
}

//! an event which gets when a process notify about processing event. 
Notifier* PipeManager::getProcessProcessingNotifier()
{
	assert(m_pProcessProcessingNotifier);
	return m_pProcessProcessingNotifier;
}


// NOTE: THIS CODE MAY BE REQUIRED FOR IMPLEMENTING BATCH INSERTS OF PROCESSES.
#if 0
/*!
Resolves and returns a vector of processes present in a vector of 
pipelines.

\param vecPipelinesToImport	The pipelines to import.
\return The processes that have been resolved and are ready to import.
*/
std::vector< boost::intrusive_ptr<IProcess> > PipeManager::resolvePipelines(
	const std::vector< std::vector< boost::intrusive_ptr<IProcess> > >& vecPipelinesToImport)
{
	boost::recursive_mutex::scoped_lock lock(mutex);	

	std::vector< boost::intrusive_ptr<IProcess> > vecProcsToImport;
	std::vector< boost::intrusive_ptr<IProcess> > vecStack;

	typedef std::map< ProcRef, boost::intrusive_ptr<IProcess> > MAP;
	MAP mapProc;

	// Automate the push/pop operation so it is exception safe.
	struct Auto
	{
		Auto(MAP& mapProc)
		{
			pushProcessResolver(createMemoryProcessResolver(mapProc));
		}
		~Auto()
		{
			popProcessResolver();
		}
	} autoObj(mapProc);


	for(std::vector< std::vector< boost::intrusive_ptr<IProcess> > >::const_iterator vecIt = 
		vecPipelinesToImport.begin(); 
		vecIt != vecPipelinesToImport.end(); ++vecIt)
	{
		std::vector< boost::intrusive_ptr<IProcess> > vecPipes = *vecIt;

		for (std::vector< boost::intrusive_ptr<IProcess> >::const_iterator 
			vecPipesIt = vecPipes.begin(); 
			vecPipesIt != vecPipes.end(); ++vecPipesIt)
		{
			vecStack.clear();
			mapProc.clear();

			// Use vec as stack.
			boost::intrusive_ptr<IProcess> spProc = *vecPipesIt;
			vecStack.push_back(spProc);

			// Populate map from stack.
			while (!vecStack.empty())
			{
				boost::intrusive_ptr<IProcess> spProc = 
					vecStack.back();
				vecStack.pop_back();

				mapProc[ProcRef(spProc)] = spProc;

				for (int n = 0; n != spProc->getParameterCount(); ++n)
				{
					PYXPointer<Parameter> spParam = spProc->getParameter(n);
					for (int nv = 0; nv != spParam->getValueCount(); ++nv)
					{
						vecStack.push_back(spParam->getValue(nv));
					}
				}
			}

			for (MAP::iterator it = mapProc.begin(); it != mapProc.end(); ++it)
			{
				// Pull the processes to ensure notifications occur.
				it->second->setFinalize(true);

				boost::intrusive_ptr<IProcess> spProc;
				try
				{
					for (std::vector<PYXPointer<ProcessResolver> >::iterator 
						it1 = m_vecProcResolver.begin();
						it1 != m_vecProcResolver.end(); ++it1)
					{
						spProc = (*it1)->resolve(it->first);
						if (spProc)
						{
							vecProcsToImport.push_back(spProc);
							break;
						}
					}
				}
				catch (...)
				{
					TRACE_ERROR("Unknown error while resolving process '" << 
						it->second << "'");
				}
				vecStack.push_back(it->second);
			}
		}
	}	

	return vecProcsToImport;
}
#endif

/*!
Imports a pipeline from memory. The processes will be integrated with the process resolver
system. (I.e. they will be cached locally, etc.)

\param spProc	The process to import.
\return The processes that were imported (or already present).
*/
std::vector<boost::intrusive_ptr<IProcess> > PipeManager::import(boost::intrusive_ptr<IProcess> spProc)
{
	if (!s_managerExists)
	{
		PYXTHROW(PYXException,"Can not import process when PipeManager was not initialized");
	}

	boost::recursive_mutex::scoped_lock lock(mutex);

	typedef std::map<ProcRef, boost::intrusive_ptr<IProcess> > MAP;
	MAP mapProc;

	// Automate the push/pop operation so it is exception safe.
	struct Auto
	{
		Auto(MAP& mapProc)
		{
			pushProcessResolver(createMemoryProcessResolver(mapProc));
		}
		~Auto()
		{
			popProcessResolver();
		}
	} autoObj(mapProc);

	// Use vec as stack.
	std::vector<boost::intrusive_ptr<IProcess> > vecProc;
	vecProc.push_back(spProc);	

	// Populate map from stack.
	while (!vecProc.empty())
	{
		boost::intrusive_ptr<IProcess> spProc = vecProc.back();
		vecProc.pop_back();
		mapProc[ProcRef(spProc)] = spProc;

		for (int n = 0; n != spProc->getParameterCount(); ++n)
		{
			PYXPointer<Parameter> spParam = spProc->getParameter(n);
			for (int nv = 0; nv != spParam->getValueCount(); ++nv)
			{
				vecProc.push_back(spParam->getValue(nv));
			}
		}
	}

	for (MAP::iterator it = mapProc.begin(); it != mapProc.end(); ++it)
	{
		// Pull the processes to ensure notifications occur.
		it->second->setFinalize(true);
		vecProc.push_back(getProcess(it->first,false));
	}

	return vecProc;
}

/*!
Imports a pipeline from a file. The processes will be integrated with the process resolver
system. (I.e. they will be cached locally, etc.)

\param strFileURI	The file to import.
\return The processes that were imported (or already present).
*/
std::vector<boost::intrusive_ptr<IProcess> > PipeManager::import(const std::string& strFileURI)
{
	if (!s_managerExists)
	{
		PYXTHROW(PYXException,"Can not import file when PipeManager was not initialized");
	}

	boost::recursive_mutex::scoped_lock lock(mutex);

	PipeHandler pipeHandler(true);

	// Automate the push/pop operation so it is exception safe.
	struct Auto
	{
		Auto(PipeHandler& pipeHandler, const std::string& strFileURI)
		{
			processPath = FileUtils::stringToPath(strFileURI).branch_path();
			pushProcessResolver(createMemoryProcessResolver(pipeHandler.m_mapProc));
		}
		~Auto()
		{
			popProcessResolver();
			processPath = boost::filesystem::path();
		}
	} autoObj(pipeHandler, strFileURI);

	// TODO: Improve this code to pass an iterator into result constructor.
	std::vector<boost::intrusive_ptr<IProcess> > vec;

	// Push the processes by parsing them all from the file.
	bool bResult = XMLUtils::parseFromFile(strFileURI, pipeHandler);

	if (!bResult)
	{
		// Parse failed.
		TRACE_ERROR("Failed to parse XML during import of '" << strFileURI << "'.");
		return vec;
	}

	for (std::map<ProcRef, boost::intrusive_ptr<IProcess> >::iterator it = pipeHandler.m_mapProc.begin();
		it != pipeHandler.m_mapProc.end();
		++it)
	{
		// Pull the processes to ensure notifications occur.
		vec.push_back(getProcess(it->first,false));
	}

	return vec;
}

/*!
Imports a pipeline from a string. The processes will be integrated with the process resolver
system. (I.e. they will be cached locally, etc.)

\param strXMLPipeline	The XML representation of the pipeline.
\return The processes that were imported (or already present).
*/
std::vector<boost::intrusive_ptr<IProcess> > PipeManager::importStr(const std::string& strXMLPipeline)
{
	if (!s_managerExists)
	{
		PYXTHROW(PYXException,"Can not import xml when PipeManager was not initialized");
	}

	// TODO:  look at this function and the import() function to see if the common code
	// can be factored out.
	boost::recursive_mutex::scoped_lock lock(mutex);

	PipeHandler pipeHandler(true);

	// Automate the push/pop operation so it is exception safe.
	struct Auto
	{
		Auto(PipeHandler& pipeHandler)
		{
			pushProcessResolver(createMemoryProcessResolver(pipeHandler.m_mapProc));
		}
		~Auto()
		{
			popProcessResolver();
		}
	} autoObj(pipeHandler);

	// TODO: Improve this code to pass an iterator into result constructor.
	std::vector<boost::intrusive_ptr<IProcess> > vec;

	// Push the processes by parsing them all from the file.
	bool bResult = XMLUtils::parseFromString(strXMLPipeline.c_str(), static_cast<int>(strXMLPipeline.size()), pipeHandler);

	if (!bResult)
	{
		// Parse failed.
		return vec;
	}

	for (auto & process : pipeHandler.m_mapProc)
	{
		for (auto & it : m_vecProcResolver)
		{
			it->notifyResolve(process.second);
			vec.push_back(process.second);
		}
	}

	return vec;
}

/*!
Imports a pipeline from memory, before which it increments process versions of 
the processes present in the CloneMap until the ProcRefs are unique.  The 
processes will be integrated with the process resolver system. (i.e. they 
will be cached locally, etc.)

\param spProc	The pipeline to import.
\param cloneMap	The map of processes whose version to increment.
\return The processes that were imported.
*/
std::vector<boost::intrusive_ptr<IProcess> > PipeManager::import(
	boost::intrusive_ptr<IProcess> spProc, 
	const std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >& cloneMap)
{
	if (!s_managerExists)
	{
		PYXTHROW(PYXException,"Can not import process when PipeManager was not initialized");
	}

	boost::recursive_mutex::scoped_lock lock(mutex);

	std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >::const_iterator mapIterator;

	for(mapIterator = cloneMap.begin(); mapIterator != cloneMap.end(); ++mapIterator)
	{
		boost::intrusive_ptr<IProcess> spMapProc = mapIterator->second;

		try
		{
			while(getProcess(ProcRef(spMapProc),false) != 0)
			{
				spMapProc->setProcVersion(spMapProc->getProcVersion() + 1);
			}
		}
		catch(...)
		{
		}
	}

	return import(spProc);
}

/*!
Returns the current process path.
*/
boost::filesystem::path PipeManager::getProcessPath()
{
	return processPath;
}

/*!
Writes a single process. Any parameters are referenced but not written.

\param out		The stream.
\param spProc	The process.
*/
void PipeManager::writeProcess(std::ostream& out, boost::intrusive_ptr<IProcess> spProc)
{
	assert(spProc);
	WriteInfo wi(out);
	nonrecursiveWriteProcess(wi, spProc);
}

/*!
Writes a single process. Any parameters are referenced but not written.

\param strURI	The file URI.
\param spProc	The process.
*/
void PipeManager::writeProcessToFile(const std::string& strURI, boost::intrusive_ptr<IProcess> spProc)
{
	std::ofstream out(strURI.c_str());
	assert(out.good());
	writeProcess(out, spProc);
}

/*!
Writes a single process. Any parameters are referenced but not written.

\param pStr		The string pointer.
\param spProc	The process.
*/
void PipeManager::writeProcessToString(std::string* pStr, boost::intrusive_ptr<IProcess> spProc)
{
	assert(pStr);
	std::ostringstream out;
	writeProcess(out, spProc);
	pStr->swap(out.str());
}

/*!
Writes a pipeline of processes. Parameter processes are written before they are referenced.

\param out		The stream to write to.
\param spPipe	The pipelne of processes to write to stream.
*/
void PipeManager::writePipeline(std::ostream& out, boost::intrusive_ptr<IProcess> spPipe)
{
	assert(spPipe);
	WriteInfo wi(out);
	wi.out << "<process_list>\n";
	recursiveWriteProcess(wi, spPipe);
	wi.out << "</process_list>\n";
}

/*!
Writes a pipeline of processes. Parameter processes are written before they are referenced.

\param strURI	The file URI.
\param spPipe	The pipeline of processes to write to file.
*/
void PipeManager::writePipelineToFile(const std::string& strURI, boost::intrusive_ptr<IProcess> spPipe)
{
	std::ofstream out(strURI.c_str());
	assert(out.good());
	writePipeline(out, spPipe);
}

/*!
Writes a pipeline of processes. Parameter processes are written before they are referenced.

\param pStr		The string pointer.
\param spPipe	The pipeline of processes to write to string.
*/
void PipeManager::writePipelineToString(std::string* pStr, boost::intrusive_ptr<IProcess> spPipe)
{
	assert(pStr);
	std::ostringstream out;
	writePipeline(out, spPipe);
	pStr->swap(out.str());
}

/*!
Reads a single process. Referenced parameters must be resolvable.

\param in		The stream.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readProcess(std::istream& in)
{
	boost::intrusive_ptr<IProcess> spProc;
	PipeHandler pipeHandler;
	XMLUtils::parse(in, pipeHandler);
	if (pipeHandler.m_mapProc.size() == 1)
	{
		spProc = pipeHandler.m_mapProc.begin()->second;
	}
	return spProc;
}

/*!
Reads a single process. Referenced parameters must be resolvable.

\param strURI	The file URI.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readProcessFromFile(const std::string& strURI)
{
	boost::recursive_mutex::scoped_lock lock(mutex);

	processPath = FileUtils::stringToPath(strURI).branch_path();
	boost::intrusive_ptr<IProcess> spProc;
	PipeHandler pipeHandler;
	XMLUtils::parseFromFile(strURI, pipeHandler);
	if (pipeHandler.m_mapProc.size() == 1)
	{
		spProc = pipeHandler.m_mapProc.begin()->second;
	}
	return spProc;
}

//! Reads a process from a string.
boost::intrusive_ptr<IProcess> PipeManager::readProcessFromString(const std::string & xmlString)
{
	return readProcessFromString(xmlString.c_str(),xmlString.size());
}

/*!
Reads a single process. Referenced parameters must be resolvable.

\param pStr			The character string pointer.
\param nStrSize		The size of the character string.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readProcessFromString(const char* pStr, int nStrSize)
{
	assert(pStr);
	boost::intrusive_ptr<IProcess> spProc;
	PipeHandler pipeHandler;
	XMLUtils::parseFromString(pStr, nStrSize, pipeHandler);
	if (pipeHandler.m_mapProc.size() == 1)
	{
		spProc = pipeHandler.m_mapProc.begin()->second;
	}
	return spProc;
}

/*!
Reads a pipeline of processes. Referenced parameters must be resolvable. Any new
processes that are only found in the stream will NOT be placed under process 
management. To do so a user must import them.

\param in		The stream.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readPipeline(std::istream& in)
{
	boost::intrusive_ptr<IProcess> spPipe;
	PipeHandler pipeHandler(true);

	//struct Auto
	//{
	//	Auto(PipeHandler& pipeHandler)
	//	{
	//		pushProcessResolver(createMemoryProcessResolver(pipeHandler.m_mapProc));
	//	}
	//	~Auto()
	//	{
	//		popProcessResolver();
	//	}
	//} autoObj(pipeHandler);

	if (XMLUtils::parse(in, pipeHandler))
	{
		std::vector<boost::intrusive_ptr<IProcess> > vecProc;
		for (std::map<ProcRef, boost::intrusive_ptr<IProcess> >::iterator it = pipeHandler.m_mapProc.begin();
			it != pipeHandler.m_mapProc.end();
			++it)
		{
			vecProc.push_back(it->second);
		}
		PipeUtils::pruneNonRoots(&vecProc);
		if (vecProc.size() == 1)
		{
			spPipe = vecProc.front();
		}
	}
	return spPipe;
}

/*!
Reads a pipeline of processes. Referenced parameters must be resolvable. Any new
processes that are only found in the stream will NOT be placed under process 
management. To do so a user must import them.

\param strURI	The file URI.
\return			The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readPipelineFromFile(const std::string& strURI)
{
	boost::recursive_mutex::scoped_lock lock(mutex);

	processPath = FileUtils::stringToPath(strURI).branch_path();
	boost::intrusive_ptr<IProcess> spPipe;
	if (!FileUtils::exists(FileUtils::stringToPath(strURI)))
	{
		TRACE_ERROR("File does not exist, can't read pipeline from file: " << strURI);
		return spPipe;
	}

	PipeHandler pipeHandler(true);

	if (XMLUtils::parseFromFile(strURI, pipeHandler))
	{
		std::vector<boost::intrusive_ptr<IProcess> > vecProc;
		for (std::map<ProcRef, boost::intrusive_ptr<IProcess> >::iterator it = pipeHandler.m_mapProc.begin();
			it != pipeHandler.m_mapProc.end();
			++it)
		{
			vecProc.push_back(it->second);
		}
		PipeUtils::pruneNonRoots(&vecProc);
		if (vecProc.size() == 1)
		{
			spPipe = vecProc.front();
		}
	}

	return spPipe;
}


//! Reads a pipeline from a string.
boost::intrusive_ptr<IProcess> PipeManager::readPipelineFromString(const std::string & xmlString)
{
	return readPipelineFromString(xmlString.c_str(),xmlString.size());
}

/*!
Reads a pipeline of processes. Referenced parameters must be resolvable. Any new
processes that are only found in the stream will NOT be placed under process 
management. To do so a user must import them.

\param pStr			The character string pointer.
\param nStrSize		The size of the character string.
\return The process.
*/
boost::intrusive_ptr<IProcess> PipeManager::readPipelineFromString(const char* pStr, int nStrSize)
{
	boost::intrusive_ptr<IProcess> spPipe;
	PipeHandler pipeHandler(true);

	// Automate the push/pop operation so it is exception safe.
	//struct Auto
	//{
	//	Auto(PipeHandler& pipeHandler)
	//	{
	//		pushProcessResolver(createMemoryProcessResolver(pipeHandler.m_mapProc));
	//	}
	//	~Auto()
	//	{
	//		popProcessResolver();
	//	}
	//} autoObj(pipeHandler);

	if (XMLUtils::parseFromString(pStr, nStrSize, pipeHandler))
	{
		std::vector<boost::intrusive_ptr<IProcess> > vecProc;
		for (std::map<ProcRef, boost::intrusive_ptr<IProcess> >::iterator it = pipeHandler.m_mapProc.begin();
			it != pipeHandler.m_mapProc.end();
			++it)
		{
			vecProc.push_back(it->second);
		}
		PipeUtils::pruneNonRoots(&vecProc);
		if (vecProc.size() == 1)
		{
			spPipe = vecProc.front();
		}
	}
	return spPipe;
}
