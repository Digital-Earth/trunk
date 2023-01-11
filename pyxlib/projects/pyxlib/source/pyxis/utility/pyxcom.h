#ifndef PYXIS__UTILITY__PYXCOM_H
#define PYXIS__UTILITY__PYXCOM_H
/******************************************************************************
pyxcom.h

begin		: 2006-12-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/trace.h"

// boost includes
#include <boost/detail/atomic_count.hpp>
#include <boost/intrusive_ptr.hpp>

// standard includes
#include <cassert>
#include <iosfwd>
#include <sstream>
#include <vector>

// Special private include
#include "pyxis/utility/pyxcom_private.h"

////////////////////////////////////////////////////////////////////////////////

// NOTE This could be a variadic macro. Alternatively, if we switch to an
// entirely data-driven mechanism for QI, it could also support introspection
// (i.e. IClassInfo) and kill two birds with one stone.

//! Helper macro for implementing query interface (use in public section
//! of class). This accounts for the common simple case of an interface
//! which derives only from PYXCOM_IUnknown.
#define IUNKNOWN_QI_IMPL(IFace) \
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppObject) \
	{ \
		/* See Essential COM p65 */ \
		assert(ppObject != 0); \
		if (riid == IFace::iid || riid == PYXCOM_IUnknown::iid) \
		{ \
			*ppObject = static_cast<IFace*>(this); \
		} \
		else \
		{ \
			*ppObject = 0; \
			return PYXCOM_NOINTERFACE; \
		} \
		AddRef(); \
		return PYXCOM_OK; \
	}

// Lots of helper macros, use them like this:
//IUNKNOWN_QI_BEGIN
//	IUNKNOWN_QI_CASE(IFaceA)
//	IUNKNOWN_QI_CASE(IFaceB)
//IUNKNOWN_QI_END

//! Helper macro for implementing query interface (use in public section
//! of class). Allows for a more flexible implementation.
#define IUNKNOWN_QI_BEGIN \
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppObject) \
	{ \
		/* See Essential COM p65 */ \
		assert(ppObject != 0); \
		/* If pointer was not null. we have a problem*/ \
		assert(*ppObject == 0); \
		if (riid == PYXCOM_IUnknown::iid) \
		{ \
			*ppObject = this; \
		}

//! Helper macro for implementing query interface (use in public section
//! of class). Allows for a more flexible implementation.
#define IUNKNOWN_QI_CASE(IFace) \
		else if (riid == IFace::iid) \
		{ \
			*ppObject = static_cast<IFace*>(this); \
		}

//! Helper macro for implementing query interface (use in public section
//! of class). Allows for a more flexible implementation.
#define IUNKNOWN_QI_END \
		else \
		{ \
			*ppObject = 0; \
			return PYXCOM_NOINTERFACE; \
		} \
		AddRef(); \
		return PYXCOM_OK; \
	}

////////////////////////////////////////////////////////////////////////////////

//! Helper class for implementing reference counting.
class PYXLIB_DECL RCImpl
{
public:

	RCImpl() : m_nRC(0) {}
	RCImpl(const RCImpl& rhs) : m_nRC(0) {}

	PYXCOM_ULONG STDMETHODCALLTYPE AddRef() { return ++m_nRC; }
	PYXCOM_ULONG STDMETHODCALLTYPE Release() { return --m_nRC; }

private:

	mutable boost::detail::atomic_count m_nRC;
};

//! Helper macro for implementing reference counting (use in public section
//! of class).
#define IUNKNOWN_RC_IMPL() \
private: \
	mutable RCImpl m_RC; \
public: \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE AddRef() \
	{ \
		return m_RC.AddRef(); \
	} \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE Release() \
	{ \
		PYXCOM_ULONG nRC = m_RC.Release(); \
		assert(0 <= nRC); \
		if (nRC == 0) \
		{ \
			delete this; \
		} \
		return nRC; \
	}

//! Helper macro for implementing reference counting (use in public section
//! of class).
#define IUNKNOWN_RC_IMPL_FINALIZE() \
private: \
	mutable RCImpl m_RC; \
public: \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE AddRef() \
	{ \
		return m_RC.AddRef(); \
	} \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE Release() \
	{ \
		PYXCOM_ULONG nRC = m_RC.Release(); \
		assert(0 <= nRC); \
		if (nRC == 0) \
		{ \
			m_RC.AddRef(); \
			finalize(); \
			m_RC.Release(); \
			delete this; \
		} \
		return nRC; \
	}

//! Helper macro for implementing reference counting (use in public section
//! of class). See Essential COM p65
#define IUNKNOWN_RC_IMPL_NODELETE() \
public: \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE AddRef() \
	{ \
		return 2; \
	} \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE Release() \
	{ \
		return 1; \
	}

//! Helper macro for implementing reference counting (use in public section
//! of class).
#define IUNKNOWN_RC_IMPL_DELEGATE(x) \
public: \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE AddRef() \
	{ \
		return x.AddRef(); \
	} \
	virtual PYXCOM_ULONG STDMETHODCALLTYPE Release() \
	{ \
		return x.Release(); \
	}

////////////////////////////////////////////////////////////////////////////////

// Put this in the public section of an interface.
#define PYXCOM_DECLARE_INTERFACE() \
	public: \
		static const IID iid

// Put this in the public section of a class.
#define PYXCOM_DECLARE_CLASS() \
	public: \
		static const CLSID clsid; \
		static const IID aiid[]; \
		static const int niid;

// Lots of helper macros, use them like this:
//PYXCOM_BEGIN_CLASS_OBJECT_TABLE
//	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(MyClassA),
//	PYXCOM_CLASS_OBJECT_TABLE_ENTRY(MyClassB),
//PYXCOM_END_CLASS_OBJECT_TABLE

// For reference regarding GetProcAddress and DLL exporting:
// http://msdn2.microsoft.com/en-us/library/64tkc9y5(VS.80).aspx
// http://blogs.msdn.com/oldnewthing/archive/2004/01/12/57833.aspx

#define PYXCOM_CLASS_OBJECT_MANAGER \
	struct ClassObjectManager \
	{ \
		ClassObjectManager() \
		{ \
			for (int n = 0; n != sizeof(aClassObject)/sizeof(aClassObject[0]); ++n) \
			{ \
				PYXCOM_ULONG nRC = aClassObject[n]->AddRef(); \
				assert(nRC == 1); \
			} \
		} \
		~ClassObjectManager() \
		{ \
			for (int n = sizeof(aClassObject)/sizeof(aClassObject[0]) - 1; 0 <= n; --n) \
			{ \
				PYXCOM_ULONG nRC = aClassObject[n]->Release(); \
				assert(nRC == 0); \
			} \
		} \
	} classObjectManager;

#define PYXCOM_GCO_FUNCTION(GCO_NAME) \
	extern "C" __declspec(dllexport) PYXCOM_HRESULT GCO_NAME(REFCLSID rclsid, REFIID riid, void** ppObject) \
	{ \
		assert(ppObject != 0); \
		if (riid == IEnumClassObject::iid) \
		{ \
			IEnumClassObject* pECO = new EnumClassObjectImpl(aClassObject, sizeof(aClassObject)/sizeof(aClassObject[0])); \
			if (pECO == 0) \
			{ \
				return PYXCOM_OUTOFMEMORY; \
			} \
			pECO->AddRef(); \
			PYXCOM_HRESULT hr = pECO->QueryInterface(riid, ppObject); \
			pECO->Release(); \
			return hr; \
		} \
		for (int n = 0; n != sizeof(aClassObject)/sizeof(aClassObject[0]); ++n) \
		{ \
			const CLSID* pclsid = 0; \
			dynamic_cast<IClassInfo*>(aClassObject[n])->getClass(&pclsid); \
			if (rclsid == *pclsid) \
			{ \
				return aClassObject[n]->QueryInterface(riid, ppObject); \
			} \
		} \
		return PYXCOM_OK; \
	}

#define PYXCOM_BEGIN_CLASS_OBJECT_TABLE \
	namespace { PYXCOM_IUnknown* aClassObject[] = {
#define PYXCOM_CLASS_OBJECT_TABLE_ENTRY(T) \
	static_cast<PYXCOM_IClassFactory*>(new ClassObjectImpl<T>)
#define PYXCOM_END_CLASS_OBJECT_TABLE \
	}; \
	PYXCOM_CLASS_OBJECT_MANAGER \
	} \
	PYXCOM_GCO_FUNCTION(PYXCOMDllGetClassObject)

#define PYXCOM_END_CLASS_OBJECT_TABLE_EXPORT_AS(GCO_NAME) \
	}; \
	PYXCOM_CLASS_OBJECT_MANAGER \
	} \
	PYXCOM_GCO_FUNCTION(GCO_NAME)

// 1) Run guidgen.exe (also available from Visual Studio as Tools -> Create GUID)
// 2) Choose GUID format 2 (DEFINE_GUID)
// 3) Click New GUID then Copy
// 4) Paste into your source file
// 5) Change DEFINE_GUID to PYXCOM_DEFINE_IID or PYXCOM_DEFINE_CLSID
// 6) Enter your interface or class name

#define PYXCOM_DEFINE_IID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const IID name::iid = \
	{ l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define PYXCOM_DEFINE_CLSID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const CLSID name::clsid = \
	{ l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

// Use this variadic macro to define your class interfaces.
#define PYXCOM_CLASS_INTERFACES(name, ...) \
	const IID name::aiid[] = { __VA_ARGS__ }; \
	const int name::niid = sizeof(name::aiid)/sizeof(name::aiid[0])

////////////////////////////////////////////////////////////////////////////////

//! PYXCOM interface for supporting introspection.
struct PYXLIB_DECL IClassInfo : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getClass(const CLSID** ppclsid) = 0;
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getNumInterfaces(int* pNumInterfaces) = 0;
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getInterfaces(const IID** ppiid) = 0;
};

////////////////////////////////////////////////////////////////////////////////

//! Helper class for implementing class objects.
template <typename T>
class ClassObjectImpl : public PYXCOM_IClassFactory, public IClassInfo
{
public:

#if 0 // can't trace when trace facility already uninitialized
	~ClassObjectImpl()
	{
		TRACE_INFO("destroying ClassObjectImpl for " << T::clsid);
	}
#endif

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IClassFactory)
		IUNKNOWN_QI_CASE(IClassInfo)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // PYXCOM_IClassFactory

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE CreateInstance(PYXCOM_IUnknown* pUnkOuter, REFIID riid, void** ppObject)
	{
		// See Essential COM p117
		assert(ppObject != 0);
		*ppObject = 0;

		if (pUnkOuter != 0)
		{
			return PYXCOM_CLASS_E_NOAGGREGATION;
		}

#if !defined(NDEBUG) && defined(_WINDOWS)
		T* pT = new(_NORMAL_BLOCK, __FILE__, __LINE__) T;		
#else
		T* pT = new T;
#endif
		if (pT == 0)
		{
			return PYXCOM_OUTOFMEMORY;
		}

		pT->AddRef();
		PYXCOM_HRESULT hr = pT->QueryInterface(riid, ppObject);
		pT->Release();
		return hr;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE LockServer(PYXCOM_BOOL bReserved) { return PYXCOM_NOTIMPL; }

public: // IClassInfo

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getClass(const CLSID** ppclsid)
	{
		assert(ppclsid != 0);
		*ppclsid = &T::clsid;
		return PYXCOM_OK;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getNumInterfaces(int* pNumInterfaces)
	{
		assert(pNumInterfaces != 0);
		*pNumInterfaces = T::niid;
		return PYXCOM_OK;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE getInterfaces(const IID** ppiid)
	{
		assert(ppiid != 0);
		*ppiid = T::aiid;
		return PYXCOM_OK;
	}
};


////////////////////////////////////////////////////////////////////////////////

//! PYXCOM interface for enumerating class objects.
struct PYXLIB_DECL IEnumClassObject : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Next(PYXCOM_ULONG nElem, PYXCOM_IUnknown** ppObject, PYXCOM_ULONG* pnFetched) = 0;
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Skip(PYXCOM_ULONG nElem) = 0;
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Reset() = 0;
	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Clone(IEnumClassObject** ppObject) = 0;
};

////////////////////////////////////////////////////////////////////////////////

//! Helper class for implementing class object enumerator.
class PYXLIB_DECL EnumClassObjectImpl : public IEnumClassObject
{

public:

	EnumClassObjectImpl(PYXCOM_IUnknown** ppObject, PYXCOM_ULONG nCount) :
		m_ppObject(ppObject),
		m_nTotal(nCount),
		m_nCurrent(0)
	{
		assert(0 < nCount);
		assert(ppObject != 0);
	}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_IMPL(IEnumClassObject);

	IUNKNOWN_RC_IMPL();

public: // IEnumClassObject

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Next(PYXCOM_ULONG nCount, PYXCOM_IUnknown** ppObject, PYXCOM_ULONG* pnFetched)
	{
		assert(ppObject != 0);

		if (1 < nCount && pnFetched == 0)
		{
			return PYXCOM_INVALIDARG;
		}

		PYXCOM_ULONG nFetched = 0;
		while (nFetched < nCount && m_nCurrent < m_nTotal)
		{
			ppObject[nFetched] = m_ppObject[m_nCurrent];
			ppObject[nFetched]->AddRef();
			++nFetched;
			++m_nCurrent;
		}

		if (pnFetched != 0)
		{
			*pnFetched = nFetched;
		}

		return nFetched == nCount ? PYXCOM_OK : PYXCOM_FALSE;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Skip(PYXCOM_ULONG nCount)
	{
		m_nCurrent += nCount;

		if (m_nTotal < m_nCurrent)
		{
			m_nCurrent = m_nTotal;
			return PYXCOM_FALSE;
		}

		return PYXCOM_OK;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Reset()
	{
		m_nCurrent = 0;
		return PYXCOM_OK;
	}

	virtual PYXCOM_HRESULT STDMETHODCALLTYPE Clone(IEnumClassObject** ppObject)
	{
		assert(ppObject != 0);

		*ppObject = new EnumClassObjectImpl(*this);
		if (*ppObject == 0)
		{
			return PYXCOM_OUTOFMEMORY;
		}

		(*ppObject)->AddRef();
		return PYXCOM_OK;
	}

private:

	PYXCOM_IUnknown** m_ppObject;
	PYXCOM_ULONG m_nTotal;
	PYXCOM_ULONG m_nCurrent;
};


////////////////////////////////////////////////////////////////////////////////

//! Less operator for comparing GUIDs.
inline bool operator <(REFGUID rguid1, REFGUID rguid2)
{
	return memcmp(&rguid1, &rguid2, sizeof(GUID)) < 0;
}

////////////////////////////////////////////////////////////////////////////////

//! Helper function for boost::intrusive_ptr which adds a reference to an object.
inline void intrusive_ptr_add_ref(const PYXCOM_IUnknown* p)
{
	// NOTE this object had better not actually be const.
	const_cast<PYXCOM_IUnknown*>(p)->AddRef();
}

//! Helper function for boost::intrusive_ptr which releases an object.
inline void intrusive_ptr_release(const PYXCOM_IUnknown* p)
{
	// NOTE this object had better not actually be const.
	const_cast<PYXCOM_IUnknown*>(p)->Release();
}

////////////////////////////////////////////////////////////////////////////////

//! Type of GetClassObject function.
typedef PYXCOM_HRESULT (*LPFNGCO)(REFCLSID, REFIID, void**);

//! Initialize the PYXCOM facility.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMInitialize();

//! Uninitialize the PYXCOM facility.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMUninitialize();

//! Register a GetClassObject function.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMRegister(LPFNGCO pfnGCO);

//! Create a globally unique identifier.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMCreateGuid(GUID* pguid);

//! Get a class object.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMGetClassObject(REFCLSID rclsid, REFIID riid, void** ppObject);

//! Create an instance.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMCreateInstance(REFCLSID rclsid, PYXCOM_IUnknown* pUnkOuter, REFIID riid, void** ppObject);

//! Get class IDs for an interface ID.
PYXLIB_DECL PYXCOM_HRESULT PYXCOMGetClassIDs(REFIID riid, std::vector<CLSID>* pVecClsid);

//! Returns whether a class implements an interface.
PYXLIB_DECL bool PYXCOMImplementsInterface(REFCLSID rclsid, REFIID riid);

//! Output a GUID in canonical form.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, REFGUID guid);

//! Input a GUID in canonical form.
PYXLIB_DECL std::istream& operator >>(std::istream& in, GUID& guid);

////! Output a GUID in canonical form.
//PYXLIB_DECL std::basic_ostream<unsigned char>& operator <<(std::basic_ostream<unsigned char>& out, REFGUID guid);
//
////! Input a GUID in canonical form.
//PYXLIB_DECL std::basic_istream<unsigned char>& operator >>(std::basic_istream<unsigned char>& in, GUID& guid);

//! Convert a GUID to a (canonical) string.
PYXLIB_DECL 
inline
std::string guidToStr(REFGUID guid)
{
	std::ostringstream out;
	out << guid;
	return out.str();
}

//! Convert a (canonical) string to a GUID.
PYXLIB_DECL 
inline
GUID strToGuid(const std::string& strGuid)
{
	std::istringstream in(strGuid);
	GUID guid;
	in >> guid;
	return guid;
}

#ifdef SWIG
PYXLIB_DECL _GUID::_GUID( const char *strGuid)
{
	GUID newValue = strToGuid( strGuid);
	memcpy( this, &newValue, sizeof( GUID));
}
#endif
////////////////////////////////////////////////////////////////////////////////

//Helper functions to create PYXCOM objects

template<class Interface>
boost::intrusive_ptr<Interface> PYXCOMCreateInstance(REFCLSID rclsid)
{
	Interface * object = NULL;
	 
	PYXCOM_HRESULT hr = PYXCOMCreateInstance(rclsid,0,Interface::iid,(void**) &object);
	if (PYXCOM_FAILED(hr))
	{
		TRACE_ERROR("Couldn't instantiate " << rclsid);		
		return boost::intrusive_ptr<Interface>();
	} 
	else 
	{
		//create a smart pointer but dont do AddRef because PYXCOMCreateInstance allready did that for us
		return boost::intrusive_ptr<Interface>(object,false);
	}	
}

template<class Interface,class Class>
boost::intrusive_ptr<Interface> PYXCOMCreateInstance()
{
	return PYXCOMCreateInstance<Interface>(Class::clsid);
}

namespace PYXCOM
{
	/*!
	PYXCOM Dependency Injection

	this class enable general functionaly of DI using PYXCOM.
	you can simply get the default implementation of a interface, or get all implementations.
	Moreover, you can set the default implementation for a given interface as well.

	*/
	class PYXLIB_DECL DI
	{
	private:
		static std::map<IID, boost::intrusive_ptr<PYXCOM_IUnknown> > s_defaultObjects;
		static std::map<IID, CLSID> s_defaultClasses;

	public:		
		//get the default implementation of an interface 
		static boost::intrusive_ptr<PYXCOM_IUnknown> get(REFIID iid);

		//get an implementation of an interface using a specific class
		static boost::intrusive_ptr<PYXCOM_IUnknown> get(REFIID iid,REFCLSID clsid);

		//get all implementations of a single interface
		static std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> > getAll(REFIID iid);

		//set the default class for an interface
		static void set(REFIID iid,REFCLSID clsid);
	
		//set a default object for an interface
		static void set(REFIID iid,const boost::intrusive_ptr<PYXCOM_IUnknown> & item);	

	public:
		template<class Interface>		
		static boost::intrusive_ptr<Interface> get()
		{
			auto & unknown = get(Interface::iid);
			if (unknown)
			{
				return unknown->QueryInterface<Interface>();
			}
			return NULL;
		};

		template<class Interface>		
		static std::vector< boost::intrusive_ptr<Interface> > getAll()
		{
			std::vector< boost::intrusive_ptr<Interface> > results;
			for(auto & item : getAll(Interface::iid))
			{
				auto & converted = item->QueryInterface<Interface>();
				if (converted)
				{
					results.push_back(converted);
				}
			}
			return results;
		};

		template<class Interface>		
		static void set(boost::intrusive_ptr<Interface> impl)
		{
			if (!impl)
			{
				return;
			}
			set(Interface::iid,impl->QueryInterface<PYXCOM_IUnknown>());
		};
	};
};

#endif // guard
