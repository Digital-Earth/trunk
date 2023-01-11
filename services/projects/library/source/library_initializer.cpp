/******************************************************************************
library_initializer.cpp

begin      : 08/03/2007 4:56:31 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define LIBRARY_SOURCE
#include "library_initializer.h"

// local includes
#include "library.h"
#include "library_feature_collection_process.h"

// pyxlib includes
#include "pyxlib_instance.h"

//PYXCOM_BEGIN_CLASS_OBJECT_TABLE
//	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(LibraryFeatureCollectionProcess)
//PYXCOM_END_CLASS_OBJECT_TABLE_EXPORT_AS(library__GCO)

/*!
The reference count.
*/
int LibraryInitializer::m_nCount = 0;

/*!
Initialize the library for use, if not already initialized.
*/
LibraryInitializer::LibraryInitializer()
{
	initialize();
}

/*!
Clean up the library after use, if not already cleaned up.
*/
LibraryInitializer::~LibraryInitializer()
{
	uninitialize();
}

/*!
Initialize the library for use, if not already initialized.
*/
void LibraryInitializer::initialize()
{
	if (0 == m_nCount++)
	{
		TRACE_INFO("Initializing Library Service");

		// Initialize classes
		Library::initialize();
//		PYXCOMRegister(library__GCO);





		TRACE_INFO("Library Service initialization complete");
	}
	else
	{
		TRACE_DEBUG(	"LibraryInitializer already initialized, reference count incremented to '" <<
						m_nCount << "'");
	}
}

/*!
Clean up the library after use, if not already cleaned up.
*/
void LibraryInitializer::uninitialize()
{
	if (0 == --m_nCount)
	{
		TRACE_INFO("Library service cleanup beginning");

		// clean up classes in reverse order from initialization
		Library::uninitialize();

		TRACE_INFO("Library service cleanup complete");
	}
	else
	{
		TRACE_DEBUG(	"LibraryInitializer reference count decremented to '" <<
						m_nCount << "'"	);	
	}
}
