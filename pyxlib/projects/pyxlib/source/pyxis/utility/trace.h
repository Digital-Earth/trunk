#ifndef PYXIS__UTILITY__TRACE_H
#define PYXIS__UTILITY__TRACE_H
/******************************************************************************
trace.h

begin		: 2004-10-05
copyright	: derived from trace.h (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

/*
The trace calls are implemented as macros so we can get the file name and line
number and to avoid the overhead of a method call when trace is disabled.
*/
//! For internal use only....
#define TRACE_IMPL( LEVEL, F, L, EXPRESSION) \
	do {\
		if ((Trace::getInstance() != 0) && TRACE_ENABLED( LEVEL)) \
		{ \
    		std::ostringstream stream; \
			stream << EXPRESSION; \
			Trace::message( LEVEL, F, L, stream.str()); \
		}\
	} while (false)

//! For serious errors and exceptions.
#define TRACE_ERROR(str) \
	TRACE_IMPL( Trace::knError, __FILE__, __LINE__, str)

//! For timestamps.
#define TRACE_TIME(str) \
	TRACE_IMPL( Trace::knTime, __FILE__, __LINE__, Trace::getInstance()->getTimestampString() << str)

//! For general information messages, for example user actions, recoverable errors, and numeric results.
#define TRACE_INFO(str) \
	TRACE_IMPL( Trace::knInfo, __FILE__, __LINE__, str)

//! For execution tracing.
#define TRACE_TEST(str) \
	TRACE_IMPL( Trace::knTest, __FILE__, __LINE__, str)

//! For debugging, for example numeric results.
#define TRACE_DEBUG(str) \
	TRACE_IMPL( Trace::knDebug, __FILE__, __LINE__, str)

//! For memory manager tracing.
#define TRACE_MEMORY(str) \
	TRACE_IMPL( Trace::knMemory, __FILE__, __LINE__, str)

//! For notification attach, detach, notify tracing.
#define TRACE_NOTIFY(str) \
	TRACE_IMPL( Trace::knNotify, __FILE__, __LINE__, str)

//! For thread execution and lock tracing.
#define TRACE_THREAD(str) \
	TRACE_IMPL( Trace::knThread, __FILE__, __LINE__, str)

//! For tracking any form of user input, mouse clicks, key presses etc.
#define TRACE_UIDEBUG(str) \
	TRACE_IMPL( Trace::knUIDebug, __FILE__, __LINE__, str)

//! For tracking user input that results in a request or action.
#define TRACE_UI(str) \
	TRACE_IMPL( Trace::knUI, __FILE__, __LINE__, str)

//! Is a given trace level enabled. Use this method before entering a loop that consists only of TRACE statements
#define TRACE_ENABLED(nLevel) \
	(0 != (Trace::getInstance()->getLevels() & (nLevel)))

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
WHEN ADDING A NEW TRACE TYPE YOU MUST DO THE FOLLOWING THINGS:

Add the new type to eLevel
Add the cooresponding string for the new trace type
Add a macro (see above) for the new trace type
Add the new case to the 'Trace::message()' method
Add the new case to the 'Trace::traceLevel()' method
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
*/

/*!
The trace facility allows the program to specify the level of 
logging to perform.  Trace statements should always be 
created using the macros.  The trace level can  be set with
the traceOn(), traceOff() or setLevels().
*/
//! Manages application log output.
class PYXLIB_DECL Trace
{
public:

	/*!
	The overall trace level is established by perfoming a bitwise OR operation
	with 1 or more of the defined values.  
	Programming Note: Adding a new trace value here requires adding a new 
	output string, as well as handlers in setLevels() and message().
	*/
	//! Trace flags.
	enum eLevel
	{	
		knNone = 0x0000,	//!> No tracing variants are logged.
		knAll = 0xFFFF,		//!> All tracing variants are logged.
		knError = 0x0001,	//!> Serious errors and exceptions.
		knTime = 0x0002,	//!> Timestamps.
		knInfo = 0x0004,	//!> General information (minimal output).
		knTest = 0x0008,	//!> Automated testing information (moderate output).
		knDebug = 0x0010,	//!> Debugging info (lots of output).
		knMemory = 0x0020,	//!> Information related to the memory manager.
		knNotify = 0x0040,	//!> Notification messages (attach, detach, notify).
		knThread = 0x0080,	//!> Threading messages, locks, thread execution tracing.
		knUIDebug = 0x0100,	//!> Unfiltered input from the user.
		knUI = 0x0200		//!> User input interpreted as a request.
	};
	
	// Prefixes for trace output
	static const std::string kstrErrorPrefix;
	static const std::string kstrTimePrefix;
	static const std::string kstrInfoPrefix;
	static const std::string kstrTestPrefix;
	static const std::string kstrDebugPrefix;
	static const std::string kstrMemoryPrefix;
	static const std::string kstrNotifyPrefix;
	static const std::string kstrThreadPrefix;
	static const std::string kstrUIDebugPrefix;
	static const std::string kstrUIPrefix;
	
	// Properties
	static const std::string kstrScope;
	static const std::string kstrLevel;
	static const std::string kstrLevelDesc;

	//! Get singleton instance.
	static Trace* getInstance();

	/*!
	Return true if the trace facility has been initialized otherwise
	return false.
	*/
	//! Determine if the trace facility is initialized.
	static bool isInitialized() {return m_pTrace != 0;}

	//! Set the trace levels with a combination of flags. 
	void setLevels(unsigned int nLevels);

	//! Get the trace levels
	inline unsigned int getLevels() const {return m_nLevels;}

	//! Definition for a callback function.  (See setCallback for docs.)
	typedef void (*CallbackFunction)( 
		Trace::eLevel nTraceLevel, const std::string &strMessage, void* pUserData);

	//! Sets the call back function "OnTrace".  (Usually called at app init.)  
	static void setCallback(CallbackFunction pCallback, void* pUserData);

	//! Trace out the current trace level.
	void traceLevel() const;

	//! Turn on a specific level of tracing.
	void traceOn(Trace::eLevel nTraceLevel);

	//! Turn off a specific level of tracing.
	void traceOff(Trace::eLevel nTraceLevel);

/*
Never use these two methods (message() and getTimestampString()). Always use
the macro's defined at the top of this file.
*/
public:

	//! Write out a message to the stream. (USE THE MACRO, NOT THIS)
	static void message(	eLevel nLevel,
							const std::string& strFile,
							int nLine,
							const std::string& strMessage	);

	//! Get a timestamp string.
	std::string getTimestampString();

	//! flush the log onto disk
	static void flush();

	//! Destroy the trace instance.
	static void destroy();

private:

	//! Constructor
	Trace();

	//! Destructor
	~Trace();

	//! Not implemented
	Trace(const Trace&);

	//! Not implemented
	void operator=(const Trace&);

	//! Get the output stream.
	inline std::ostream& getOutputStream() {return m_out;}

private:

	//! The most recent timestamp
	std::time_t m_time;

	//! The clock to get the time from
	std::clock_t m_clock;

  	//! The current trace levels
  	unsigned int m_nLevels;

	//! Output stream.
	std::ofstream m_out;

private:

	//! A mutex to protect the file from multithreaded access.
	static boost::recursive_mutex m_mutex;

	//! Callback function.
	static CallbackFunction m_pCallbackFunction;

	//! Callback user data.
	static void* m_pCallbackUserData;

	//! Singleton
	static Trace* m_pTrace;

private:

	//! For initialization.
	friend class AppServices;
};

#endif // guard
