/******************************************************************************
pyxcom.cpp

begin		: 2006-12-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/pyxcom.h"

// pyxis includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/file_utils.h"

// windows includes
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
#define _WIN32_WINNT 0x0502 // to get SetDllDirectory (requires Windows XP SP1 or greater)
#include <windows.h>
#pragma warning(pop)

// boost includes
#include <boost/filesystem/operations.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

namespace
{

//! Type of map for class object management.
typedef std::map<CLSID, LPFNGCO> MapClassObjectType;

//! Map for class object management.
MapClassObjectType mapClassObject;

//! Handles to loaded DLLs (containing GetClassObject functions).
std::vector<HINSTANCE> vechDll;

//! Reference counting for initialization.
int nInitRC = 0;

//! Indicates if the library has ever been initialized.
bool bInitialized = false;

//! Helper class for implementing class object enumerator.
class CompositeEnumClassObject : public IEnumClassObject
{
public:

	CompositeEnumClassObject() :
		m_it(mapClassObject.begin())
	{
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_IMPL(IEnumClassObject);

	IUNKNOWN_RC_IMPL();

public: // IEnumClassObject

	virtual HRESULT STDMETHODCALLTYPE Next(ULONG nCount, PYXCOM_IUnknown** ppObject, ULONG* pnFetched)
	{
		assert(ppObject != 0);

		if (1 < nCount && pnFetched == 0)
		{
			return E_INVALIDARG;
		}

		ULONG nFetched = 0;
		while (nFetched < nCount && m_it != mapClassObject.end())
		{
			m_it->second(m_it->first, PYXCOM_IUnknown::iid, (void**) &ppObject[nFetched]);
			++nFetched;
			++m_it;
		}

		if (pnFetched != 0)
		{
			*pnFetched = nFetched;
		}

		return nFetched == nCount ? S_OK : S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE Skip(ULONG nCount)
	{
		ULONG nFetched = 0;
		while (nFetched < nCount && m_it != mapClassObject.end())
		{
			++nFetched;
			++m_it;
		}

		return nFetched == nCount ? S_OK : S_FALSE;
	}

	virtual HRESULT STDMETHODCALLTYPE Reset()
	{
		m_it = mapClassObject.begin();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Clone(IEnumClassObject** ppObject)
	{
		assert(ppObject != 0);

		*ppObject = new CompositeEnumClassObject(*this);
		if (*ppObject == 0)
		{
			return E_OUTOFMEMORY;
		}

		(*ppObject)->AddRef();
		return S_OK;
	}

private:

	MapClassObjectType::const_iterator m_it;
};

}

const GUID GUID_NULL = { 0 };

// {00000000-0000-0000-C000-000000000046}
PYXCOM_DEFINE_IID(PYXCOM_IUnknown, 
0x0, 0x0, 0xc000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46);

// {00000001-0000-0000-C000-000000000046}
PYXCOM_DEFINE_IID(PYXCOM_IClassFactory, 
0x1, 0x0, 0xc000, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x46);

// {46FC6A2A-2121-4fbf-8354-DB550324467A}
PYXCOM_DEFINE_IID(IClassInfo, 
0x46fc6a2a, 0x2121, 0x4fbf, 0x83, 0x54, 0xdb, 0x55, 0x3, 0x24, 0x46, 0x7a);

// {C68144C5-1978-4f77-A5A1-AF67A9C4AF1D}
PYXCOM_DEFINE_IID(IEnumClassObject, 
0xc68144c5, 0x1978, 0x4f77, 0xa5, 0xa1, 0xaf, 0x67, 0xa9, 0xc4, 0xaf, 0x1d);

/*!
Loads DLLs in the plugins directory, seeking implementations of
IEnumClassObject.

Calls to PYXCOMInitialize and PYXCOMUninitialize must match.
*/
HRESULT PYXCOMInitialize()
{
	if (++nInitRC != 1)
	{
		return S_OK;
	}

// NOTE: Some Windows APIs require UNICODE strings. We assume
// all our strings are 8 bit clean and simply convert between
// string and wstring as needed.
#ifdef UNICODE
	typedef std::wstring tstring;
#else
	typedef std::string tstring;
#endif

	TRACE_INFO("===== Initializing PYXCOM =====");

	if (bInitialized)
	{
		assert(false && "Can't initialize PYXCOM a second time.");
		return S_FALSE;
	}

	HRESULT hr;
	boost::filesystem::path plugins(AppServices::getApplicationPath() / "plugins");

	if (FileUtils::exists(plugins) && FileUtils::isDirectory(plugins))
	{
		TRACE_INFO("using directory '" << FileUtils::pathToString(plugins) << '\'');

		// NOTE: SetDllDirectory is a Windows-specific API available only
		// on Windows XP SP1 and greater.
		std::string pluginDir = FileUtils::pathToString(plugins);
		tstring strPlugins(pluginDir.begin(), pluginDir.end());
		if (SetDllDirectory(strPlugins.c_str()) == 0)
		{
			TRACE_ERROR("SetDllDirectory failed.");
		}

		boost::filesystem::directory_iterator itEnd;
		boost::filesystem::directory_iterator itDir(plugins);

		for (; itDir != itEnd; ++itDir)
		{
			boost::filesystem::path item = *itDir;

			if (FileUtils::isDirectory(item))
			{
				continue;
			}

			// Only try files whose names match this filter
			static const boost::regex e(".+\\.[dD][lL][lL]$");

			if (!boost::regex_match(FileUtils::pathToString(item.leaf()), e))
			{
				// Skip if file extension doesn't match
				continue;
			}

			TRACE_INFO("loading '" << FileUtils::pathToString(item.leaf()) << '\'');

			// NOTE: LoadLibrary, GetProcAddress, etc. are Windows-specific APIs.
			std::string itemDll = FileUtils::pathToString(item);
			tstring strDll(itemDll.begin(), itemDll.end());
			HINSTANCE hDll = LoadLibrary(strDll.c_str());
			if (hDll == 0)
			{
				// http://msdn2.microsoft.com/en-us/library/ms680582.aspx
				DWORD dw = GetLastError();
				LPTSTR lpMsgBuf;
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0, NULL );
				tstring strMsg(lpMsgBuf, lpMsgBuf + lstrlen(lpMsgBuf));
				TRACE_ERROR("  --> unable to load library: (" << dw << ") " << std::string(strMsg.begin(), strMsg.end()));
				LocalFree(lpMsgBuf);
				continue;
			}

			LPFNGCO pfnGCO =
				(LPFNGCO) GetProcAddress(hDll, "PYXCOMDllGetClassObject");
			if (pfnGCO == 0)
			{
				TRACE_INFO("  --> no suitable entry point found");
// TODO: Make sure tests etc. are unregistered before freeing the library.
#if defined(FREE_LIBRARY)
				FreeLibrary(hDll);
#endif
				continue;
			}

			MapClassObjectType::size_type nMapSize = mapClassObject.size();

			hr = PYXCOMRegister(pfnGCO);
			if (FAILED(hr))
			{
// TODO: Make sure tests etc. are unregistered before freeing the library.
#if defined(FREE_LIBRARY)
				FreeLibrary(hDll);
#endif
				continue;
			}

			if (nMapSize < mapClassObject.size())
			{
				vechDll.push_back(hDll);
			}
			else
			{
// TODO: Make sure tests etc. are unregistered before freeing the library.
#if defined(FREE_LIBRARY)
				FreeLibrary(hDll);
#endif
				TRACE_INFO("  --> no suitable class info found");
			}
		}
	}
	else
	{
		TRACE_ERROR("couldn't find directory '" << FileUtils::pathToString(plugins) << '\'');
	}

	TRACE_INFO("===== PYXCOM initialized =====");
	bInitialized = true;

	return S_OK;
}

/*!
Unloads DLLs in the plugins directory.

Calls to PYXCOMInitialize and PYXCOMUninitialize must match.
*/
HRESULT PYXCOMUninitialize()
{
	assert(0 < nInitRC);

	if (--nInitRC < 0)
	{
		assert(false && "PYXCOM Already uninitialized, can't repeat operation.");
		return S_FALSE;
	}

	return S_OK;


#if 0

If dll's are freed before all of the processes are cleaned up then the application 
is in an error state. This can happen when a .Net system is built upon PYXLib. In
that case it is possible to have the main application exit (and pyxlib uninitialized)
before all of the PYXCom objects associated with other .Net objects are cleanud up.

	for (int n = static_cast<int>(vechDll.size()) - 1; 0 <= n; --n)
	{
		FreeLibrary(vechDll[n]);
	}
	vechDll.clear();
	mapClassObject.clear();

	return S_OK;
#endif
}

/*!
Manually register.
*/
HRESULT PYXCOMRegister(LPFNGCO pfnGCO)
{
	boost::intrusive_ptr<IEnumClassObject> spECO;
	HRESULT hr = pfnGCO(CLSID_NULL, IEnumClassObject::iid, (void**) &spECO);
	if (FAILED(hr))
	{
		TRACE_INFO("  --> no class object enumerator found");
		return hr;
	}

	while (true)
	{
		boost::intrusive_ptr<PYXCOM_IUnknown> spCO;
		if (spECO->Next(1, (PYXCOM_IUnknown**) &spCO, 0) != S_OK)
		{
			break;
		}

		boost::intrusive_ptr<IClassInfo> spCI;
		if (SUCCEEDED(spCO->QueryInterface(IClassInfo::iid, (void**) &spCI)))
		{
			const CLSID* pclsid;
			spCI->getClass(&pclsid);
			TRACE_INFO("  --> found clsid " << *pclsid);

			if (mapClassObject.find(*pclsid) == mapClassObject.end())
			{
				mapClassObject[*pclsid] = pfnGCO;
			}
			else
			{
				// TODO: Need a policy decision here.
				TRACE_ERROR("  --> duplicate clsid found");
			}
		}
	}

	return S_OK;
}

/*!
Fun fact: apparently there are enough GUIDs to uniquely label every
molecule on Earth.
*/
HRESULT PYXCOMCreateGuid(GUID* pguid)
{
	assert(pguid);

#ifdef _WINDOWS
	return CoCreateGuid(pguid);
#else
	return E_NOTIMPL;
#endif
}

/*!
Recognizes a special case to enumerate all class objects: simply request
IEnumClassObject::iid with CLSID_NULL.
*/
HRESULT PYXCOMGetClassObject(REFCLSID rclsid, REFIID riid, void** ppObject)
{
	// See Essential COM p114

	// Special case: asking to enumerate all class objects
	if (riid == IEnumClassObject::iid)
	{
		IEnumClassObject* pECO = new CompositeEnumClassObject;
		if (pECO == 0)
		{
			return E_OUTOFMEMORY;
		}

		pECO->AddRef();
		HRESULT hr = pECO->QueryInterface(riid, ppObject);
		pECO->Release();
		return hr;
	}

	MapClassObjectType::const_iterator it = mapClassObject.find(rclsid);

	if (it != mapClassObject.end())
	{
		// This is a function call
		return (it->second)(rclsid, riid, ppObject);
	}

	return CLASS_E_CLASSNOTAVAILABLE;
}

/*!
Class aggregation is not supported, so pUnkOuter must be null.
*/
HRESULT PYXCOMCreateInstance(REFCLSID rclsid, PYXCOM_IUnknown* pUnkOuter, REFIID riid, void** ppObject)
{
	// See Essential COM p118

	boost::intrusive_ptr<PYXCOM_IClassFactory> spCF;
	HRESULT hr = PYXCOMGetClassObject(rclsid, PYXCOM_IClassFactory::iid, (void**) &spCF);
	if (SUCCEEDED(hr))
	{
		hr = spCF->CreateInstance(pUnkOuter, riid, ppObject);
	}
	else
	{
		TRACE_ERROR("Failed to find class factory " << guidToStr(rclsid) << " - could be that there are missing plugins dlls.");
	}

	return hr;
}

/*!
Returns the CLSIDs that implement the specified IID.
*/
PYXLIB_DECL HRESULT PYXCOMGetClassIDs(REFIID riid, std::vector<CLSID>* pVecClsid)
{
	assert(pVecClsid != 0);
	pVecClsid->clear();

	HRESULT hr;

	boost::intrusive_ptr<IEnumClassObject> spECO;
	hr = PYXCOMGetClassObject(CLSID_NULL, IEnumClassObject::iid, (void**) &spECO);
	if (FAILED(hr))
	{
		TRACE_INFO("couldn't enumerate class objects");
	}

	while (true)
	{
		boost::intrusive_ptr<PYXCOM_IUnknown> spCO;
		if (spECO->Next(1, (PYXCOM_IUnknown**) &spCO, 0) != S_OK)
		{
			break;
		}

		boost::intrusive_ptr<IClassInfo> spCI;
		if (SUCCEEDED(spCO->QueryInterface(IClassInfo::iid, (void**) &spCI)))
		{
			int nCount;
			spCI->getNumInterfaces(&nCount);
			const IID* piid;
			spCI->getInterfaces(&piid);
			if (std::find(piid, piid + nCount, riid) != piid + nCount)
			{
				const CLSID* pclsid;
				spCI->getClass(&pclsid);
				pVecClsid->push_back(*pclsid);
			}
		}
	}

	return hr;
}

/*!
Determine if a particular class implements a particular interface.

\param rclsid	The class to test the implementation of.
\param riid		The interface that the class is being queried for.

\return true if the class implements the interface, false if it doesn't,
		or if it can't be determined (could be false negative).
*/
PYXLIB_DECL bool PYXCOMImplementsInterface(REFCLSID rclsid, REFIID riid)
{
	HRESULT hr;

	boost::intrusive_ptr<IEnumClassObject> spECO;
	hr = PYXCOMGetClassObject(CLSID_NULL, IEnumClassObject::iid, (void**) &spECO);
	if (FAILED(hr))
	{
		TRACE_INFO("couldn't enumerate class objects");
	}

	while (true)
	{
		boost::intrusive_ptr<PYXCOM_IUnknown> spCO;
		if (spECO->Next(1, (PYXCOM_IUnknown**) &spCO, 0) != S_OK)
		{
			break;
		}

		boost::intrusive_ptr<IClassInfo> spCI;
		if (SUCCEEDED(spCO->QueryInterface(IClassInfo::iid, (void**) &spCI)))
		{
			const CLSID* pclsid;
			spCI->getClass(&pclsid);

			if (*pclsid == rclsid)
			{
				int nCount;
				spCI->getNumInterfaces(&nCount);
				const IID* piid;
				spCI->getInterfaces(&piid);
				return std::find(piid, piid + nCount, riid) != piid + nCount;
			}
		}
	}

	// undetermined (could be false negative)
	return false;
}

/*!
Example of canonical form: {00000000-0000-0000-C000-000000000046}
*/
template <typename T>
static
std::basic_ostream<T>& serialize(std::basic_ostream<T>& out, REFGUID guid)
{
	std::ios_base::fmtflags oldflags =
		out.flags(std::ios_base::hex | std::ios_base::uppercase);
	int oldfill = out.fill();
	out.fill('0');
	out << '{';
	out << std::setw(8) << guid.Data1;
	out << '-';
	out << std::setw(4) << guid.Data2;
	out << '-';
	out << std::setw(4) << guid.Data3;
	out << '-';
	out << std::setw(2) << static_cast<int>(guid.Data4[0]);
	out << std::setw(2) << static_cast<int>(guid.Data4[1]);
	out << '-';
	out << std::setw(2) << static_cast<int>(guid.Data4[2]);
	out << std::setw(2) << static_cast<int>(guid.Data4[3]);
	out << std::setw(2) << static_cast<int>(guid.Data4[4]);
	out << std::setw(2) << static_cast<int>(guid.Data4[5]);
	out << std::setw(2) << static_cast<int>(guid.Data4[6]);
	out << std::setw(2) << static_cast<int>(guid.Data4[7]);
	out << '}';
	out.flags(oldflags);
	out.fill(oldfill);
	return out;
}

/*!
Example of canonical form: {00000000-0000-0000-C000-000000000046}
*/
template <typename T>
static 
std::basic_istream<T>& deserialize(std::basic_istream<T>& in, GUID& guid)
{
	// TODO: Replace reinterpret casts with something a little cleaner.

	// TODO: Return an error code in "in".

	// TODO Can't guarantee that this is as rock solid as the output
	// function, but it does seem to work.

	std::ios_base::fmtflags oldflags =
		in.flags(std::ios_base::hex | std::ios_base::uppercase);

	bool hasBrackets = false;
	if (in.peek() == '{')
	{
		in.get();
		hasBrackets = true;
	}

	in >> guid.Data1;

	if (in.get() != '-')
	{
		return in;
	}

	in >> guid.Data2;

	if (in.get() != '-')
	{
		return in;
	}

	in >> guid.Data3;

	if (in.get() != '-')
	{
		return in;
	}

	T buf[2];
	unsigned int n;

	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[0] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[1] = n;

	if (in.get() != '-')
	{
		return in;
	}

	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[2] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[3] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[4] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[5] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[6] = n;
	in.read(buf, 2);
	sscanf_s(reinterpret_cast<char *>(buf), "%2X", &n);
	guid.Data4[7] = n;

	if (hasBrackets && in.get() != '}')
	{
		return in;
	}

	in.flags(oldflags);
	return in;
}

/*!
Example of canonical form: {00000000-0000-0000-C000-000000000046}
*/
std::ostream& operator <<(std::ostream& out, REFGUID guid)
{
	return serialize(out, guid);
}

/*!
Example of canonical form: {00000000-0000-0000-C000-000000000046}
*/
std::istream& operator >>(std::istream& in, GUID& guid)
{
	return deserialize(in, guid);
}

///*!
//Example of canonical form: {00000000-0000-0000-C000-000000000046}
//*/
//std::basic_ostream<unsigned char>& operator <<(std::basic_ostream<unsigned char>& out, REFGUID guid)
//{
//	return serialize(out, guid);
//}
//
///*!
//Example of canonical form: {00000000-0000-0000-C000-000000000046}
//*/
//std::basic_istream<unsigned char>& operator >>(std::basic_istream<unsigned char>& in, GUID& guid)
//{
//	return deserialize(in, guid);
//}

namespace PYXCOM
{
	std::map<IID,boost::intrusive_ptr<PYXCOM_IUnknown>> DI::s_defaultObjects;
	std::map<IID,CLSID> DI::s_defaultClasses;
	boost::recursive_mutex di_mutex;


	boost::intrusive_ptr<PYXCOM_IUnknown> DI::get(REFIID iid)
	{	
		CLSID classToCreate;

		{
			boost::recursive_mutex::scoped_lock lock(di_mutex);

			auto defaultObject = s_defaultObjects.find(iid);
			if (defaultObject != s_defaultObjects.end())
			{
				return defaultObject->second;
			}

			auto defaultClass = s_defaultClasses.find(iid);
			if (defaultClass == s_defaultClasses.end())
			{
				std::vector<CLSID> classes;

				PYXCOMGetClassIDs(iid,&classes);

				if (classes.empty())
				{
					return NULL;
				}

				s_defaultClasses[iid] = classes.front();
				classToCreate = classes.front();			
			}
			else
			{
				classToCreate = defaultClass->second;
			}
		}

		return PYXCOMCreateInstance<PYXCOM_IUnknown>(classToCreate);
	}

	//get an implementation of an interface using a specific class
	boost::intrusive_ptr<PYXCOM_IUnknown> DI::get(REFIID iid,REFCLSID clsid)
	{
		return PYXCOMCreateInstance<PYXCOM_IUnknown>(clsid);
	}

	//get all implementations of a single interface
	std::vector<boost::intrusive_ptr<PYXCOM_IUnknown>> DI::getAll(REFIID iid)
	{
		std::vector<boost::intrusive_ptr<PYXCOM_IUnknown>> result;

		std::vector<CLSID> classes;
		PYXCOMGetClassIDs(iid,&classes);

		for(auto & clsid : classes)
		{
			result.push_back(PYXCOMCreateInstance<PYXCOM_IUnknown>(clsid));
		}

		return result;
	}

	//set the default class for an interface
	void DI::set(REFIID iid,REFCLSID clsid)
	{
		boost::recursive_mutex::scoped_lock lock(di_mutex);		
		s_defaultClasses[iid] = clsid;
	}
	
	//set a default object for an interface
	void DI::set(REFIID iid,const boost::intrusive_ptr<PYXCOM_IUnknown> & item)
	{
		boost::recursive_mutex::scoped_lock lock(di_mutex);
		s_defaultObjects[iid] = item;
	}
};