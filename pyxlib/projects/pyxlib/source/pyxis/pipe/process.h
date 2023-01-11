#ifndef PYXIS__PIPE__PROCESS_H
#define PYXIS__PIPE__PROCESS_H
/******************************************************************************
process.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/procs/exceptions.h"
#include "pyxis/pipe/parameter.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process_spec.h"
#include "pyxis/pipe/process_identity.h"
#include "pyxis/pipe/process_init_error.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/xml_utils.h"
#include "pyxis/geometry/geometry.h"

// boost includes
#include <boost/intrusive_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <iomanip>
#include <map>
#include <set>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Process events
////////////////////////////////////////////////////////////////////////////////

/*!
Process event base class.
*/
class PYXLIB_DECL ProcessEvent : public NotifierEvent
{
public:

	//! Returns the process that triggered the event.
	boost::intrusive_ptr<IProcess> getProcess()
	{
		return m_spProc;
	}

	//! Creator
	static PYXPointer<ProcessEvent> create(
		boost::intrusive_ptr<IProcess> spProc)
	{
		return PYXNEW(ProcessEvent, spProc);
	}

protected:

	ProcessEvent(boost::intrusive_ptr<IProcess> spProc) :
		m_spProc(spProc)
	{
	}

private:

	//! The process that triggered the event.
	boost::intrusive_ptr<IProcess> m_spProc;
};


class PYXLIB_DECL ProcessProcessingEvent : public ProcessEvent
{
public:
	static std::string Error;
	static std::string Processing;
	static std::string Fetching;
	static std::string Downloading;

public:	
	const std::string & getProcessingMessage() const 
	{
		return m_message;
	}

	static PYXPointer<ProcessProcessingEvent> create(const boost::intrusive_ptr<IProcess> & spProc,const std::string & message) 
	{
		return PYXNEW(ProcessProcessingEvent,spProc,message);
	}

	ProcessProcessingEvent(const boost::intrusive_ptr<IProcess> & spProc,const std::string & message) : ProcessEvent(spProc), m_message(message)
	{
	}

private:
	boost::intrusive_ptr<IProcess> m_proc;
	std::string m_message;
};

/*!
Process version event.
*/
class PYXLIB_DECL ProcessVersionEvent : public ProcessEvent
{
public:

	//! Creator
	static PYXPointer<ProcessVersionEvent> create(
		boost::intrusive_ptr<IProcess> spProc)
	{
		return PYXNEW(ProcessVersionEvent, spProc);
	}

private:

	ProcessVersionEvent(boost::intrusive_ptr<IProcess> spProc) :
		ProcessEvent(spProc)
	{
	}
};

////////////////////////////////////////////////////////////////////////////////
// Process interface
////////////////////////////////////////////////////////////////////////////////

/*!
A process is an abstract node in a pipeline. Processes have inputs, attributes and
data. When properly defined and initialized the process also has an output.
*/
//! A processing node in a pipeline.
struct PYXLIB_DECL IProcess : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! A value that indicates the most recent known initialization status of the process
	enum eInitStatus
	{
		knInitialized = 0,		//! The process and its children are properly initialized
		knNeedsInit,			//! The object has never been initialized or has changed since its last initialization.
		knFailedToInit,			//! The last attempt to initialize this process or its children failed
		knDoesNotMeetSpec,		//! The process does not have the required inputs
		knInitializing			//! The process is in the middle of initializing
	};

	//! Return the unique process identifer.
	virtual REFGUID STDMETHODCALLTYPE getProcID() const = 0;

	//! Return the version of the process.
	virtual int STDMETHODCALLTYPE getProcVersion() const = 0;

	//! The human readable name of the process.
	virtual const std::string& STDMETHODCALLTYPE getProcName() const = 0;

	//! Free form descriptive text.
	virtual const std::string& STDMETHODCALLTYPE getProcDescription() const = 0;

	//! The specification that define inputs and outputs for the process.
	virtual PYXPointer<ProcessSpec> STDMETHODCALLTYPE getSpec() const = 0;

	//! Get the output of the process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const = 0;

	//! Get the output of the process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() = 0;

	//! Return the number of parameters defined in the specification.
	virtual int STDMETHODCALLTYPE getParameterCount() const = 0;

	//! Return a specific parameter for the process.
	virtual PYXPointer<Parameter> STDMETHODCALLTYPE getParameter(int n) const = 0;

	//! Return all of the parameters for the process.
	virtual const std::vector<PYXPointer<Parameter> >& STDMETHODCALLTYPE getParameters() const = 0;

	//! Check for equality among processes.
	virtual bool STDMETHODCALLTYPE equals(boost::intrusive_ptr<IProcess> spProc) const = 0;

	//! Return a unique identifier for the in memory process.
	virtual int STDMETHODCALLTYPE getHashCode() const = 0;

	/*!
	Return an xml process identity node (see ProcessIdentity class for details).
	*/
	//! Return an XML node representing the identity of the process.
	virtual std::string STDMETHODCALLTYPE getIdentity() const = 0;

	/*
	Note: For a primer on XML Schema, see the local MSDN article at 
	ms-help://MS.VSCC.v80/MS.MSDN.v80/MS.WIN32COM.v10.en/dnxml/html/understandxsd.htm
	*/
	/*!Constructs the attribute description of the process in XSD string 
	format.

	\return   The XSD string describing the process' attributes
	*/
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const = 0;

	//! Return the map of name, value attribute pairs.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const = 0;

	//! Return the data block of the process.
	virtual std::string STDMETHODCALLTYPE getData() const = 0;

	//! Return the notifier for the process (not currently active).
	virtual Notifier& STDMETHODCALLTYPE getNotifier() = 0;

	//! DataChanged is fired whenever this process' data changes.
	virtual Notifier& STDMETHODCALLTYPE getDataChanged() = 0;

	// For serialization
	virtual void STDMETHODCALLTYPE setProcID(REFGUID procid) = 0;
	virtual void STDMETHODCALLTYPE setProcVersion(int nVersion) = 0;
	virtual void STDMETHODCALLTYPE setProcName(const std::string& strName) = 0;

	/*!
		The description can contain as much or as little information as the user desires.
	*/
	//! A brief description of the process.
	virtual void STDMETHODCALLTYPE setProcDescription(const std::string& strDescription) = 0;

	/*!
		Pass in a map of the complete complement of attributes for the process and set all
		of the values. Some processes support setting individual attribute values but that 
		is not currently general policy. POLICY UNDER REVIEW.
		
		\param mapAttr	The map of the full compliment of key value attribute pairs.
	*/
	//! Set the entire set of attributes for the process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr) = 0;

	/*!
		Set the value for the data payload of the process. Can hold anything that can
		serialized to string, however, processes are meant to be lightweight objects and
		as such, the data block should be kept to a reasonable (kilobytes) size.

		\param strData	The data to write to the data block.
	*/
	//! Set the text data block for the process
	virtual void STDMETHODCALLTYPE setData(const std::string& strData) = 0;

	/*!
		Set all of the parameters for the process from an input vector of parameters.

		\param vecParam	The entire complement of parameters to set in the process.
	*/
	//! Set the full complement of parameters for the process. 
	virtual void STDMETHODCALLTYPE setParameters(const std::vector<PYXPointer<Parameter> >& vecParam) = 0;

	/*!
		This method attempts to initialize this particular process. The method should
		be able to respond very quickly when the state is already known. This means
		that the init call can be called repeatedly to check the initializaiton status.
		This method should never throw an exception.

		\return	The current initialzation status of the process.
	*/
	//! Initialize the process so that it can successfully provide a valid output.
	virtual eInitStatus STDMETHODCALLTYPE initProc(bool bRecursive = false) = 0;

	/*!
		Reinitialize a process by setting its init status to knNeedsInit and then calling initProc().
		  If bRecursive is set to 'true', then first all input processes are reinitialized.

		\return The current initialization status of the process.
	*/
	//! Reinitialize a process, and maybe its inputs by setting the init status to knNeedsInit and then calling initProc().
	virtual eInitStatus STDMETHODCALLTYPE reinitProc(bool bRecursive = false) = 0;

	/*!
		When an error has occurred during intializaiton (state is knFailedToInit or
		knDoesNotMeetSpec) this method will be able to return an object describing 
		the initialization failure. If the process is not in an error state this 
		method will return an empty smart pointer. Callers should consider multi
		threaded access to this process when making this call and mutex accordingly.
	*/
	//! When an error has occurred, return the error information object.
	virtual boost::intrusive_ptr<const IProcessInitError> STDMETHODCALLTYPE getInitError() const = 0;

	//! Return the latest initialization state.
	virtual eInitStatus STDMETHODCALLTYPE getInitState() const = 0;

	//! Sets whether a process finalizes. Returns old setting. This API subject to change!
	virtual bool STDMETHODCALLTYPE setFinalize(bool bFinalize) = 0;
};

//! The event type that gets fired whenever a process's data changes.  
class PYXLIB_DECL ProcessDataChangedEvent : public NotifierEvent
{
public:
	enum ChangeTrigger
	{
		knInputDataChange, //the input was changed - this means we need to regenrate the data
		knNewTileDownloaded //a new tile was calculated (become available) but the input data is the same - just add the data
	};

public:
	static PYXPointer< ProcessDataChangedEvent > create(
		boost::intrusive_ptr<IProcess> process, 
		PYXPointer<PYXGeometry> geometry = PYXPointer<PYXGeometry>(),
		ChangeTrigger trigger = knInputDataChange)
	{
		PYXPointer< ProcessDataChangedEvent > result = PYXNEW(ProcessDataChangedEvent);
		result->m_process = process;
		result->m_geometry = geometry;
		result->m_trigger = trigger;
		return result;
	}

	//! The process that has changed.
	boost::intrusive_ptr<IProcess> getProcess()
	{   return m_process;}

	//! The area in which the change occurred.  (NULL for unknown/everywhere.)
	PYXPointer<PYXGeometry> getGeometry()
	{   return m_geometry;}

	ChangeTrigger getDataChangeTrigger()
	{   return m_trigger; }

protected:
	explicit ProcessDataChangedEvent() {};

	//! The process that has changed.
	boost::intrusive_ptr<IProcess> m_process;

	//! The area in which the change occurred.  (NULL for unknown/everywhere.)
	PYXPointer<PYXGeometry> m_geometry;

	//! The trigger for data change event
	ChangeTrigger m_trigger;
};

////////////////////////////////////////////////////////////////////////////////
// Process references
////////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL ProcRef
{
public:

	//! Create a null process reference.
	explicit
	ProcRef() :
		m_procid(GUID_NULL),
		m_nProcVer(0)
	{
	}

	//! Create a process reference from an existing process.
	explicit
		ProcRef(const boost::intrusive_ptr<IProcess> & spProc)
	{
		if (spProc)
		{
			m_procid = spProc->getProcID();
			m_nProcVer = spProc->getProcVersion();
		}
		else
		{
			m_procid = GUID_NULL;
			m_nProcVer = 0;
		}
	}

	//! Create a process reference as specified.
	explicit
	ProcRef(	REFGUID procid,
				int nProcVer	) :
		m_procid(procid),
		m_nProcVer(nProcVer)
	{
	}

public:

	REFGUID getProcID() const
	{
		return m_procid;
	}

	int getProcVersion() const
	{
		return m_nProcVer;
	}

private:
	GUID m_procid;
	int m_nProcVer;
};

//! Equality operator for process references.
inline
bool operator ==(const ProcRef& lhs, const ProcRef& rhs)
{
	return lhs.getProcID() == rhs.getProcID() && lhs.getProcVersion() == rhs.getProcVersion();
}

//! Inequality operator for process references.
inline
bool operator !=(const ProcRef& lhs, const ProcRef& rhs)
{
	return lhs.getProcID() != rhs.getProcID() || lhs.getProcVersion() != rhs.getProcVersion();
}

//! Less operator for process references.
inline
bool operator <(const ProcRef& lhs, const ProcRef& rhs)
{
	return lhs.getProcID() < rhs.getProcID() ||
		lhs.getProcID() == rhs.getProcID() && lhs.getProcVersion() < rhs.getProcVersion();
}

/*!
Example of canonical form: {12345678-ABCD-1234-ABCD-12345678ABCD}[1234]
*/
//! Output a procref in canonical form.
inline
std::ostream& operator <<(std::ostream& out, const ProcRef& procref)
{
	return out << procref.getProcID() << '[' << procref.getProcVersion() << ']';
}

//! Output the name and procref for a process in a readable form
inline std::ostream& operator <<(std::ostream& out, const IProcess& proc)
{
	return out << proc.getProcName() << " - " << 
		ProcRef(proc.getProcID(), proc.getProcVersion());
}

/*!
Example of canonical form: {12345678-ABCD-1234-ABCD-12345678ABCD}[1234]
*/
//! Input a procref in canonical form.
inline
std::istream& operator >>(std::istream& in, ProcRef& procref)
{
	GUID procid;
	int nProcVer;
	in >> procid;
	in.get(); // should be '['
	in >> nProcVer;
	in.get(); // should be ']'
	procref = ProcRef(procid, nProcVer);
	return in;
}

//! Convert a process reference to a string.
inline
std::string procRefToStr(const ProcRef& procref)
{
	std::ostringstream out;
	out << procref;
	return out.str();
}

//! Convert a string to a process reference.
inline
ProcRef strToProcRef(const std::string& str)
{
	ProcRef procref;
	std::istringstream in(str);
	in >> procref;
	return procref;
}


////////////////////////////////////////////////////////////////////////////////
// Process life time tracking
////////////////////////////////////////////////////////////////////////////////


/*!
An indication that a process is alive (weak reference).
This class can be used as diagnostic tool for memory management.
Also, the getProcRef() can be used with PipeManager to recover a pointer to the process if needed.
*/
//! An indication that a process is alive (weak reference)
class PYXLIB_DECL ProcessLifeTimeIndication : public PYXObject
{
private:
	std::string m_name;
	ProcRef m_procRef;
	PYXPointer<ProcessSpec> m_spec;

public:
	static inline PYXPointer<ProcessLifeTimeIndication> create(const IProcess & process) 
	{
		return PYXNEW(ProcessLifeTimeIndication,process);
	}

private:
	ProcessLifeTimeIndication(const IProcess & process);

public:
	const std::string & getProcName() const;
	const ProcRef & getProcRef() const;
	const PYXPointer<ProcessSpec> & getProcSpec() const;
};

/*!
This object maintain a weak reference list for all tracker processes.
The goal of this class is to allow developers peek the system to see which process are alive in memory.

This class works with ProcessImpl template to keep track on all processes.
*/
//! a singleton that provides a snapshot of all alive processes in memory.
class PYXLIB_DECL ProcessLifeTimeTracker
{
public:
	void notifyProcessCreated(const IProcess & process);
    void notifyProcessDestoryed(const IProcess & process);

	int getTrackedProcessCount();
	
	std::vector< PYXPointer< ProcessLifeTimeIndication > > getTrackedProcesses();

public:
	static ProcessLifeTimeTracker & getInstance();

private:
	std::set<const IProcess *>    m_trackedProcesses;
	boost::recursive_mutex m_mutex;
};

////////////////////////////////////////////////////////////////////////////////
// Process base class
////////////////////////////////////////////////////////////////////////////////

//! Helper class for implementing processes.
template <typename PROC>
class ProcessImpl : public IProcess
{
public:

	typedef boost::intrusive_ptr<PROC> Pointer;

protected:

	ProcessImpl( const std::string &procName, const std::string &procDescription) :
		 m_notifier("Process: " + procName),
	     m_dataChanged("Data Changed in process " + procName),
		 m_initState(knNeedsInit)
	{
		initialize();
		setProcName(procName);
		setProcDescription(procDescription);
	}

	ProcessImpl() :
		 m_initState(knNeedsInit)
	{
		initialize();
		setProcName(PROC::getSpecStatic()->getName());
		setProcDescription(PROC::getSpecStatic()->getDescription());
		m_notifier.setNotifierName("Process: " + getProcName());
		m_dataChanged.setNotifierName("Data Changed in process " + getProcName());
	}

private:

	void initialize()
	{
		// Initialize the ID and version.
		PYXCOMCreateGuid(&m_procid);
		m_nVersion = 1;
		m_bFinalize = true;

		// Set the (empty) parameters from the spec
		PYXPointer<ProcessSpec> spProcSpec = PROC::getSpecStatic();

		initalizeParamertersFromSpec(spProcSpec);
        
        ProcessLifeTimeTracker::getInstance().notifyProcessCreated(*this);
	}

protected:
	void initalizeParamertersFromSpec(const PYXPointer<ProcessSpec> & spProcSpec)
	{
		m_vecParam.clear();

		for (int n = 0; n != spProcSpec->getParameterCount(); ++n)
		{
			m_vecParam.push_back(Parameter::create(spProcSpec->getParameter(n)));
		}
		attachObserversToInputs();
	}

public: // IProcess

	virtual REFGUID STDMETHODCALLTYPE getProcID() const
	{
		//NOTE: m_procRefMutex is been used and not m_procMutex.
		//we do so to allow creating new ProcRef(process) without generating a deadlock if the process
		//now locked itself due to initialization.
		boost::recursive_mutex::scoped_lock lock(m_procRefMutex);
		return m_procid;
	}

	virtual int STDMETHODCALLTYPE getProcVersion() const
	{
		//NOTE: m_procRefMutex is been used and not m_procMutex.
		//we do so to allow creating new ProcRef(process) without generating a deadlock if the process
		//now locked itself due to initialization.
		boost::recursive_mutex::scoped_lock lock(m_procRefMutex);
		return m_nVersion;
	}

	virtual const std::string& STDMETHODCALLTYPE getProcName() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_strName;
	}

	virtual const std::string& STDMETHODCALLTYPE getProcDescription() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_strDescription;
	}

	virtual int STDMETHODCALLTYPE getParameterCount() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return static_cast<int>(m_vecParam.size());
	}

	virtual PYXPointer<Parameter> STDMETHODCALLTYPE getParameter(int n) const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_vecParam[n];
	}

	virtual const std::vector<PYXPointer<Parameter> >& STDMETHODCALLTYPE getParameters() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_vecParam;
	}

	virtual bool STDMETHODCALLTYPE equals(boost::intrusive_ptr<IProcess> spProc) const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return this == spProc.get();
	}

	virtual int STDMETHODCALLTYPE getHashCode() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
#pragma warning(push)
#pragma warning(disable: 4311) // warning C4311: 'reinterpret_cast' : pointer truncation
		// Yes this will truncate a pointer if compiled as 64-bit. It's OK in this case.
		return reinterpret_cast<int>(this);
#pragma warning(pop)
	}

	virtual std::string STDMETHODCALLTYPE getIdentity() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		PYXPointer<ProcessSpec> spSpec = getSpec();
		assert(spSpec);

		ProcessIdentity identity(spSpec->getClass());
		identity.setData(getData());
		identity.setAttributes(getAttributes());

		const int nParameterCount = getParameterCount();
		for (int n = 0; n < nParameterCount; ++n)
		{
			PYXPointer<Parameter> spParam = getParameter(n);
			assert(spParam);
			identity.addInput(*spParam);
		}

		return identity();
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const
	{
		// default returns a schema of string attributes
		std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\""
			" elementFormDefault=\"qualified\""
			" xmlns=\"http://tempuri.org/XMLSchema.xsd\""
			" xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\""
			" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
				"<xs:element name=\"GenericProcessSchema\">"
					"<xs:complexType>"
						"<xs:sequence>";

		std::map<std::string, std::string> mapAttr = getAttributes();
		std::map<std::string, std::string>::iterator it;

		for (it = mapAttr.begin(); it != mapAttr.end(); ++it)
		{
			strXSD += "<xs:element name=\"" + it->first + "\" type=\"xs:string\">";
			strXSD += "<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>" + it->first + "</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>";
		}
		strXSD += "</xs:sequence></xs:complexType></xs:element></xs:schema>";
		return strXSD;
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		// Default does nothing
		return std::map<std::string, std::string>();
	}

	virtual std::string STDMETHODCALLTYPE getData() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_strData;
	}

	virtual Notifier& STDMETHODCALLTYPE getNotifier()
	{
		return m_notifier;
	}

	virtual Notifier& STDMETHODCALLTYPE getDataChanged()
	{
		return m_dataChanged;
	}

public:

	virtual void STDMETHODCALLTYPE setProcID(REFGUID procid)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		{
			//NOTE: see comment in getProcID
			boost::recursive_mutex::scoped_lock lock(m_procRefMutex);
			m_procid = procid;
		}		
		m_initState = knNeedsInit;
	}

	virtual void STDMETHODCALLTYPE setProcVersion(int nVersion)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		{
			//NOTE: see comment in getProcVersion
			boost::recursive_mutex::scoped_lock lock(m_procRefMutex);
			m_nVersion = nVersion;
		}		
		m_initState = knNeedsInit;
	}

	virtual void STDMETHODCALLTYPE setProcName(const std::string& strName)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		m_strName = strName;
		m_initState = knNeedsInit;
	}

	virtual void STDMETHODCALLTYPE setProcDescription(const std::string& strDescription)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		m_strDescription = strDescription;
		m_initState = knNeedsInit;
	}

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr)
	{
		/*
		By default does nothing. Any implementation of this method needs to set the init state
		of the process when appropriate. If the process needs to be re-initialized due to the 
		changes that were made set it like so:
			m_initState = knNeedsInit;
		*/
	}

	virtual void STDMETHODCALLTYPE setData(const std::string& strData)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		m_strData = strData;
		m_initState = knNeedsInit;
	}

	virtual void STDMETHODCALLTYPE setParameters(const std::vector<PYXPointer<Parameter> >& vecParam)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		// remove the observers from the current parameters
		detachObserversFromInputs();

		m_initState = knNeedsInit;
		m_vecParam = vecParam;

		// attach to the new parameters
		attachObserversToInputs();
	}

	virtual eInitStatus STDMETHODCALLTYPE initProc(bool bRecursive = false)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		// clear out the current error state
		m_spInitError = boost::intrusive_ptr<IProcessInitError>();

		// process all of the children first
		if (bRecursive)
		{
			// cycle through each of the children
			std::vector<PYXPointer<Parameter> >::iterator it = m_vecParam.begin();
			int nFailCount = 0;
			for (; it != m_vecParam.end(); ++it)
			{
				for (int n = 0; n < (*it)->getValueCount(); ++n)
				{
					assert((*it)->getValue(n));
					if ((*it)->getValue(n)->initProc(true) != knInitialized)
					{
						++nFailCount;
					}
				}
			}

			// Check the failure count and set return the error when appropriate
			if (nFailCount != 0)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new InputInitError());
				m_initState = knFailedToInit;
				return m_initState;
			}
		}

		// if the process is already initialized, return
		if (m_initState != knInitialized)
		{
			if (!verifySpec(this))
			{
				m_initState = knDoesNotMeetSpec;
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new ProcSpecFailure());
				return m_initState;
			}

			try
			{
				// perform the actual initialization and examine the state
				m_initState = initImpl();	
				if (m_initState == knFailedToInit)
				{
					if (!m_spInitError)
					{
						TRACE_ERROR(
							"A process failed to initialize but no error state was set, creating generic error for " << 
							getProcName());
						m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
						m_spInitError->setError(
							"Generic process initialization error.");
					}
				}
				else if (m_initState != knInitialized)
				{
					TRACE_ERROR("Process initialization should only return pass or fail, Policy Breach!");
					assert(false && "Process initialization should only return pass or fail, Policy Breach!");
				}
			}
			catch (PYXException& e)
			{
				TRACE_ERROR("A PYXIS exception was thrown during process initialization. Processes should never throw during init.");
				TRACE_ERROR("Exception details: " << e.getLocalizedErrorString() << "\n" << e.getFullErrorString());
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError(e.getLocalizedErrorString() + "\n" + e.getFullErrorString());
				m_initState = knFailedToInit;
			}
			catch (...)
			{
				TRACE_ERROR("A generic exception occurred during process initialization for: " << getProcName());
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError(
					"A generic error occurred during process initialization.");	
				m_initState = knFailedToInit;
			}
			assert(	(m_initState == knInitialized || m_spInitError) && 
					"Policy Breach! invalid error and state combination.");
		}
		return m_initState;
	}

	virtual eInitStatus STDMETHODCALLTYPE reinitProc(bool bRecursive = false)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		m_initState = knNeedsInit;

		if (bRecursive)
		{
			std::vector<PYXPointer<Parameter> >::iterator it = m_vecParam.begin();
			for (; it != m_vecParam.end(); ++it)
			{
				for (int n = 0; n < (*it)->getValueCount(); ++n)
				{
					assert((*it)->getValue(n));
					(*it)->getValue(n)->reinitProc(true);
				}
			}
		}

		return initProc(false);
	}

	virtual eInitStatus STDMETHODCALLTYPE getInitState() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_initState;
	}

	virtual boost::intrusive_ptr<const IProcessInitError> STDMETHODCALLTYPE 
		getInitError() const
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		return m_spInitError;
	}

	virtual bool STDMETHODCALLTYPE setFinalize(bool bFinalize)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		bool bOld = m_bFinalize;
		m_bFinalize = bFinalize;
		return bOld;
	}

public:

	//! Called before garbage collection to notify PipeManager.
	void finalize()
	{
		// remove the observers from the old parameter
		detachObserversFromInputs();
        
        // notify life time tracker we done. 
        ProcessLifeTimeTracker::getInstance().notifyProcessDestoryed(*this);

		if (m_bFinalize)
		{
			PipeManager::notifyFinalize(this);
		}
	}

protected:

	/*!
		The logical initialization that prepares the process for successfully
		providing its output. If this method is called it can be assumed that 
		the process has already been verified to meet its specification and
		that all child processes have been initialized. The only values that \
		should ever be returned by this method are either knInitialized
		or knFailedToInit.

		\return An indication of successful or failed initialization.
	*/
	//! The logical initialization code of the process.
	virtual eInitStatus initImpl()
	{
		return knInitialized;
	}

	void incProcVersion(bool bNotify = true)
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);
		++m_nVersion;
		if (bNotify)
		{
			m_notifier.notify(ProcessVersionEvent::create(this));
			onDataChanged();
		}
	}

	template<class T>
	void setInitProcError(const std::string & error)
	{
		//TRACE_ERROR("Process Failed to init(" << getProcName() << ":" << ProcRef(getProcID(),getProcVersion()) << " ) :" << error);
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new T());
		m_spInitError->setError(error);
	}

	// The callback for any change notifier
	void parameterChangeCallback(PYXPointer<NotifierEvent> spEvent)
	{
		m_initState = knNeedsInit;

		boost::intrusive_ptr<ParameterEvent> spParameterEvent = boost::dynamic_pointer_cast<ParameterEvent>(spEvent);

		if (spParameterEvent)
		{
			for(std::vector<boost::intrusive_ptr<IProcess>>::iterator it = spParameterEvent->removedValues.begin();
				it != spParameterEvent->removedValues.end();
				++it)
			{
				(*it)->getDataChanged().detach( this, &ProcessImpl<PROC>::handleInputDataChanged);
			}

			for(std::vector<boost::intrusive_ptr<IProcess>>::iterator it = spParameterEvent->addedValues.begin();
				it != spParameterEvent->addedValues.end();
				++it)
			{
				(*it)->getDataChanged().attach( this, &ProcessImpl<PROC>::handleInputDataChanged);
			}
		}
	}

	virtual void notifyProcessing(const std::string & message) const
	{
		const IProcess * const proc = dynamic_cast<const IProcess * const>(this);
		PipeManager::getProcessProcessingNotifier()->notify(ProcessProcessingEvent::create(const_cast<IProcess*>(proc) ,message));
	}

	//! The current initialization state for the object.
	IProcess::eInitStatus m_initState;

	//! The data for the process' data block
	std::string m_strData;

	//! A pointer to the error that occurred during initialization
	boost::intrusive_ptr<IProcessInitError> m_spInitError;

	//! The process mutex
	mutable boost::recursive_mutex m_procMutex;

	//! The process procRef settings mutex
	mutable boost::recursive_mutex m_procRefMutex;

	//! Utility function to raise a DataChanged event.
	virtual void onDataChanged(PYXPointer<PYXGeometry> geometry = PYXPointer<PYXGeometry>(),
							   ProcessDataChangedEvent::ChangeTrigger trigger = ProcessDataChangedEvent::knInputDataChange)
	{
		m_dataChanged.notify(ProcessDataChangedEvent::create(this, geometry, trigger));
	}

	//! Handler for any input changing its data.  
	virtual void handleInputDataChanged(PYXPointer<NotifierEvent> eventData)
	{
		PYXPointer<ProcessDataChangedEvent> processDataChangedEvent =
			boost::dynamic_pointer_cast<ProcessDataChangedEvent>(eventData);

		// By default, we simply pass the event on to our observers.
		onDataChanged(processDataChangedEvent->getGeometry(),processDataChangedEvent->getDataChangeTrigger());
	}

	//! Helper function.  Detaches observers from inputs.
	void detachObserversFromInputs()
	{
		for (int n = 0; n < static_cast<int>(m_vecParam.size()); ++n)
		{			
			m_vecParam[n]->getChangeNotifier().detach(this, &ProcessImpl<PROC>::parameterChangeCallback);

			for (int v = 0; v < static_cast<int>(m_vecParam[n]->getValueCount()); ++v)
			{
				boost::intrusive_ptr<IProcess> inputProcess = m_vecParam[n]->getValue( v);
				if (inputProcess != NULL)
				{
					inputProcess->getDataChanged().detach( 
						this, &ProcessImpl<PROC>::handleInputDataChanged);
				}
			}
		}
	}

	//! Helper function.  Attaches observers to inputs.
	void attachObserversToInputs()
	{
		for (int n = 0; n < static_cast<int>(m_vecParam.size()); ++n)
		{
			m_vecParam[n]->getChangeNotifier().attach(this, &ProcessImpl<PROC>::parameterChangeCallback);

			for (int v = 0; v < static_cast<int>(m_vecParam[n]->getValueCount()); ++v)
			{
				boost::intrusive_ptr<IProcess> inputProcess = m_vecParam[n]->getValue( v);
				if (inputProcess != NULL)
				{
					inputProcess->getDataChanged().attach( 
						this, &ProcessImpl<PROC>::handleInputDataChanged);
				}
			}
		}
	}


private:

	/*!
		Once created, the process id never changes. Should any of the inputs or
		attributes change, the version would be altered but the process id would
		remain constant. The version in combination with the process identifier 
		is a unique identifier for this process.
	*/
	//! The process identifier for this object.
	GUID m_procid;

	/*!
		The uniqe version for this particular process object. Any changes to attributes
		or inputs should produce a new version of this process. The version in combination
		with the process identifier is a unique identifier for this process.
	*/
	//! The version of the process.
	int m_nVersion;

	//! The input parameters of the process
	std::vector<PYXPointer<Parameter> > m_vecParam;

	//! The user defined name of the process
	std::string m_strName;

	//! The user defined description of the process
	std::string m_strDescription;

	//! The notification object for this process.
	Notifier m_notifier;

	//! DataChanged is fired whenever the data in this process changes.
	Notifier m_dataChanged;

	//! An indication to help with object lifetime and cleanup. 
	bool m_bFinalize;
};

////////////////////////////////////////////////////////////////////////////////
// Process misc
////////////////////////////////////////////////////////////////////////////////

//! Helper macro for implementing process getSpec (use in public section of class).
#define IPROCESS_GETSPEC_IMPL() \
public: \
	static PYXPointer<ProcessSpec> getSpecStatic() \
	{ \
		static PYXPointer<ProcessSpec> spProcSpec(ProcessSpec::create( \
			clsid, \
			std::vector<IID>(), \
			std::vector<PYXPointer<ParameterSpec> >(), \
			"", \
			""	)); \
		return spProcSpec; \
	}; \
	virtual PYXPointer<ProcessSpec> STDMETHODCALLTYPE getSpec() const \
	{ \
		return getSpecStatic(); \
	}

/*!
Process class, name description, then list the output interfaces.
*/
//! Helper macro for process specs.
#define IPROCESS_SPEC_BEGIN(clazz, name, desc, category, ...) \
	namespace \
	{ \
		struct ProcessSpecInit_##clazz \
		{ \
			ProcessSpecInit_##clazz(); \
		} processSpecInit##clazz; \
		ProcessSpecInit_##clazz::ProcessSpecInit_##clazz() \
		{ \
			const IID aiid[] = { __VA_ARGS__ }; \
			const int niid = sizeof(aiid)/sizeof(aiid[0]); \
			PYXPointer<ProcessSpec> spProcSpec = clazz::getSpecStatic(); \
			std::vector<IID> vecIID; \
			for (int nCount = 0; nCount < niid; ++nCount) \
				vecIID.push_back(aiid[nCount]); \
			spProcSpec->setOutputIIDs(vecIID); \
			spProcSpec->setName(name); \
			spProcSpec->setDescription(desc); \
			spProcSpec->setCategory(category); \
			std::vector<PYXPointer<ParameterSpec> > vecParamSpec;

//! Helper macro for process specs.
#define IPROCESS_SPEC_PARAMETER(iid, min, max, name, desc) \
	vecParamSpec.push_back(ParameterSpec::create(iid, min, max, name, desc));

//! Helper macro for process specs.
#define IPROCESS_SPEC_END \
			spProcSpec->setParameterSpecs(vecParamSpec); \
		} \
	}

/*!
 Helper macro to support classes that multiply derive from PYXCOM_IUnknown (which is fairly 
 common in the world of COM/PYXCOM).  Allows the boost::intrusive_ptr to determine 
 a single PYXCOM_IUnknown to call into.  Specify one of the base classes that is the 
 "preferred route" back to PYXCOM_IUnknown.  Note that BASE_CLASS must not inherit from
 PYXCOM_IUnknown multiple times.
 */
#define IUNKNOWN_DEFAULT_CAST(THIS_CLASS, BASE_CLASS) \
class ForceMeaningfulError ## THIS_CLASS ## BASE_CLASS: public BASE_CLASS{}; \
	friend inline void intrusive_ptr_add_ref(const THIS_CLASS* p) \
	{ \
		static_cast<PYXCOM_IUnknown*>( \
			static_cast< BASE_CLASS *>( \
			const_cast< THIS_CLASS *>(p)))->AddRef(); \
	} \
	\
	friend inline void intrusive_ptr_release(const THIS_CLASS* p) \
	{ \
		static_cast<PYXCOM_IUnknown*>( \
			static_cast< BASE_CLASS *>( \
			const_cast< THIS_CLASS *>(p)))->Release(); \
	}


#define UPDATE_PROCESS_ATTRIBUTE(MAP_ATTRIBUTES,NAME,TYPE,MEMBER) \
	{ \
		std::map<std::string, std::string>::const_iterator __IT = (MAP_ATTRIBUTES).find((NAME)); \
		if (__IT != (MAP_ATTRIBUTES).end()) \
		{ \
			(MEMBER) = StringUtils::fromString<TYPE>(__IT->second); \
		} \
	} \

#define UPDATE_PROCESS_ATTRIBUTE_STRING(MAP_ATTRIBUTES,NAME,MEMBER) \
	{ \
		std::map<std::string, std::string>::const_iterator __IT = (MAP_ATTRIBUTES).find((NAME)); \
		if (__IT != (MAP_ATTRIBUTES).end()) \
		{ \
			(MEMBER) = (__IT->second); \
		} \
	} \


/*!
Determine if all of the required parameter values required by the specification 
are present in the passed process. This does not mean that the process or input
processes can be initialized, only that it meets spec.

\param spProc	The process to examine.

\return true if the process 
*/
bool PYXLIB_DECL verifySpec(boost::intrusive_ptr<IProcess> spProc);

#endif // guard
