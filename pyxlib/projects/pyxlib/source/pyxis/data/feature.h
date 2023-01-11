#ifndef PYXIS__DATA__FEATURE_H
#define PYXIS__DATA__FEATURE_H
/******************************************************************************
feature.h

begin		: 2004-10-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/record.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/region/region.h"
#include "feature_style.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <string>

/*!
A simple feature with geometry and fields.
*/
//! A feature.
struct PYXLIB_DECL IFeature : public IRecord
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual bool STDMETHODCALLTYPE isWritable() const = 0;

	virtual const std::string& STDMETHODCALLTYPE getID() const = 0;

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry() = 0;

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const = 0;

	/*!
	The style is in the following format: 
	<style>
		<LineColour>value</LineColour>
		<Icon>value</Icon>
		<onClick>value</onClick>
	</style>	
	*/
	virtual std::string STDMETHODCALLTYPE getStyle() const = 0;	

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const = 0;	
};

#define IFEATURE_IMPL() \
protected: \
	PYXPointer<PYXGeometry> m_spGeom; /* TODO: Remove this; deprecated. */ \
	bool m_bWritable; \
	mutable std::string m_strID; \
	mutable std::string m_strStyle; \
public: \
	virtual bool STDMETHODCALLTYPE isWritable() const \
	{ \
		return m_bWritable; \
	} \
	virtual const std::string& STDMETHODCALLTYPE getID() const \
	{ \
		if (m_strID == "") m_strID = "Uninitialized PYXIS Feature ID"; \
		return m_strID; \
	} \
	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const \
	{ \
		return m_spGeom; \
	} \
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry() \
	{ \
		return m_spGeom; \
	} \
	virtual std::string STDMETHODCALLTYPE getStyle() const \
	{ \
		return m_strStyle; \
	} \
	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const \
	{ \
		if (m_strStyle.size() == 0) \
		{ \
			return ""; \
		} \
		PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(m_strStyle); \
		return styleDoc->getNodeText("/style/" + strStyleToGet); \
	}

#define IFEATURE_IMPL_PROXY(proxy) \
public: \
	virtual bool STDMETHODCALLTYPE isWritable() const \
	{ \
		return (proxy).isWritable(); \
	} \
	virtual const std::string& STDMETHODCALLTYPE getID() const \
	{ \
		return (proxy).getID(); \
	} \
	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const \
	{ \
		return (proxy).getGeometry(); \
	} \
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry() \
	{ \
		return (proxy).getGeometry(); \
	} \
	virtual std::string STDMETHODCALLTYPE getStyle() const \
	{ \
		return (proxy).getStyle(); \
	} \
	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const \
	{ \
		return (proxy).getStyle(strStyleToGet); \
	}


/*!
*/
//! Utilities for encoding and decoding feature IDs into a string.
class PYXLIB_DECL FIDStr
{
public:

	static void test();

public:

	//! Encodes feature IDs into a string.
	static void encode(std::string* pStr, const std::vector<std::string>& vecFID);

	//! Decodes feature IDs from a string.
	static void decode(const std::string* pStr, std::vector<std::string>* pVecFID);

	//! Adds a feature ID to an encoded string.
	static bool add(std::string* pStr, const std::string& strFID);

	//! Adds a feature ID to an encoded string.
	static void uncheckedAdd(std::string* pStr, const std::string& strFID);

	//! Removes a feature ID from an encoded string.
	static bool remove(std::string* pStr, const std::string& strFID);

	//! Returns whether an encoded string contains a feature ID.
	static bool contains(const std::string* pStr, const std::string& strFID);

	//! Returns the number of feature IDs in an encoded string.
	static int count(const std::string* pStr);
};

#endif // guard
