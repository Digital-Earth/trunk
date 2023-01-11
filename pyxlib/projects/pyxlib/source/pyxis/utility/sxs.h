#ifndef PYXIS__UTILITY__SXS_H
#define PYXIS__UTILITY__SXS_H
/******************************************************************************
sxs.h

begin		: 2007-06-19
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/intrusive_ptr.hpp>

// standard includes
#include <cassert>
#include <iosfwd>
#include <map>
#include <string>

//! DOM-like interface for simple XML serialization.
struct PYXLIB_DECL ISXS : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Attribute map holds name value strings.
	typedef std::map<std::string, std::string> AttributeMap;

public:

	///////
	// Name

	//! Returns the XML element name.
	virtual std::string STDMETHODCALLTYPE getSXSName() const = 0;

	//! Sets the XML element name.
	virtual void STDMETHODCALLTYPE setSXSName(const std::string& strName) = 0;

	/////////////
	// Attributes

	//! Returns whether the object has any XML attributes.
	virtual bool STDMETHODCALLTYPE hasSXSAttributes() const = 0;

	//! Returns whether the object has an XML attribute.
	virtual bool STDMETHODCALLTYPE hasSXSAttribute(const std::string& strName) const = 0;

	//! Returns the XML attributes.
	virtual AttributeMap STDMETHODCALLTYPE getSXSAttributes() const = 0;

	//! Returns an XML attribute.
	virtual std::string STDMETHODCALLTYPE getSXSAttribute(const std::string& strName) const = 0;

	//! Sets the XML attributes.
	virtual void STDMETHODCALLTYPE setSXSAttributes(const AttributeMap& attrMap) = 0;

	//! Sets an XML attribute.
	virtual void STDMETHODCALLTYPE setSXSAttribute(const std::string& strName, const std::string& strValue) = 0;

	//! Removes an XML attribute.
	virtual void STDMETHODCALLTYPE removeSXSAttribute(const std::string& strName) = 0;

	//! Clears the XML attributes.
	virtual void STDMETHODCALLTYPE clearSXSAttributes() = 0;

	///////////
	// Children

	//! Returns whether the object has any XML child elements.
	virtual bool STDMETHODCALLTYPE hasSXSChildren() const = 0;

	//! Returns the number of XML child elements.
	virtual int STDMETHODCALLTYPE getNumberOfSXSChildren() const = 0;

	//! Returns the specified XML child element.
	virtual boost::intrusive_ptr<ISXS> STDMETHODCALLTYPE getSXSChild(int n) const = 0;

	//! Sets the specified XML child element.
	virtual void STDMETHODCALLTYPE setSXSChild(int n, boost::intrusive_ptr<ISXS> spObject) = 0;

	//! Adds an XML child element.
	virtual void STDMETHODCALLTYPE addSXSChild(boost::intrusive_ptr<ISXS> spObject) = 0;

	//! Inserts an XML child element.
	virtual void STDMETHODCALLTYPE insertSXSChild(int n, boost::intrusive_ptr<ISXS> spObject) = 0;

	//! Removes an XML child element.
	virtual void STDMETHODCALLTYPE removeSXSChild(int n) = 0;

	//! Clears the XML child elements.
	virtual void STDMETHODCALLTYPE clearSXSChildren() = 0;

	/////////////
	// Characters

	//! Returns whether the object has any XML characters.
	virtual bool STDMETHODCALLTYPE hasSXSCharacters() const = 0;

	//! Returns the XML characters.
	virtual std::string STDMETHODCALLTYPE getSXSCharacters() const = 0;

	//! Sets the XML characters.
	virtual void STDMETHODCALLTYPE setSXSCharacters(const std::string strCharacters) = 0;

	//! Clears the XML characters.
	virtual void STDMETHODCALLTYPE clearSXSCharacters() = 0;

	/////////////////
	// Initialization

	//! Initializes the object. (Called when XML element ends.)
	virtual void STDMETHODCALLTYPE initSXS() = 0;

};

//! Default implementation of ISXS interface.
class PYXLIB_DECL DefaultSXSObject : public ISXS
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(ISXS)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // ISXS

	virtual std::string STDMETHODCALLTYPE getSXSName() const
	{
		return m_strName;
	}

	virtual void STDMETHODCALLTYPE setSXSName(const std::string& strName)
	{
		m_strName = strName;
	}

	virtual bool STDMETHODCALLTYPE hasSXSAttributes() const
	{
		return !m_attrMap.empty();
	}

	virtual bool STDMETHODCALLTYPE hasSXSAttribute(const std::string& strName) const
	{
		return m_attrMap.find(strName) != m_attrMap.end();
	}

	virtual AttributeMap STDMETHODCALLTYPE getSXSAttributes() const
	{
		return m_attrMap;
	}

	virtual std::string STDMETHODCALLTYPE getSXSAttribute(const std::string& strName) const
	{
		ISXS::AttributeMap::const_iterator it = m_attrMap.find(strName);
		return it != m_attrMap.end() ? it->second : "";
	}

	virtual void STDMETHODCALLTYPE setSXSAttributes(const AttributeMap& attrMap)
	{
		m_attrMap = attrMap;
	}

	virtual void STDMETHODCALLTYPE setSXSAttribute(const std::string& strName, const std::string& strValue)
	{
		m_attrMap[strName] = strValue;
	}

	virtual void STDMETHODCALLTYPE removeSXSAttribute(const std::string& strName)
	{
		m_attrMap.erase(strName);
	}

	virtual void STDMETHODCALLTYPE clearSXSAttributes()
	{
		m_attrMap.clear();
	}

	virtual bool STDMETHODCALLTYPE hasSXSChildren() const
	{
		return !m_vecChildren.empty();
	}

	virtual int STDMETHODCALLTYPE getNumberOfSXSChildren() const
	{
		return static_cast<int>(m_vecChildren.size());
	}

	virtual boost::intrusive_ptr<ISXS> STDMETHODCALLTYPE getSXSChild(int n) const
	{
		return m_vecChildren[n];
	}

	virtual void STDMETHODCALLTYPE setSXSChild(int n, boost::intrusive_ptr<ISXS> spObject)
	{
		assert(spObject);
		m_vecChildren[n] = spObject;
	}

	virtual void STDMETHODCALLTYPE addSXSChild(boost::intrusive_ptr<ISXS> spObject)
	{
		assert(spObject);
		m_vecChildren.push_back(spObject);
	}

	virtual void STDMETHODCALLTYPE insertSXSChild(int n, boost::intrusive_ptr<ISXS> spObject)
	{
		assert(spObject);
		m_vecChildren.insert(m_vecChildren.begin() + n, spObject);
	}

	virtual void STDMETHODCALLTYPE removeSXSChild(int n)
	{
		m_vecChildren.erase(m_vecChildren.begin() + n);
	}

	virtual void STDMETHODCALLTYPE clearSXSChildren()
	{
		m_vecChildren.clear();
	}

	virtual bool STDMETHODCALLTYPE hasSXSCharacters() const
	{
		return !m_strCharacters.empty();
	}

	virtual std::string STDMETHODCALLTYPE getSXSCharacters() const
	{
		return m_strCharacters;
	}

	virtual void STDMETHODCALLTYPE setSXSCharacters(const std::string strCharacters)
	{
		m_strCharacters = strCharacters;
	}

	virtual void STDMETHODCALLTYPE clearSXSCharacters()
	{
		m_strCharacters.clear();
	}

	virtual void STDMETHODCALLTYPE initSXS()
	{
	}

private:

	std::string m_strName;
	ISXS::AttributeMap m_attrMap;
	std::vector<boost::intrusive_ptr<ISXS> > m_vecChildren;
	std::string m_strCharacters;

};

//! Factory interface for simple XML serialization.
struct PYXLIB_DECL ISXSFactory : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Name vector.
	typedef std::vector<std::string> NameVector;

public:

	//! Returns the names this factory can handle.
	virtual NameVector STDMETHODCALLTYPE getSXSNames() const = 0;

	//! Creates the specified SXS object. May return null.
	virtual boost::intrusive_ptr<ISXS> STDMETHODCALLTYPE createSXSObject(const std::string& strName, const ISXS::AttributeMap& attrMap) = 0;

};

/*!
Typically the name template parameter must be declared like this to have
external linkage: char my_name[] = "my_name";
*/
//! Default implementation of ISXSFactory interface.
template <typename T, const char* const name>
class PYXLIB_DECL DefaultSXSFactory : public ISXSFactory
{

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(ISXSFactory)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // ISXSFactory

	virtual NameVector STDMETHODCALLTYPE getSXSNames() const
	{
		return NameVector(1, name);
	}

	virtual boost::intrusive_ptr<ISXS> STDMETHODCALLTYPE createSXSObject(const std::string& strName, const ISXS::AttributeMap& attrMap)
	{
		assert(strName == name);
		return new T();
	}

};

//! Parser for simple XML serialization.
class PYXLIB_DECL SXSParser
{

public:

	//! Factory map.
	typedef std::map<std::string, boost::intrusive_ptr<ISXSFactory> > FactoryMap;

public:

	//! Unit testing.
	static void test();

public:

	/*!
	Use this for simple parsing. Don't modify it unless you really intend to
	modify the default. If you want a new parser with the defaults so you
	can modify a copy, then just copy the default.
	*/
	//! Returns the default parser.
	static SXSParser& getDefaultParser()
	{
		static SXSParser defaultParser;
		return defaultParser;
	}

public:

	//! Registers the factory under the names it handles.
	void registerFactory(boost::intrusive_ptr<ISXSFactory> spFactory);

	//! Unregisters the factory under the names it handles.
	void unregisterFactory(boost::intrusive_ptr<ISXSFactory> spFactory);

	//! Unregisters all factories.
	void unregisterAllFactories();

public:

	//! Writes an object to a stream.
	void writeObject(std::ostream& out, boost::intrusive_ptr<ISXS> spObject);
	
	//! Writes an object to a file.
	void writeObjectToFile(const std::string& strURI, boost::intrusive_ptr<ISXS> spObject);

	//! Writes an object to a string.
	void writeObjectToString(std::string* pStr, boost::intrusive_ptr<ISXS> spObject);

	//! Reads an object from a stream.
	void readObject(std::istream& in, boost::intrusive_ptr<ISXS>& spObject);

	//! Reads an object from a file.
	void readObjectFromFile(const std::string& strURI, boost::intrusive_ptr<ISXS>& spObject);

	//! Reads an object from a string.
	void readObjectFromString(const char* pStr, int nStrSize, boost::intrusive_ptr<ISXS>& spObject);

private:

	//! The factory map.
	FactoryMap m_factoryMap;

};

#endif // guard
