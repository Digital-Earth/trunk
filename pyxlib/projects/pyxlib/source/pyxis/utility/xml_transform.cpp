/******************************************************************************
xml_transform.cpp

begin		: 2008-04-15
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/xml_transform.h"

#include "pyxis/utility/trace.h"
#include "pyxis/utility/tester.h"

PYXPointer<CSharpFunctionProvider> CSharpFunctionProvider::m_spProvider;

PYXPointer<CSharpFunctionProvider> CSharpFunctionProvider::getCSharpFunctionProvider()
{
	return m_spProvider;
}

void CSharpFunctionProvider::setCSharpFunctionProvider( PYXPointer<CSharpFunctionProvider> spProvider)
{
	m_spProvider = spProvider;
}

std::string CSharpFunctionProvider::applyXsltTransform(std::string transform, std::string inputXml)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

bool CSharpFunctionProvider::doesXPathMatch(std::string xPathExpression, std::string inputXml)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return false;
}

bool CSharpFunctionProvider::isWellFormedURI(const std::string checkUri)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return false;
}

std::string CSharpFunctionProvider::setDefaultValueForUrlQueryParameter(std::string baseUri, std::string  key, std::string value)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::overwriteUrlQueryParameter(std::string baseUri, std::string  key, std::string value)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::removeUrlQueryParameter(std::string baseUri, std::string  key)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::getUrlQueryParameter(std::string baseUri, std::string key)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::getUrlHost(std::string baseUri)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::XMLSerialize(std::vector<std::string> serializeMe)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

bool CSharpFunctionProvider::XMLDeserialize(std::vector<std::string>& target, std::string source)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return false;
}
std::string CSharpFunctionProvider::getSerializedManifest(boost::intrusive_ptr<IPath> spPathProc)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::getSerializedManifestForFile(std::string filename)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}

std::string CSharpFunctionProvider::getIdentity(std::string strManifest)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpFunctionProvider()");
	return "";
}





PYXPointer<CSharpXMLDocProvider> CSharpXMLDocProvider::m_spProvider;

PYXPointer<CSharpXMLDocProvider> CSharpXMLDocProvider::getCSharpXMLDocProvider()
{
	return m_spProvider;
}

void CSharpXMLDocProvider::setCSharpXMLDocProvider( PYXPointer<CSharpXMLDocProvider> spProvider)
{
	m_spProvider = spProvider;
}

int CSharpXMLDocProvider::createDocument(const std::string & xmlString,bool removeNamespaces)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return 0;
}

void CSharpXMLDocProvider::destroyDocument(const int & docHandle)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");	
}


void CSharpXMLDocProvider::saveToFile( const int & docHandle, const std::string & path )
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");	
}

int CSharpXMLDocProvider::getNodesCount(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");	
}

bool CSharpXMLDocProvider::hasNode(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return false;
}

void CSharpXMLDocProvider::setNodeText(const int & docHandle, const std::string & xmlPath, const std::string & text)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

std::string CSharpXMLDocProvider::getNodeText(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return "";
}

std::string CSharpXMLDocProvider::getInnerXMLString(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return "";
}

void CSharpXMLDocProvider::setInnerXMLString(const int & docHandle, const std::string & xmlPath, const std::string & innerXml)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");	
}

std::string CSharpXMLDocProvider::getOuterXMLString(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return "";
}

void CSharpXMLDocProvider::removeNode(const int & docHandle, const std::string & xmlPath)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

void CSharpXMLDocProvider::addChild(const int & docHandle, const std::string & xmlPath, const std::string & xmlNode)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

void CSharpXMLDocProvider::addChildWithInnerText(const int & docHandle, const std::string & xmlPath, const std::string & xmlNode, const std::string & innerText)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

bool CSharpXMLDocProvider::hasAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return false;
}

std::string CSharpXMLDocProvider::getAttributeValue(const int & docHandle, const std::string & xmlPath, const std::string & attributeName)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
	return "";
}

void CSharpXMLDocProvider::setAttributeValue(const int & docHandle, const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

void CSharpXMLDocProvider::addAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

void CSharpXMLDocProvider::removeAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName)
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

void CSharpXMLDocProvider::addNamespace( const int & docHandle, const std::string & prefix, const std::string & uri )
{
	PYXTHROW(PYXException, "This function should never be called before initializing with setCSharpXMLDocProvider()");
}

//! Tester Class
Tester<CSharpXMLDoc> gTester;

//! Test Method 
void CSharpXMLDoc::test()
{
	PYXPointer<CSharpXMLDoc> doc =	CSharpXMLDoc::create("");

	//test clean start
	TEST_ASSERT(doc->getXMLString() == "");

	//test checking nodes and adding nodes
	TEST_ASSERT(doc->hasNode("/node") == false);
	doc->addChild("/","node");
	TEST_ASSERT(doc->hasNode("/node") == true);

	//testin sub nodes adding and checking
	TEST_ASSERT(doc->hasNode("/node/node2") == false);
	doc->addChild("/node","node2");	
	TEST_ASSERT(doc->hasNode("/node/node2") == true);	
	TEST_ASSERT(doc->getXMLString() == "<node><node2 /></node>");
	

	//do several copy doc and make sure we ended up ok
	for (int i=0;i<10;i++)
	{
		int oldHandle = doc->m_handle;
		//create a new doc that should be the same as the old one,.
		doc = CSharpXMLDoc::create(doc->getXMLString());

		TEST_ASSERT(doc->m_handle != oldHandle);
	}
	TEST_ASSERT(doc->hasNode("/node/node2") == true);
	TEST_ASSERT(doc->getXMLString() == "<node><node2 /></node>");
	
	//testing set Node Text
	doc->setNodeText("/node/node2","PYXIS");
	TEST_ASSERT(doc->getNodeText("/node/node2") == "PYXIS");
	TEST_ASSERT(doc->getXMLString() == "<node><node2>PYXIS</node2></node>");

	//testing get Outter and Inner text
	TEST_ASSERT(doc->getInnerXMLString("/node") == "<node2>PYXIS</node2>");
	TEST_ASSERT(doc->getOuterXMLString("/node/node2") == "<node2>PYXIS</node2>");

	//testing remove nodes
	doc->removeNode("/node/node2");
	TEST_ASSERT(doc->hasNode("/node/node2") == false);
	TEST_ASSERT(doc->hasNode("/node") == true);

	//testing adding attributes
	doc->addChild("/node","node2");	

	TEST_ASSERT(doc->hasAttribute("/node/node2","name") == false);
	doc->addAttribute("/node/node2","name","A Name");
	TEST_ASSERT(doc->hasAttribute("/node/node2","name") == true);

	//testing get and set values
	TEST_ASSERT(doc->getAttributeValue("/node/node2","name") == "A Name");
	doc->setAttributeValue("/node/node2","name","new Name");
	TEST_ASSERT(doc->getAttributeValue("/node/node2","name") == "new Name");	
	doc->setAttributeValue("/node/node2","name","<name with specail #!@/> tags'and stuff\" like that");
	TEST_ASSERT(doc->getAttributeValue("/node/node2","name") == "<name with specail #!@/> tags'and stuff\" like that");

	//testing remove attributes
	doc->removeAttribute("/node/node2","name");
	TEST_ASSERT(doc->hasAttribute("/node/node2","name") == false);

	//testing second time addtions
	doc->addAttribute("/node/node2","name","A Name");
	doc->addAttribute("/node/node2","name","other Name");
	TEST_ASSERT(doc->hasAttribute("/node/node2","name") == true);
	//make sure we have the first attribute value
	TEST_ASSERT(doc->getAttributeValue("/node/node2","name") == "A Name");

	//make sure we have only one node2 node
	TEST_ASSERT(doc->getNodesCount("/node/node2") == 1);

	//add another node
	doc->addChild("/node","node2");	
	
	//make sure we still have the attribute
	TEST_ASSERT(doc->hasAttribute("/node/node2","name") == true);

	//make sure we have two nodes2
	TEST_ASSERT(doc->getNodesCount("/node/node2") == 2);

	//make sure we can access each node
	doc->setNodeText("/node/node2[2]","PYXIS");	
	TEST_ASSERT(doc->getOuterXMLString("/node/node2[1]") == "<node2 name=\"A Name\" />");
	TEST_ASSERT(doc->getOuterXMLString("/node/node2[2]") == "<node2>PYXIS</node2>");

	TRACE_TEST(doc->getXMLString());
}




CSharpXMLDoc::CSharpXMLDoc(const std::string & xmlString,bool removeNamespaces)
{
	m_handle = CSharpXMLDocProvider::getCSharpXMLDocProvider()->createDocument(xmlString,removeNamespaces);
}

CSharpXMLDoc::~CSharpXMLDoc()
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->destroyDocument(m_handle);
}

std::string CSharpXMLDoc::getXMLString()
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getOuterXMLString(m_handle,"/");
}


void CSharpXMLDoc::saveToFile( const std::string & path )
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->saveToFile(m_handle,path);
}


int CSharpXMLDoc::getNodesCount(const std::string & xmlPath)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getNodesCount(m_handle,xmlPath);
}


bool CSharpXMLDoc::hasNode(const std::string & xmlPath)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->hasNode(m_handle,xmlPath);
}

void CSharpXMLDoc::setNodeText(const std::string & xmlPath, const std::string & text)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->setNodeText(m_handle,xmlPath,text);
}

std::string CSharpXMLDoc::getNodeText(const std::string & xmlPath)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getNodeText(m_handle,xmlPath);
}

std::string CSharpXMLDoc::getInnerXMLString(const std::string & xmlPath)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getInnerXMLString(m_handle,xmlPath);
}

void CSharpXMLDoc::setInnerXMLString(const std::string & xmlPath,const std::string & innerXml)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->setInnerXMLString(m_handle,xmlPath,innerXml);
}

std::string CSharpXMLDoc::getOuterXMLString(const std::string & xmlPath)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getOuterXMLString(m_handle,xmlPath);
}

void CSharpXMLDoc::removeNode(const std::string & xmlPath)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->removeNode(m_handle,xmlPath);
}

void CSharpXMLDoc::addChild(const std::string & xmlPath, const std::string & xmlNode)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->addChild(m_handle,xmlPath,xmlNode);
}

void CSharpXMLDoc::addChildWithInnerText( const std::string & xmlPath, const std::string & xmlNode, const std::string & innerText )
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->addChildWithInnerText(m_handle,xmlPath,xmlNode,innerText);
}

bool CSharpXMLDoc::hasAttribute(const std::string & xmlPath, const std::string & attributeName)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->hasAttribute(m_handle,xmlPath,attributeName);
}

std::string CSharpXMLDoc::getAttributeValue(const std::string & xmlPath, const std::string & attributeName)
{
	return CSharpXMLDocProvider::getCSharpXMLDocProvider()->getAttributeValue(m_handle,xmlPath,attributeName);
}

void CSharpXMLDoc::setAttributeValue(const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->setAttributeValue(m_handle,xmlPath,attributeName,attributeValue);
}

void CSharpXMLDoc::addAttribute(const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->addAttribute(m_handle,xmlPath,attributeName,attributeValue);
}

void CSharpXMLDoc::removeAttribute(const std::string & xmlPath, const std::string & attributeName)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->removeAttribute(m_handle,xmlPath,attributeName);
}

void CSharpXMLDoc::addNamespace( const std::string & prefix,const std::string & uri)
{
	CSharpXMLDocProvider::getCSharpXMLDocProvider()->addNamespace(m_handle,prefix,uri);
}
