/******************************************************************************
bitmap_process.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "bitmap_process.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

#include "pyxis/utility/bitmap_server_provider.h"

// standard includes
#include <cassert>


// {5A0BC809-48AF-4152-B22A-E12ED3A059E7}
PYXCOM_DEFINE_CLSID(BitmapProcess, 
0x5a0bc809, 0x48af, 0x4152, 0xb2, 0x2a, 0xe1, 0x2e, 0xd3, 0xa0, 0x59, 0xe7);

PYXCOM_CLASS_INTERFACES(BitmapProcess, IProcess::iid, IBitmap::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BitmapProcess, "Bitmap Reader", "Loads a Bitmap from a file or url.", "Hidden",
					IBitmap::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IUrl::iid, 1, 1, "Bitmap Path", "The input Bitmap file or url.");
IPROCESS_SPEC_END

//! The unit test class
Tester<BitmapProcess> gTester;


/*!
The unit test method for the class.
*/
void BitmapProcess::test()
{
	// Create a test process with a path
	boost::intrusive_ptr<IProcess> spProc(new BitmapProcess);
	TEST_ASSERT(spProc);

	// create an bitmap process
	BitmapProcess *pBitmap = dynamic_cast<BitmapProcess*>(spProc.get());
	TEST_ASSERT(pBitmap != 0);

	
	// add a path to it
	boost::intrusive_ptr<IProcess> spPathProc;
	PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathProc);
	std::map<std::string, std::string> mapAttr;
	
	
	boost::filesystem::path path = 
		gTester.getTestDataPath() / 
		FileUtils::stringToPath("pyxis_processes/bitmap.png");
	
	if (!FileUtils::exists(path))
	{
		TRACE_TEST("Test data not configured, skipping remaining Bitmap Process tests.");
		return;
	}
	mapAttr["uri"] = FileUtils::pathToString(path);
	
	spPathProc->setAttributes(mapAttr);
	spPathProc->initProc();
	
	spProc->getParameter(0)->addValue(spPathProc);
		
	TEST_ASSERT(spProc->initProc() == knInitialized);

	TEST_ASSERT(pBitmap->getWidth() == 64 && "Failed to load image");

	TEST_ASSERT(pBitmap->getHeight() == 64 && "Failed to load image");

	TEST_ASSERT(pBitmap->getBitmapCount() == 1);

	PYXPointer<PYXBitmap> theRawBitmap = pBitmap->getRawBitmap();

	TEST_ASSERT(theRawBitmap);

	if (theRawBitmap)
	{
		TEST_ASSERT(theRawBitmap->getWidth() == pBitmap->getWidth());

		TEST_ASSERT(theRawBitmap->getHeight() == pBitmap->getHeight());
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE BitmapProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	
	return mapAttr;
}

void STDMETHODCALLTYPE BitmapProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
}

std::string STDMETHODCALLTYPE BitmapProcess::getAttributeSchema() const
{
      return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"BitmapProcess\">"
		  "<xs:complexType>"			
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus BitmapProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_bitmap = NULL;

	if (!(getParameter(0)->getValue(0)))
	{		
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"BitmapProcess: Missing input.");

		return knFailedToInit;
	}
	
	// Get the path argument.
	boost::intrusive_ptr<IUrl> spUrl;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);
	if (!spUrl)
	{	
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"BitmapProcess: Url parameter was not properly initialized.");

		return knFailedToInit;
	}	
		
	return knInitialized;
}

void BitmapProcess::loadImage(int index)
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (m_bitmap && m_loadedImageIndex == index)
	{
		//image already loaded
		return;
	}

	m_bitmap = NULL;	

	if (index>0)
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	// Get the path argument.
	boost::intrusive_ptr<IUrl> spUrl;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);
	if (!spUrl)
	{	
		PYXTHROW(PYXException, "Url parameter was not properly initialized.");
	}

	// Check if is a IPath... 
	boost::intrusive_ptr<IPath> spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IPath::iid, (void**) &spPath);

	// if it does IPath, use it
	if (spPath)
	{	
		boost::filesystem::path path = FileUtils::stringToPath(spPath->getLocallyResolvedPath());	
		TRACE_INFO("Attempting to open Bitmap file: " << FileUtils::pathToString(path));

		m_bitmap = PYXBitmap::createFromPath(FileUtils::pathToString(path));	
	} else 
	{
		m_bitmap = PYXBitmap::createFromPath(spUrl->getUrl());
	}	

	if (!m_bitmap)
	{	
		PYXTHROW(PYXException, "failed to load bitmap from path.");				
	}

	m_loadedImageIndex = index;
}


////////////////////////////////////////////////////////////////////////////////
// IBitmap
////////////////////////////////////////////////////////////////////////////////

//! Returns bitmap width in pixels
int STDMETHODCALLTYPE BitmapProcess::getWidth(int index)
{
	loadImage(index);
	
	return m_bitmap->getWidth();
}

//! Returns bitmap height in pixels
int STDMETHODCALLTYPE BitmapProcess::getHeight(int index)
{
	loadImage(index);

	return m_bitmap->getHeight();
}

//! Returns a string with the bitmap RGB or RGBA data
const PYXPointer<PYXBitmap> STDMETHODCALLTYPE BitmapProcess::getRawBitmap(int index)
{
	loadImage(index);

	return m_bitmap;
}

//! Returns the number bitmaps provided by process
int BitmapProcess::getBitmapCount() 
{
	return 1;
}

std::string BitmapProcess::getBitmapDefinition(int index ) 
{
	std::string bitmapDefinition = "<src>";
	
	// Get the path argument.
	boost::intrusive_ptr<IUrl> spUrl;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IUrl::iid, (void**) &spUrl);
	if (!spUrl)
	{	
		PYXTHROW(PYXException, "Url parameter was not properly initialized.");
	}
	// Check if is a IPath... 
	boost::intrusive_ptr<IPath> spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IPath::iid, (void**) &spPath);

	// if it does IPath, use it
	if (spPath)
	{	
		bitmapDefinition	+= XMLUtils::toSafeXMLText(spPath->getLocallyResolvedPath());
	} else 
	{
		bitmapDefinition	+= XMLUtils::toSafeXMLText(spUrl->getUrl());
	}
	
	bitmapDefinition	+= "</src>";

	return bitmapDefinition;
}


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

BitmapProcess::BitmapProcess() 	
{
}