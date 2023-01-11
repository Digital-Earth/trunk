/******************************************************************************
bitmap_crop_process.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "bitmap_crop_process.h"
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

////////////////////////////////////////////////////////////////////////////////
//
//
// BitmapCropProcess
//
//
////////////////////////////////////////////////////////////////////////////////

// {AABE9E2A-9A50-42f8-A9F8-7DFBE2C5C3A9}
PYXCOM_DEFINE_CLSID(BitmapCropProcess, 
0xaabe9e2a, 0x9a50, 0x42f8, 0xa9, 0xf8, 0x7d, 0xfb, 0xe2, 0xc5, 0xc3, 0xa9);

PYXCOM_CLASS_INTERFACES(BitmapCropProcess, IProcess::iid, IBitmap::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BitmapCropProcess, "Bitmap Crop", "Crop an input bitmap", "Hidden",
					IBitmap::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IBitmap::iid, 1, 1, "input Bitmap", "The input Bitmap.");
IPROCESS_SPEC_END

//! The unit test class
Tester<BitmapCropProcess> gTester;


/*!
The unit test method for the class.
*/
void BitmapCropProcess::test()
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


	// Create a test process with a path
	boost::intrusive_ptr<IProcess> spCropProc(new BitmapCropProcess);
	TEST_ASSERT(spCropProc);

	// create an bitmap process
	BitmapCropProcess *pCropBitmap = dynamic_cast<BitmapCropProcess*>(spCropProc.get());
	TEST_ASSERT(pCropBitmap != 0);

	spCropProc->getParameter(0)->addValue(spProc);

	mapAttr.clear();
	mapAttr["X"] = 10;
	mapAttr["Y"] = 10;
	mapAttr["Width"] = 32;
	mapAttr["Height"] = 32;

	spCropProc->setAttributes(mapAttr);
		
	TEST_ASSERT(pCropBitmap->initProc() == knInitialized);

	TEST_ASSERT(pCropBitmap->getWidth() == 64 && "Failed to load image");

	TEST_ASSERT(pCropBitmap->getHeight() == 64 && "Failed to load image");

	TEST_ASSERT(pCropBitmap->getBitmapCount() == 1);

	PYXPointer<PYXBitmap> theRawBitmap = pCropBitmap->getRawBitmap();

	TEST_ASSERT(theRawBitmap);

	if (theRawBitmap)
	{
		TEST_ASSERT(theRawBitmap->getWidth() == pCropBitmap->getWidth());

		TEST_ASSERT(theRawBitmap->getHeight() == pCropBitmap->getHeight());
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE BitmapCropProcess::getAttributes() const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::map<std::string, std::string> mapAttr;
		
	mapAttr["X"] = StringUtils::toString(m_x);
	mapAttr["Y"] = StringUtils::toString(m_y);
	mapAttr["Width"] = StringUtils::toString(m_width);
	mapAttr["Height"] = StringUtils::toString(m_height);
	
	return mapAttr;
}

void STDMETHODCALLTYPE BitmapCropProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{	
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"X",int,m_x);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Y",int,m_y);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Width",int,m_width);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Height",int,m_height);	
}

std::string STDMETHODCALLTYPE BitmapCropProcess::getAttributeSchema() const
{
      return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"BitmapCropProcess\">"
		  "<xs:complexType>"
		    "<xs:sequence>"
			
		      "<xs:element name=\"X\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>X coordinate</friendlyName>"
					"<description>X coordiante of the crop region</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
	  
		      "<xs:element name=\"Y\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Y coordinate</friendlyName>"
					"<description>Y coordiante of the crop region</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"Width\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Width</friendlyName>"
					"<description>The width of the crop region</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"Height\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Height</friendlyName>"
					"<description>The height of the crop region</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus BitmapCropProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_bitmap = NULL;

	if (!(getParameter(0)->getValue(0)))
	{		
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(
			"BitmapCropProcess: Missing input.");

		return knFailedToInit;

	}
	
	// Get the path argument.
	boost::intrusive_ptr<IBitmap> spBitmap;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IBitmap::iid, (void**) &spBitmap);
	if (!spBitmap)
	{
		
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(			
			"BitmapCropProcess: Bitmap parameter was not properly initialized.");

		return knFailedToInit;
	}	

	m_spInputBitmap = spBitmap;
	
	return knInitialized;
}

void BitmapCropProcess::loadImage(int index)
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (m_bitmap && m_loadedImageIndex == index)
	{
		//image already loaded
		return;
	}

	m_bitmap = NULL;	

	if (index >= m_spInputBitmap->getBitmapCount())
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	m_bitmap = PYXBitmap::createFromDefinition(getBitmapDefinition(index));
	
	if (!m_bitmap)
	{
		PYXTHROW(PYXException, "failed to load bitmap");				
	}

	m_loadedImageIndex = index;
}


////////////////////////////////////////////////////////////////////////////////
// IBitmap
////////////////////////////////////////////////////////////////////////////////

//! Returns bitmap width in pixels
int STDMETHODCALLTYPE BitmapCropProcess::getWidth(int index) 
{
	if (index >= m_spInputBitmap->getBitmapCount())
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	return m_width;
}

//! Returns bitmap height in pixels
int STDMETHODCALLTYPE BitmapCropProcess::getHeight(int index) 
{
	if (index >= m_spInputBitmap->getBitmapCount())
	{
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	return m_height;
}

//! Returns a string with the bitmap RGB or RGBA data
const PYXPointer<PYXBitmap> STDMETHODCALLTYPE BitmapCropProcess::getRawBitmap(int index )
{
	loadImage(index);	

	return m_bitmap;
}

//! Returns the number bitmaps provided by process
int BitmapCropProcess::getBitmapCount() 
{
	return m_spInputBitmap->getBitmapCount();
}

std::string BitmapCropProcess::getBitmapDefinition(int index) 
{
	std::string bitmapDefinition = "<crop X=\""+StringUtils::toString(m_x)+"\" Y=\""+StringUtils::toString(m_y)+"\" Width=\""+StringUtils::toString(m_width)+ "\" Height=\"" +StringUtils::toString(m_height)+"\">";
		
	boost::intrusive_ptr<IBitmap> spBitmap;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IBitmap::iid, (void**) &spBitmap);

	bitmapDefinition	+= spBitmap->getBitmapDefinition(index);

	bitmapDefinition	+= "</crop>";

	return bitmapDefinition;
}


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

BitmapCropProcess::BitmapCropProcess() :
	m_x(0),
	m_y(0),
	m_width(0),
	m_height(0)
{
}