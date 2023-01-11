/******************************************************************************
bitmap_grid_process.cpp

begin		: Feb 01, 2010
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE
#include "bitmap_grid_process.h"
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
// BitmapGridProcess
//
//
////////////////////////////////////////////////////////////////////////////////

// {8010AE74-DC26-41cd-907F-93EA4D07AD90}
PYXCOM_DEFINE_CLSID(BitmapGridProcess, 
0x8010ae74, 0xdc26, 0x41cd, 0x90, 0x7f, 0x93, 0xea, 0x4d, 0x7, 0xad, 0x90);

PYXCOM_CLASS_INTERFACES(BitmapGridProcess, IProcess::iid, IBitmap::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BitmapGridProcess, "Divide Bitmap into Grid", "Divides the input bitmap into a grid of sub-bitmaps.", "Hidden",
					IBitmap::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IBitmap::iid, 1, 1, "input Bitmap", "The input Bitmap.");
IPROCESS_SPEC_END

//! The unit test class
Tester<BitmapGridProcess> gTester;


/*!
The unit test method for the class.
*/
void BitmapGridProcess::test()
{
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
	boost::intrusive_ptr<IProcess> spGridProc(new BitmapGridProcess);
	TEST_ASSERT(spGridProc);

	// create an bitmap process
	BitmapGridProcess *pGridBitmap = dynamic_cast<BitmapGridProcess*>(spGridProc.get());
	TEST_ASSERT(pGridBitmap != 0);

	spGridProc->getParameter(0)->addValue(spProc);

	mapAttr.clear();
	mapAttr["rows"] = 4;
	mapAttr["cols"] = 2;

	spGridProc->setAttributes(mapAttr);
		
	TEST_ASSERT(spGridProc->initProc() == knInitialized);

	TEST_ASSERT(pGridBitmap->getWidth() == 64/2 && "Failed to load image");

	TEST_ASSERT(pGridBitmap->getHeight() == 64/4 && "Failed to load image");

	TEST_ASSERT(pGridBitmap->getBitmapCount() == 4*2);

	PYXPointer<PYXBitmap> theRawBitmap = pGridBitmap->getRawBitmap();

	TEST_ASSERT(theRawBitmap);

	if (theRawBitmap)
	{
		TEST_ASSERT(theRawBitmap->getWidth() == pGridBitmap->getWidth());

		TEST_ASSERT(theRawBitmap->getHeight() == pGridBitmap->getHeight());
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE BitmapGridProcess::getAttributes() const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::map<std::string, std::string> mapAttr;
		
	mapAttr["rows"] = StringUtils::toString(m_rows);
	mapAttr["cols"] = StringUtils::toString(m_cols);
	mapAttr["input_index"] = StringUtils::toString(m_inputIndex);
	mapAttr["maximum_index"] = StringUtils::toString(m_maximumIndex);
	
	return mapAttr;
}

void STDMETHODCALLTYPE BitmapGridProcess ::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{	
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"rows",int,m_rows);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"cols",int,m_cols);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"input_index",int,m_inputIndex);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"maximum_index",int,m_maximumIndex);
}

std::string STDMETHODCALLTYPE BitmapGridProcess::getAttributeSchema() const
{
      return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"BitmapGridProcess\">"
		  "<xs:complexType>"
		    "<xs:sequence>"
		      "<xs:element name=\"rows\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Rows</friendlyName>"
					"<description>Number of Rows in the grid.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
		      "<xs:element name=\"cols\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Columns</friendlyName>"
					"<description>Number of Columns in the grid.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"input_index\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Input Bitmap Index</friendlyName>"
					"<description>The input bitmap index to query from the Input Bitmap Process.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"maximum_index\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Maximum Output Index</friendlyName>"
					"<description>The Maximum number of sub-bitmaps to emit.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

IProcess::eInitStatus BitmapGridProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_bitmap = NULL;

	if (!(getParameter(0)->getValue(0)))
	{	
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(			
			"BitmapGridProcess: Missing input.");

		return knFailedToInit;		
	}
		
	// Get the path argument.
	boost::intrusive_ptr<IBitmap> spBitmap;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IBitmap::iid, (void**) &spBitmap);
	if (!spBitmap)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError(			
			"BitmapGridProcess: Bitmap parameter was not properly initialized.");

		return knFailedToInit;		
	}	

	m_spInputBitmap = spBitmap;
	
	return knInitialized;
}

void BitmapGridProcess::loadImage(int index)
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (m_bitmap && m_loadedImageIndex == index)
	{
		//image already loaded
		return;
	}

	m_bitmap = NULL;	

	if (m_inputIndex >= m_spInputBitmap->getBitmapCount())
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
int STDMETHODCALLTYPE BitmapGridProcess::getWidth(int index) 
{
	if (index >= getBitmapCount())
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	return m_spInputBitmap->getWidth(m_inputIndex)/m_cols;
}

//! Returns bitmap height in pixels
int STDMETHODCALLTYPE BitmapGridProcess::getHeight(int index) 
{
	if (index >= getBitmapCount())
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	return m_spInputBitmap->getHeight(m_inputIndex)/m_rows;
}

//! Returns a string with the bitmap RGB or RGBA data
const PYXPointer<PYXBitmap> STDMETHODCALLTYPE BitmapGridProcess::getRawBitmap(int index )
{
	loadImage(index);

	return m_bitmap;
}

//! Returns the number bitmaps provided by process
int BitmapGridProcess::getBitmapCount() 
{
	if (m_maximumIndex > 0)
	{
		return m_maximumIndex;
	}
	else 
	{
		return m_rows*m_cols;
	}
}

std::string BitmapGridProcess::getBitmapDefinition(int index) 
{
	if (index >= getBitmapCount())
	{		
		PYXTHROW(PYXException, "Bitmap Index out of range.");
	}

	std::string bitmapDefinition = "<grid Rows=\""+StringUtils::toString(m_rows)+"\" Cols=\""+StringUtils::toString(m_cols)+"\" Index=\""+StringUtils::toString(index)+ "\" >";
		
	boost::intrusive_ptr<IBitmap> spBitmap;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IBitmap::iid, (void**) &spBitmap);

	bitmapDefinition	+= spBitmap->getBitmapDefinition(m_inputIndex);

	bitmapDefinition	+= "</grid>";

	return bitmapDefinition;
}


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

BitmapGridProcess::BitmapGridProcess() :
	m_cols(0),
	m_rows(0),
	m_inputIndex(0),
	m_maximumIndex(0)
{
}
