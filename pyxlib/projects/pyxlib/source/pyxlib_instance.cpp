/******************************************************************************
pyxlib_instance.cpp

begin		: 2006-11-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxlib_instance.h"

// pyxlib includes
#include "pyxis/derm/derm_utils.h"
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/spiral_iterator.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/polygon.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/procs/proc_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/memory_manager.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/utility/xml_utils.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/geometry/inner_tile.h"

#if _WINDOWS
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition

// windows includes
#include <tchar.h>
#include <windows.h>

#pragma warning(pop)
#endif

//! Generic default application name.
static const std::string kstrAppName = "PYXIS Application";


#ifdef INSTANCE_COUNTING
InstanceCounter::InstanceCountMap s_snapshot1;
#endif


/*!
The reference count.
*/
boost::detail::atomic_count PYXLibInstance::m_nCount(0L);

/*!
Initialize the library for use, if not already initialized. If an empty
application name is provided, a default name is used.

\param strAppName			The application name that will appear in messages 
							and logging.
\param bClearCache			If true, any existing cache is cleared
\param strWorkingDirectory	The directory for all files and directories (cache,
							temp, logs etc) that are created by the application.
\param strAppDirectory		The directory where the WorldView exe resides.
*/
PYXLibInstance::PYXLibInstance(	const std::string& strAppName,
								bool bClearCache,
								const std::string& strWorkingDirectory, 
								const std::string& strAppDirectory,
								const std::string& strCacheDir )
{
	if (strAppName.empty())
	{
		initialize(kstrAppName, bClearCache, strWorkingDirectory, strAppDirectory, strCacheDir);
	}
	else
	{
		initialize(strAppName, bClearCache, strWorkingDirectory, strAppDirectory, strCacheDir);
	}
}

/*!
Clean up the library after use, if not already cleaned up.
*/
PYXLibInstance::~PYXLibInstance()
{
	uninitialize();
}

/*!
Initialize the SDK for use, if not already initialized.

\param strAppName			The application name that will appear in messages 
							and logging.
\param bClearCache			If true, any existing cache is cleared
\param strWorkingDirectory	The directory for all files and directories (cache,
							temp, logs etc) that are created by the application.
\param strAppDirectory		The directory where the WorldView exe resides.
*/
void PYXLibInstance::initialize(	const std::string& strAppName,
									bool bClearCache,
									const std::string& strWorkingDirectory, 
									const std::string& strAppDirectory,
									const std::string& strCacheDir )
{
	if (1 == ++m_nCount)
	{
		//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

		initUtility(strAppName, bClearCache, strWorkingDirectory, strAppDirectory, strCacheDir);

#ifdef INSTANCE_COUNTING
		s_snapshot1 = InstanceCounter::takeSnapShot();
#endif

		initDerm();
		initGeometry();
		initPipe();
		initProcs();
	}
	else
	{
		TRACE_DEBUG(	"PYXLibInstance already initialized with application name '" << 
						AppServices::getAppName() << "'. New name of '" <<
						strAppName << "' Ignored and reference count incremented to '" <<
						m_nCount << "'.");
	}
}

/*!
Clean up the library after use, if not already cleaned up.
*/
void PYXLibInstance::uninitialize()
{
	if (0 == --m_nCount)
	{
		TRACE_INFO("About to stop thread pool");
		PYXThreadPool::shutdown();

		TRACE_INFO("About to clean up PYXLib, including 'Trace'");
		uninitProcs();
		uninitPipe();
		uninitGeometry();
		uninitDerm();

#ifdef INSTANCE_COUNTING
		TRACE_INFO("Checking memleaks using InstanceCounter snapshot:");
		InstanceCounter::traceObjectCountChange(s_snapshot1,InstanceCounter::takeSnapShot());
#endif

		uninitUtility();
	}
	else
	{
		TRACE_DEBUG(	"PYXLib reference count decremented to '" <<
						m_nCount << "'."	);	
	}
}

//! Initialize the procs module
void PYXLibInstance::initProcs()
{
	TRACE_INFO("Initializing procs module.");
	ProcUtils::initialize();
}

//! Clean up the procs module
void PYXLibInstance::uninitProcs()
{
	TRACE_INFO("Uninitializing procs module.");
	ProcUtils::uninitialize();
}

//! Initialize the pipe module
void PYXLibInstance::initPipe()
{
	TRACE_INFO("Initializing PIPE module.");
	PipeManager::initialize();
}

//! Clean up the pipe module
void PYXLibInstance::uninitPipe()
{
	TRACE_INFO("Uninitializing PIPE module.");
	PipeManager::uninitialize();
}

/*!
Initialize the derm module.
*/
void PYXLibInstance::initDerm()
{
	TRACE_INFO("Initializing derm module.");
	DermUtils::initialize();
	MemoryManager::initStaticData();
	WGS84::initStaticData();
	PYXMath::initStaticData();
	Icosahedron::initStaticData();
	PYXIcosMath::initStaticData();
	ReferenceSphere::initStaticData();
	SnyderProjection::initStaticData();
	PYXSpiralIterator::initStaticData();
}

/*!
Destroy the derm module.
*/
void PYXLibInstance::uninitDerm()
{
	TRACE_INFO("Uninitializing derm module.");
	DermUtils::uninitialize();
	MemoryManager::freeStaticData();
	WGS84::freeStaticData();
	PYXMath::freeStaticData();
	PYXIcosMath::freeStaticData();
	Icosahedron::freeStaticData();
	ReferenceSphere::freeStaticData();
	SnyderProjection::freeStaticData();
	PYXSpiralIterator::freeStaticData();
}

/*!
Initialize the geometry module.
*/
void PYXLibInstance::initGeometry()
{
	TRACE_INFO("Initializing geometry module.");
	PYXVectorGeometry::initStaticData();
	PYXPrimeInnerTileIterator::initStaticData();
	PYXPolygon::initStaticData();
}

/*!
Destroy the geometry module.
*/
void PYXLibInstance::uninitGeometry()
{
	TRACE_INFO("Uninitializing geometry module.");
	PYXVectorGeometry::freeStaticData();
	PYXPrimeInnerTileIterator::freeStaticData();
	PYXPolygon::freeStaticData();
}

/*!
Set up utility initialization.

\param strAppName			The application name, used to name files and directories.
\param bClearCache			If true, any existing cache is cleared
\param strWorkingDir		The absolute path to the directory to store all 
							dynamically generated files. This includes temp files,
							trace files, and properties files. Leaving the path blank
							will co-locate the working directory to the 
							application directory.
\param strApplicationDir	The directory where the WorldView exe is located.
*/
void PYXLibInstance::initUtility(	const std::string& strAppName,
									bool bClearCache,
									const std::string& strWorkingDirectory, 
									const std::string& strAppDirectory,
									const std::string& strCacheDir )
{
	AppServices::initialize(	strAppName,
								bClearCache,
								strWorkingDirectory, 
								strAppDirectory,
								strCacheDir	); // DO NOT TRACE BEFORE HERE!
	TRACE_INFO("Application services initialized");
	TRACE_INFO("Initializing utility module");
	TestFrame::initStaticData();
	XMLUtils::initialize();
	SSLUtils::initialize();
	PYXCOMInitialize();
}

/*!
Delete the trace instance, temp files and other utilities.
*/
void PYXLibInstance::uninitUtility()
{
	TRACE_INFO("Uninitializing utility module.");
	PYXCOMUninitialize();
	XMLUtils::uninitialize();
	SSLUtils::uninitialize();
	TestFrame::freeStaticData();
	PYXThreadPool::shutdown();
	AppServices::uninitialize(); // DO NOT TRACE AFTER HERE!
}

#if _WINDOWS
/*!
Callback function to output.  Sends trace output to the diagnostic window.

\param	nLevel		The trace level.
\param	strMessage	The trace message.
*/
static void onTrace(Trace::eLevel nLevel, const std::string &strMessage, void* pUserData)
{
	OutputDebugStringA(strMessage.c_str());
	OutputDebugString(_T("\r\n"));
}
#endif

