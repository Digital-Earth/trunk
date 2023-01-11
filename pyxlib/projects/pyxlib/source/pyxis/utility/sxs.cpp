/******************************************************************************
sxs.cpp

begin		: 2007-06-19
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/sxs.h"

// pyxlib includes
#include "pyxis/utility/xml_utils.h"

// xerces includes
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes
#include <iostream>
#include <sstream>

XERCES_CPP_NAMESPACE_USE

// {C3F9E64E-C7BF-45ee-8588-989794C0B137}
PYXCOM_DEFINE_IID(ISXS, 
0xc3f9e64e, 0xc7bf, 0x45ee, 0x85, 0x88, 0x98, 0x97, 0x94, 0xc0, 0xb1, 0x37);

// {F77DDC37-4207-4ccd-B91E-4CE1AF7ADF71}
PYXCOM_DEFINE_IID(ISXSFactory, 
0xf77ddc37, 0x4207, 0x4ccd, 0xb9, 0x1e, 0x4c, 0xe1, 0xaf, 0x7a, 0xdf, 0x71);

// {C8F36702-F0B0-4b17-9DCC-D04D682E4862}
PYXCOM_DEFINE_CLSID(DefaultSXSObject, 
0xc8f36702, 0xf0b0, 0x4b17, 0x9d, 0xcc, 0xd0, 0x4d, 0x68, 0x2e, 0x48, 0x62);
PYXCOM_CLASS_INTERFACES(DefaultSXSObject, ISXS::iid, PYXCOM_IUnknown::iid);

// TODO DefaultSXSObject isn't currently registered with PYXCOM.
// This is OK but I'm just noting it here to be clear.

namespace
{

//! The unit test class.
Tester<SXSParser> gTester;

//! Name for unit testing.
char barname[] = "bar";

/*!
If needs to be used elsewhere, then move into XMLUtils for reuse.
*/
//! Suitable for use in boost::shared_ptr as a custom deleter.
struct XMLStringReleaser
{
    void operator()(char** buf) const
    {
		XMLString::release(buf);
    }
};

/*!
Create it with a factory map. Use it to handle parsing. The root object is
available after parsing. The handler can be reused.
*/
//! SAX2 handler for SXS.
class SXSHandler : public DefaultHandler
{
public:

	SXSHandler(const SXSParser::FactoryMap& factoryMap) :
		m_factoryMap(factoryMap)
	{
	}

public: // DefaultHandler

	virtual void resetDocument()
	{
		m_vecObject.clear();
		m_strCharacters.clear();
	}

	virtual void startElement(
		const XMLCh* const uri,
		const XMLCh* const localname,
		const XMLCh* const qname,
		const Attributes& attrs		)
	{
		char* name = XMLString::transcode(localname);
		boost::shared_ptr<void> spGuard(&name, XMLStringReleaser());
		ISXS::AttributeMap attrMap = XMLUtils::getAttributes(attrs);

		boost::intrusive_ptr<ISXS> spObject;
		
		// Try to create SXS object using a registered factory.
		{
			SXSParser::FactoryMap::const_iterator it = m_factoryMap.find(name);
			if (it != m_factoryMap.end())
			{
				spObject = it->second->createSXSObject(name, attrMap);
			}
		}

		// But if that fails, fall back to a simple DOM-like object.
		if (!spObject)
		{
			spObject = new DefaultSXSObject();
		}

		spObject->setSXSName(name);
		if (!attrMap.empty())
		{
			spObject->setSXSAttributes(attrMap);
		}

		m_vecObject.push_back(spObject);
	}

	virtual void endElement(
		const XMLCh* const uri,
		const XMLCh* const localname,
		const XMLCh* const qname	)
	{
		boost::intrusive_ptr<ISXS> spObject(m_vecObject.back());
		m_vecObject.pop_back();

		if (!m_strCharacters.empty())
		{
			spObject->setSXSCharacters(m_strCharacters);
			m_strCharacters.clear();
		}

		spObject->initSXS();

		if (!m_vecObject.empty())
		{
			m_vecObject.back()->addSXSChild(spObject);
		}
		else
		{
			// The root object should be left on the stack.
			m_vecObject.push_back(spObject);
		}
	}

	virtual void characters(
        const XMLCh* const chars,
        const XMLSize_t length	)
	{
		// TODO need to transcode?
		m_strCharacters.append(chars, chars + length);
	}

    void fatalError(const SAXParseException& e)
	{
		TRACE_ERROR("Fatal error during SXS parse: " << e.getMessage());
	}

public:

	boost::intrusive_ptr<ISXS> getRootObject() const
	{
		// The root object should be on the stack.
		return m_vecObject.empty() ? 0 : m_vecObject.front();
	}

private:

	//! Factory map.
	const SXSParser::FactoryMap& m_factoryMap;

	//! Stack of objects.
	std::vector<boost::intrusive_ptr<ISXS> > m_vecObject;

	//! Characters.
	std::string m_strCharacters;
};

//! Determines whether two SXS objects are equal using the SXS interface.
bool isEqualSXSObjects(boost::intrusive_ptr<ISXS> spObject1, boost::intrusive_ptr<ISXS> spObject2)
{
	if (spObject1->getSXSName() != spObject2->getSXSName()
		|| spObject1->getSXSAttributes() != spObject2->getSXSAttributes()
		|| spObject1->getSXSCharacters() != spObject2->getSXSCharacters())
	{
		return false;
	}

	if (spObject1->getNumberOfSXSChildren() != spObject2->getNumberOfSXSChildren())
	{
		return false;
	}

	int nChildCount = spObject1->getNumberOfSXSChildren();
	for (int nChild = 0; nChild != nChildCount; ++nChild)
	{
		// Recursion.
		if (!isEqualSXSObjects(spObject1->getSXSChild(nChild), spObject2->getSXSChild(nChild)))
		{
			return false;
		}
	}

	return true;
}

}

//! Unit test.
void SXSParser::test()
{
	SXSParser parser;

	boost::intrusive_ptr<ISXS> spObject = new DefaultSXSObject();

	spObject->setSXSName("foo");

	{
		std::string strExpected = "<foo/>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}

	ISXS::AttributeMap attrMap;
	attrMap["abc"] = "2";
	attrMap["def"] = "3";
	attrMap["ghi"] = "4";
	attrMap["special"] = "&<>\"";
	spObject->setSXSAttributes(attrMap);

	{
		std::string strExpected = "<foo abc=\"2\" def=\"3\" ghi=\"4\" special=\"&amp;&lt;&gt;&quot;\"/>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}

	boost::intrusive_ptr<ISXS> spChild = new DefaultSXSObject();
	spChild->setSXSName("bar");
	attrMap.clear();
	attrMap["jkl"] = "5";
	attrMap["mno"] = "6";
	attrMap["pqrs"] = "7";
	spChild->setSXSAttributes(attrMap);
	spChild->setSXSCharacters("xyzzy");

	spObject->addSXSChild(spChild);

	{
		std::string strExpected = "<foo abc=\"2\" def=\"3\" ghi=\"4\" special=\"&amp;&lt;&gt;&quot;\"><bar jkl=\"5\" mno=\"6\" pqrs=\"7\">xyzzy</bar></foo>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}

	boost::intrusive_ptr<ISXS> spChild2 = new DefaultSXSObject();
	spChild2->setSXSName("bar");
	attrMap.clear();
	attrMap["tuv"] = "8";
	attrMap["wxyz"] = "9";
	spChild2->setSXSAttributes(attrMap);
	spChild2->setSXSCharacters(" s p a c e ");
	spObject->addSXSChild(spChild2);

	{
		std::string strExpected = "<foo abc=\"2\" def=\"3\" ghi=\"4\" special=\"&amp;&lt;&gt;&quot;\"><bar jkl=\"5\" mno=\"6\" pqrs=\"7\">xyzzy</bar><bar tuv=\"8\" wxyz=\"9\"> s p a c e </bar></foo>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}

	spObject->setSXSCharacters("characters");

	{
		std::string strExpected = "<foo abc=\"2\" def=\"3\" ghi=\"4\" special=\"&amp;&lt;&gt;&quot;\"><bar jkl=\"5\" mno=\"6\" pqrs=\"7\">xyzzy</bar><bar tuv=\"8\" wxyz=\"9\"> s p a c e </bar>characters</foo>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));

		// Test factories.
		class CustomSXSObject : public DefaultSXSObject {};
		parser.registerFactory(new DefaultSXSFactory<CustomSXSObject, barname>());
		in.str(strExpected);
		spRoot = 0;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
		TEST_ASSERT(typeid(*spRoot) == typeid(DefaultSXSObject));
		TEST_ASSERT(typeid(*spRoot->getSXSChild(0)) == typeid(CustomSXSObject));
		TEST_ASSERT(typeid(*spRoot->getSXSChild(1)) == typeid(CustomSXSObject));
		parser.unregisterAllFactories();
	}

	spObject->clearSXSAttributes();

	{
		std::string strExpected = "<foo><bar jkl=\"5\" mno=\"6\" pqrs=\"7\">xyzzy</bar><bar tuv=\"8\" wxyz=\"9\"> s p a c e </bar>characters</foo>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}

	spObject->clearSXSChildren();

	{
		std::string strExpected = "<foo>characters</foo>";
		std::ostringstream out;
		parser.writeObject(out, spObject);
		TEST_ASSERT(out.str() == strExpected);
		std::istringstream in(strExpected);
		boost::intrusive_ptr<ISXS> spRoot;
		parser.readObject(in, spRoot);
		TEST_ASSERT(isEqualSXSObjects(spRoot, spObject));
	}
}

void SXSParser::registerFactory(boost::intrusive_ptr<ISXSFactory> spFactory)
{
	assert(spFactory);

	ISXSFactory::NameVector vecName = spFactory->getSXSNames();

	for (ISXSFactory::NameVector::iterator it = vecName.begin();
		it != vecName.end(); ++it)
	{
		assert(!it->empty());
		m_factoryMap[*it] = spFactory;
	}
}

void SXSParser::unregisterFactory(boost::intrusive_ptr<ISXSFactory> spFactory)
{
	assert(spFactory);

	ISXSFactory::NameVector vecName = spFactory->getSXSNames();

	for (ISXSFactory::NameVector::iterator it = vecName.begin();
		it != vecName.end(); ++it)
	{
		assert(!it->empty());
		m_factoryMap.erase(*it);
	}
}

void SXSParser::unregisterAllFactories()
{
	m_factoryMap.clear();
}

void SXSParser::writeObject(std::ostream& out, boost::intrusive_ptr<ISXS> spObject)
{
	assert(spObject);

	// TODO need to do a lot of checking that tokens are OK,
	// protecting against magic characters, transcoding, etc.

	// TODO pretty printing might be a nice enhancement (but what about characters?)

	out << '<' << spObject->getSXSName();

	ISXS::AttributeMap attrMap = spObject->getSXSAttributes();
	for (ISXS::AttributeMap::iterator it = attrMap.begin();
		it != attrMap.end(); ++it)
	{
		out << ' ' << it->first << "=\"" << XMLUtils::toSafeXMLText(it->second, true) << '"';
	}

	if (spObject->hasSXSChildren() || spObject->hasSXSCharacters())
	{
		out << '>';

		int nChildCount = spObject->getNumberOfSXSChildren();
		for (int nChild = 0; nChild != nChildCount; ++nChild)
		{
			// Recursion.
			writeObject(out, spObject->getSXSChild(nChild));
		}

		if (spObject->hasSXSCharacters())
		{
			out << spObject->getSXSCharacters();
		}

		out << "</" << spObject->getSXSName() << '>';
	}
	else
	{
		out << "/>";
	}
}

void SXSParser::writeObjectToFile(const std::string& strURI, boost::intrusive_ptr<ISXS> spObject)
{
	std::ofstream out(strURI.c_str());
	writeObject(out, spObject);
}

void SXSParser::writeObjectToString(std::string* pStr, boost::intrusive_ptr<ISXS> spObject)
{
	assert(pStr);
	std::ostringstream out;
	writeObject(out, spObject);
	pStr->swap(out.str());
}

void SXSParser::readObject(std::istream& in, boost::intrusive_ptr<ISXS>& spObject)
{
	spObject = 0;

	SXSHandler handler(m_factoryMap);
	if (XMLUtils::parse(in, handler))
	{
		spObject = handler.getRootObject();
	}
}

void SXSParser::readObjectFromFile(const std::string& strURI, boost::intrusive_ptr<ISXS>& spObject)
{
	spObject = 0;

	SXSHandler handler(m_factoryMap);
	if (XMLUtils::parseFromFile(strURI, handler))
	{
		spObject = handler.getRootObject();
	}
}

void SXSParser::readObjectFromString(const char* pStr, int nStrSize, boost::intrusive_ptr<ISXS>& spObject)
{
	spObject = 0;

	SXSHandler handler(m_factoryMap);
	if (XMLUtils::parseFromString(pStr, nStrSize, handler))
	{
		spObject = handler.getRootObject();
	}
}
