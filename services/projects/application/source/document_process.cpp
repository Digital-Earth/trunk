/******************************************************************************
document_process.cpp

begin      : 3/1/2007 12:00:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
// local inclues
#include "document_process.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// {D4550BA5-B5B8-4950-A198-CEA258FB6BC2}
PYXCOM_DEFINE_CLSID(DocumentProcess, 
0xd4550ba5, 0xb5b8, 0x4950, 0xa1, 0x98, 0xce, 0xa2, 0x58, 0xfb, 0x6b, 0xc2);
PYXCOM_CLASS_INTERFACES(DocumentProcess, IProcess::iid, IDocument::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(DocumentProcess, "Document Process", "Collection of settings and objects that provide a unique view of the earth.", "Hidden",
					IDocument::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, 1, "Visualization Process", "The entire visual pipeline associated with the document.")
IPROCESS_SPEC_END

const std::string DocumentProcess::kstrDefaultName = "PYXIS Document";
const std::string DocumentProcess::kstrDefaultDescription = "A document for a PYXIS application.";
const std::string DocumentProcess::kstrDocExt = ".ppl";
const std::string DocumentProcess::kstrCameraAttrib = "camera";

//! Tester class
Tester<DocumentProcess> gTester;

//! Test method
void DocumentProcess::test()
{
	boost::intrusive_ptr<IDocument> spDoc;
	PYXCOMCreateInstance(strToGuid("{D4550BA5-B5B8-4950-A198-CEA258FB6BC2}"), 0, IDocument::iid, (void**) &spDoc);
	
	boost::intrusive_ptr<IProcess> spViewPointProcess;
	PYXCOMCreateInstance(strToGuid("{85ECCFCB-1D9B-4df9-807F-391D03FCB1FB}"), 0, IProcess::iid, (void**) &spViewPointProcess);
	spDoc->setViewPointProcess(spViewPointProcess);

	//Test serialization.
	{
		std::string tempFile = FileUtils::pathToString(AppServices::makeTempFile(".ppl"));	
		spDoc->save(tempFile);
		TEST_ASSERT(spDoc->isOutOfDate() == false);
		boost::intrusive_ptr<IDocument> spNewDoc;
		{
			boost::intrusive_ptr<IProcess> spProc = PipeManager::readPipelineFromFile(tempFile);
			spProc->QueryInterface(IDocument::iid, (void**) &spNewDoc);
		}
		TEST_ASSERT(spNewDoc);

		TEST_ASSERT(ProcRef(spNewDoc->getViewPointProcess()) == ProcRef(spDoc->getViewPointProcess()));
	}
	
	//Test setters and getters
	{
		spDoc->setViewPointProcess(spViewPointProcess);
		TEST_ASSERT(spDoc->isOutOfDate() == true);
		TEST_ASSERT(spDoc->getViewPointProcess() == spViewPointProcess);

		{ //Test that VisProcess serialized in 1 Doc is the same one deserialized in new document.

			std::string tempFile = FileUtils::pathToString(AppServices::makeTempFile(".ppl"));
			spDoc->save(tempFile);
			TEST_ASSERT(spDoc->isOutOfDate() == false);
			boost::intrusive_ptr<IDocument> spNewDoc;
			{
				boost::intrusive_ptr<IProcess> spProc = PipeManager::readPipelineFromFile(tempFile);
				spProc->QueryInterface(IDocument::iid, (void**) &spNewDoc);
			}
			TEST_ASSERT(spNewDoc);

			TEST_ASSERT(ProcRef(spNewDoc->getViewPointProcess()) == ProcRef(spDoc->getViewPointProcess()));
			TEST_ASSERT(spNewDoc->isOutOfDate() == false);
		}
	}
}

//! Default constructor.
DocumentProcess::DocumentProcess(void): 
	ProcessImpl<DocumentProcess>(kstrDefaultName, kstrDefaultDescription),
	m_bDirtyFlag(false)
{
}

//! Destructor.
DocumentProcess::~DocumentProcess(void)
{
}

//! Initializer sets the document status to unmodified.
IProcess::eInitStatus DocumentProcess::initImpl()
{
	if (m_strCamCookie.empty())
	{
		// Sensible default position
		TRACE_INFO("Setting camera position to default.");
		m_strCamCookie = "45 -75 0 0 0 12742014";
	}
	m_bDirtyFlag = false;
	return knInitialized;
}

/*!
Saves a document via the PipeManager::writeProcess, to the given path. Once the 
document has been saved the dirty flag, idicating that their were unsaved changes
is reset.

\param path The file path to serialize the document to.
*/
void DocumentProcess::save(const std::string& path)
{
	assert(!path.empty() && "Cannot save to an empty path.");

	PipeManager::writePipelineToFile(path, boost::intrusive_ptr<DocumentProcess>(this));
	m_bDirtyFlag = false;

	// TODO: Send a notification
}

/*!
Serializes a document to a map of strings. The map is returned to PipeManger for 
serialization out to disk. The map, maps the name of the attribute in the document
to be saved, to the attribute's data.

\returns A map, mapping the name of the attribute to it's data.
*/
std::map<std::string, std::string> STDMETHODCALLTYPE DocumentProcess::getAttributes() const
{
	std::map<std::string, std::string> attributes;

	// write out the attributes
	attributes[kstrCameraAttrib] = m_strCamCookie;

	return attributes;
}

std::string STDMETHODCALLTYPE DocumentProcess::getAttributeSchema() const
{
	return "";
}

/*!
Return the ViewPointProcess for the document.

\return	The ViewPointProcess.
*/
boost::intrusive_ptr<IProcess> DocumentProcess::getViewPointProcess()
{
	// acquire the ViewPointProcess
	PYXPointer<Parameter> param = getParameter(0);

	// currently the ViewPointProcess is the only parameter, upgrade safety with expansion.
	assert(param->getSpec()->getName() == "Visualization Process");
	boost::intrusive_ptr<IProcess> returnProc;
	if (param->getValueCount() == 1)
	{
		returnProc = param->getValue(0);	
	}
	return returnProc;
}

/*!
Sets the visualization pipeline used for the document.

\param spViewPointProcess	The new ChannelCombiner process that is the root of a 
							visualization pipeline.
*/
void DocumentProcess::setViewPointProcess(boost::intrusive_ptr<IProcess> spViewPointProcess)
{
	// verify the passed parameter
	assert(spViewPointProcess && "Attempting to set a null ViewPoint process!");
	if (spViewPointProcess->getSpec()->getClass() != 
		strToGuid("{85ECCFCB-1D9B-4df9-807F-391D03FCB1FB}"))
	{
		PYXTHROW(
			DocumentException, 
			"Document only accepts a ViewPoint process as its input!");
	}

	// acquire visualization process input
	PYXPointer<Parameter> param = getParameter(0);
	assert(param->getSpec()->getName() == "Visualization Process");
	if (param->getValueCount() == 0)
	{
		param->addValue(spViewPointProcess);
	}
	else
	{
		param->setValue(0, spViewPointProcess);
	}

	m_bDirtyFlag = true;

// TODO: Send a notification
}

/*!
Deserialize a document, by searching the map of attributes for known attributes. Once
an attribute is found it the data from the map is loaded into the data members for the
document. If a data is missing from the map the document during deserialization attempts
to compensate by creating defaults for the document data members.

\param mapAttr	A map mapping known attributes of the document to the attribute's data.
*/
void DocumentProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string,std::string>::const_iterator it = mapAttr.find(kstrCameraAttrib);
	
	if (it != mapAttr.end())
	{
		m_strCamCookie = it->second;
	}

	
// TODO: Send a notification
}
