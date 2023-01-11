#ifndef PYXIS__UTILITY__XML_TRANSFORM_H
#define PYXIS__UTILITY__XML_TRANSFORM_H
/******************************************************************************
xml_transform.h

begin		: 2008-04-15
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/procs/path.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <string>
#include <vector>

class PYXLIB_DECL CSharpFunctionProvider : public PYXObject
{
public:

	//! Applies the given transform on the inputXml, returns transformed content.
	virtual std::string applyXsltTransform(std::string transform, std::string inputXml);

	//! Tests to see if the given XPath expression matches the input.
	virtual bool doesXPathMatch(std::string xPathExpression, std::string inputXml);

	//! Test to see if a URI is a well formed URI.
	virtual bool isWellFormedURI(std::string checkUri);

	//update an url by adding a query parameter with a value if value is not set already
	virtual std::string setDefaultValueForUrlQueryParameter(std::string baseUri, std::string  key, std::string value);

	//update an url by overwriting a query parameter 
	virtual std::string overwriteUrlQueryParameter(std::string baseUri, std::string key, std::string value);

	//update an url by remove a query parameter
	virtual std::string removeUrlQueryParameter(std::string baseUri, std::string key);

	//get a value of query parameter inside a url
	virtual std::string getUrlQueryParameter(std::string baseUri, std::string key);

	//get the host from a url
	virtual std::string getUrlHost(std::string baseUri);

	//! Serialize a vector of string to an XML string.
	virtual std::string XMLSerialize(std::vector<std::string> serializeMe);

	//! Serialize a vector of string to an XML string.
	virtual bool XMLDeserialize(std::vector<std::string>& target, std::string source);

	virtual std::string getSerializedManifest(boost::intrusive_ptr<IPath> spPathProc);

    virtual std::string getSerializedManifestForFile(std::string filename);

    virtual std::string getIdentity(std::string strManifest);

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~CSharpFunctionProvider()
	{}

private:

	static PYXPointer<CSharpFunctionProvider> m_spProvider;

public:

	static PYXPointer<CSharpFunctionProvider> getCSharpFunctionProvider();
	static void setCSharpFunctionProvider( PYXPointer<CSharpFunctionProvider> spProvider);
};

class PYXLIB_DECL CSharpXMLDocProvider : public PYXObject
{
public:	
	virtual int createDocument(const std::string & xmlString,bool removeNamespaces);

	virtual void destroyDocument(const int & docHandle);

	virtual void saveToFile(const int & docHandle, const std::string & path);

	virtual int getNodesCount(const int & docHandle, const std::string & xmlPath);

	virtual bool hasNode(const int & docHandle, const std::string & xmlPath);

	virtual void setNodeText(const int & docHandle, const std::string & xmlPath, const std::string & text);

	virtual std::string getNodeText(const int & docHandle, const std::string & xmlPath);

	virtual std::string getInnerXMLString(const int & docHandle, const std::string & xmlPath);

	virtual void setInnerXMLString(const int & docHandle, const std::string & xmlPath, const std::string & innerXml);

	virtual std::string getOuterXMLString(const int & docHandle, const std::string & xmlPath);

	virtual void removeNode(const int & docHandle, const std::string & xmlPath);

	virtual void addChild(const int & docHandle, const std::string & xmlPath, const std::string & xmlNode);

	virtual void addChildWithInnerText(const int & docHandle, const std::string & xmlPath, const std::string & xmlNode, const std::string & innerText);

	virtual bool hasAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName);

	virtual std::string getAttributeValue(const int & docHandle, const std::string & xmlPath, const std::string & attributeName);

	virtual void setAttributeValue(const int & docHandle, const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue);

	virtual void addAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue);

	virtual void removeAttribute(const int & docHandle, const std::string & xmlPath, const std::string & attributeName);

	virtual void addNamespace(const int & docHandle, const std::string & prefix, const std::string & uri);

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~CSharpXMLDocProvider()
	{}

private:

	static PYXPointer<CSharpXMLDocProvider> m_spProvider;

public:

	static PYXPointer<CSharpXMLDocProvider> getCSharpXMLDocProvider();
	static void setCSharpXMLDocProvider( PYXPointer<CSharpXMLDocProvider> spProvider);
};


/*!

A simple wapper around .Net XMLDocument class to access XML nodes and modify the XML Document.

XPath synyax reference: http://www.w3.org/TR/xpath/
XPath examples: http://msdn.microsoft.com/en-us/library/ms256086.aspx

Examples of XPath:
	"/" - root node.
	"/node1/node2" - from the root, goto node1 and then to node2 - (allways return the first node?)
	"/shelf/book[3]" - the third book on the self.
	"/shelf/book[@author = "PYXIS"] - all books with atributes author = "PYXIS".
	countNodes("/node1/node2") - return number of nodes2 under node1.
	countNodes("/node1/*") - return number of all child nodes of node1.

*/
class PYXLIB_DECL CSharpXMLDoc : public PYXObject
{
protected:
	CSharpXMLDoc(const std::string & xmlString,bool removeNamespaces);

public:
	static PYXPointer<CSharpXMLDoc> create(const std::string & xmlString)
	{
		return PYXNEW(CSharpXMLDoc,xmlString,false);
	}

	static PYXPointer<CSharpXMLDoc> createWithoutNamespaces(const std::string & xmlString)
	{
		return PYXNEW(CSharpXMLDoc,xmlString,true);
	}

	virtual ~CSharpXMLDoc();

public:
	std::string getXMLString();

	void saveToFile(const std::string & path);

	int getNodesCount(const std::string & xmlPath);

	bool hasNode(const std::string & xmlPath);

	void setNodeText(const std::string & xmlPath, const std::string & text);

	std::string getNodeText(const std::string & xmlPath);

	std::string getInnerXMLString(const std::string & xmlPath);

	void setInnerXMLString(const std::string & xmlPath,const std::string & innerXml);

	std::string getOuterXMLString(const std::string & xmlPath);

	void removeNode(const std::string & xmlPath);

	void addChild(const std::string & xmlPath, const std::string & xmlNode);

	void addChildWithInnerText(const std::string & xmlPath, const std::string & xmlNode, const std::string & innerText);

	bool hasAttribute(const std::string & xmlPath, const std::string & attributeName);

	std::string getAttributeValue(const std::string & xmlPath, const std::string & attributeName);

	template<typename T>
	T getAttributeValue(const std::string & xmlPath, const std::string & attributeName, const T & defaultValue)
	{
		std::string attrValue(getAttributeValue(xmlPath,attributeName));

		if (attrValue.size() > 0)
		{
			return StringUtils::fromString<T>(attrValue);
		}
		else
		{
			return defaultValue;
		}
	}


	void setAttributeValue(const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue);

	void addAttribute(const std::string & xmlPath, const std::string & attributeName, const std::string & attributeValue);

	void removeAttribute(const std::string & xmlPath, const std::string & attributeName);

	void addNamespace(const std::string & prefix,const std::string & uri);

protected:	
	int m_handle;


public:
	//! UnitTest Method
	static void test();
};



#endif // guard
