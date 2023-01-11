/******************************************************************************
trace.cpp

begin		: 2004-10-05
copyright	: derived from trace.cpp (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "trace.h"

// local includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <cassert>

//! Singleton.
Trace* Trace::m_pTrace = 0;

//! Global definition for callback function.
Trace::CallbackFunction Trace::m_pCallbackFunction = 0;

//! Global definition for callback user data.
void* Trace::m_pCallbackUserData = 0;

//! indicate that the trace instance has been destroyed.
bool bTraceDestroyed = false;

//! Definition for the static mutex object
boost::recursive_mutex Trace::m_mutex;

//! Prefixes for trace output
const std::string Trace::kstrErrorPrefix = "ERROR";
const std::string Trace::kstrTimePrefix = "TIME";
const std::string Trace::kstrInfoPrefix = "INFO";
const std::string Trace::kstrTestPrefix = "TEST";
const std::string Trace::kstrDebugPrefix = "DEBUG";
const std::string Trace::kstrMemoryPrefix = "MEMORY";
const std::string Trace::kstrNotifyPrefix = "NOTIFY";
const std::string Trace::kstrThreadPrefix = "THREAD";
const std::string Trace::kstrUIDebugPrefix = "UI_DEBUG";
const std::string Trace::kstrUIPrefix = "UI";

//! Properties
const std::string Trace::kstrScope = "Trace";
const std::string Trace::kstrLevel = "Level";
const std::string Trace::kstrLevelDesc = "Bit pattern that defines trace level (see Trace::eLevel).";

/*!
Send a trace message to cout if the trace level is high enough. Do not call
this method directly, use the macros defined in trace.h instead.

\param nLevel		The trace level.
\param strFile		The name of the file where the message originated.
\param nLine		The line in the file where the message originated.
\param strMessage	The message.
*/
void Trace::message(	eLevel nLevel,
						const std::string& strFile,
						int nLine,
						const std::string& strMessage	)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	static int messageNoFlush = 0;
	assert(Trace::getInstance() != 0);

	// trace to output stream, trace level is checked by macro
	std::ostream& out = Trace::getInstance()->getOutputStream();
	std::string strPrefix;
	bool forceFlush = false;

	switch (nLevel)
	{
		case knError:
			strPrefix = kstrErrorPrefix;
			forceFlush = true;
			break;

		case knTime:
			strPrefix = kstrTimePrefix;
			break;

		case knInfo:
			strPrefix = kstrInfoPrefix;
			break;

		case knTest:
			strPrefix = kstrTestPrefix;
			break;

		case knDebug:
			strPrefix = kstrDebugPrefix;
			break;

		case knMemory:
			strPrefix = kstrMemoryPrefix;
			break;

		case knNotify: 
			strPrefix = kstrNotifyPrefix; 
			break;

		case knThread:
			strPrefix = kstrThreadPrefix;
			break;

		case knUIDebug:
			strPrefix = kstrUIDebugPrefix;
			break;

		case knUI:
			strPrefix = kstrUIPrefix;
			break;

		default:
			assert(false && "Unknown trace type.");
			break;
	}

	// message format is "file(line) : prefix: message"
	std::ostringstream stream;
	stream << strFile << '(' << nLine << ") : ";
	if (!strPrefix.empty())
	{
		stream << strPrefix << ": ";
	}
	stream << strMessage;

	// send to the output stream 
	out << stream.str() << "\n";
	
	messageNoFlush++;
	
	if (forceFlush || messageNoFlush>50)
	{
		out.flush();
		messageNoFlush = 0;
	}

	// If the callback has been set, then we send a copy of the trace 
	//	output to that callback.
	if (m_pCallbackFunction != 0)
	{
		try
		{
    		m_pCallbackFunction(nLevel, stream.str(), m_pCallbackUserData);
		}
		catch (...)
		{
			// We eat any failure here.  This is likely a shutdown issue.
		}
	}
}	

/*!
Sets the call back function. (Usually called at app init.)

\param pCallback	The function which will be called every time a trace event occurs.
\param pUserData	Opaque pointer which will be returned to the callback function.
*/
void Trace::setCallback(CallbackFunction pCallback, void* pUserData)
{
	m_pCallbackFunction = pCallback;
	m_pCallbackUserData = pUserData;
}

/*!
Constructor creates a trace file called 'trace.log' in the working directory
of the application (AppServices). The default trace level is to write all 
forms of trace to the log (knAll).

\sa AppServices
*/
Trace::Trace() :
	m_nLevels(knAll),
	m_out()
{
	// Get the trace file path.
	boost::filesystem::path tracePath = AppServices::getTraceFilePath();

	// If there is already a file there, move/rename it.
	if (FileUtils::exists(tracePath))
	{
		// Construct a path for the trace from the previous run.
		boost::filesystem::path tracedPath(tracePath);
		tracedPath.replace_extension("old");

		// Delete any file in that location.
		if (FileUtils::exists(tracedPath))
		{
			try
			{
				boost::filesystem::remove(tracedPath);
			}
			catch (...)
			{
			}
		}

		// Rename the trace file.
		try
		{
			boost::filesystem::rename(tracePath, tracedPath);
		}
		catch (...)
		{
		}
	}

	// Create the new trace file.
	m_out.open(
		FileUtils::pathToString(tracePath).c_str(), 
		std::ios::out | std::ios::trunc);

	std::time(&m_time);
	m_clock = std::clock();
	m_out << "Log started: " << StringUtils::now(); // ctime adds newline
	m_out << std::endl;
}

/*!
Destructor
*/
Trace::~Trace()
{
	std::time(&m_time);
	m_clock = std::clock();
	m_out << std::endl;
	m_out << "Log ended: " << StringUtils::now(); // ctime adds newline
	m_out.flush();
}

// TODO: Make this thread-safe.  See ReferenceSphere::getInstance() as an example.
/*!
Get singleton instance. Once an instance of the class is created the exception
tracing is turned on.
*/
Trace* Trace::getInstance()
{
	if (0 == m_pTrace && !bTraceDestroyed)
	{
		m_pTrace = new Trace();
		PYXException::enableTrace();
	}

	return m_pTrace;
}

/*! force the Trace to flush pending message into disk */
void Trace::flush()
{		
	auto trace = Trace::getInstance();

	if (trace)
	{
		trace->getOutputStream().flush();		
	}
}

/*!
Stop tracing exceptions and destroy the trace instance. Once this method is called
the trace instance can not be re-acquired in this program run.
*/
void Trace::destroy()
{
	PYXException::disableTrace();
	delete m_pTrace;
	m_pTrace = 0;
	bTraceDestroyed = true;
}

/*!
Set the trace level for the application.  The method will output the new level
to the log and indicate available features through a TRACE_INFO call.  

\param nLevels	The new trace level
*/
void Trace::setLevels(unsigned int nLevels)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// set the trace level
	m_nLevels = nLevels;
	
	// write the new level out to the trace log
	traceLevel();
}

/*!
\return The timestamp as a string with trailing space.
*/
std::string Trace::getTimestampString()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::clock_t clockPrev = m_clock;
	m_clock = std::clock();
	std::string strNow(StringUtils::now().substr(11, 8));
	double fSeconds = static_cast<double>(m_clock - clockPrev) / CLK_TCK;

	// Format is "(hh:mm:ss -> n seconds): "
	std::string strResult("(");
	strResult.append(strNow);
	strResult.append(" -> ");
	strResult.append(StringUtils::toString(fSeconds));
	strResult.append(" seconds): ");

	return strResult;
}

/*!
Turn on a specific level of tracing. Using knNone as a parameter will have
no effect in this method.

\param nTraceLevel	The trace level to turn on.
*/
void Trace::traceOn(Trace::eLevel nTraceLevel)
{
	m_nLevels |= nTraceLevel;
	traceLevel();
}

/*! 
Turn off a specific level of tracing.  Using knNone as an parameter will
turn on all trace levels.

\param nTraceLevel	The trace level to turn off.
*/
void Trace::traceOff(Trace::eLevel nTraceLevel)
{
	m_nLevels &= ~nTraceLevel;
	traceLevel();
}

/*!
Print out the level of tracing to the trace stream. This method must be called
by a thread safe method since it is not thread safe on its own.
*/
void Trace::traceLevel() const
{
	// construct and output the trace level directly to the stream
	std::string strOutput =		"Trace level set to '" + 
								StringUtils::toString(m_nLevels) + 
								"'\r\n";
	strOutput += "Available trace values: ";
	if (0 != (knError & m_nLevels))
	{
		strOutput += kstrErrorPrefix + " ";
	}
	if (0 != (knTime & m_nLevels))
	{
		strOutput += kstrTimePrefix + " ";
	}
	if (0 != (knInfo & m_nLevels))
	{
		strOutput += kstrInfoPrefix + " ";
	}
	if (0 != (knTest & m_nLevels))
	{
		strOutput += kstrTestPrefix + " ";
	}
	if (0 != (knDebug & m_nLevels))
	{
		strOutput += kstrDebugPrefix + " ";
	}
	if (0 != (knMemory & m_nLevels))
	{
		strOutput += kstrMemoryPrefix + " ";
	}
	if (0 != (knNotify & m_nLevels))
	{
		strOutput += kstrNotifyPrefix + " ";
	}
	if (0 != (knThread & m_nLevels))
	{
		strOutput += kstrThreadPrefix + " ";
	}
	if (0 != (knUIDebug & m_nLevels))
	{
		strOutput += kstrUIDebugPrefix + " ";
	}
	if (0 != (knUI & m_nLevels))
	{
		strOutput += kstrUIPrefix + " ";
	}

	// send to the output stream
	std::ostream& out = Trace::getInstance()->getOutputStream();
	out << strOutput << std::endl;

	// If the callback has been set, then we send a copy of the trace 
	//	output to that callback.
	if (m_pCallbackFunction != 0)
	{
		m_pCallbackFunction(Trace::knInfo, strOutput, m_pCallbackUserData);
	}
}
