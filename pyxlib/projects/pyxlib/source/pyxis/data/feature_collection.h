#ifndef PYXIS__DATA__FEATURE_COLLECTION_H
#define PYXIS__DATA__FEATURE_COLLECTION_H
/******************************************************************************
feature_collection.h

begin		: 2007-02-28
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature.h"
#include "pyxis/utility/object.h"

// standard includes

/*!
*/
//! A feature iterator.
class PYXLIB_DECL FeatureIterator : public PYXObject
{
public:

	virtual bool end() const = 0;

	virtual void next() = 0;

	virtual boost::intrusive_ptr<IFeature> getFeature() const = 0;
};

/*!
Delegates to STL-style iterators.
*/
//! Templated helper class for feature iterators.
template <typename IT>
class DefaultFeatureIterator : public FeatureIterator
{
public:

	DefaultFeatureIterator(IT it, IT itEnd) :
		m_it(it),
		m_itEnd(itEnd)
	{
	}

	template <typename IT>
	static PYXPointer<DefaultFeatureIterator<IT> > create(IT it, IT itEnd)
	{
		return PYXNEW(DefaultFeatureIterator<IT>, it, itEnd);
	}

public:

	virtual bool end() const
	{
		return m_it == m_itEnd;
	}

	virtual void next()
	{
		++m_it;
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return *m_it;
	}

private:

	IT m_it;
	IT m_itEnd;
};

//! Templated helper function.
template <typename IT>
inline
PYXPointer<DefaultFeatureIterator<IT> > createDefaultFeatureIterator(IT it, IT itEnd)
{
	return DefaultFeatureIterator<IT>::create(it, itEnd);
}

/*!
Delegates to STL-style iterators.
*/
//! Templated helper class for an empty feature iterator.
template <typename IT>
class EmptyFeatureIterator : public FeatureIterator
{
public:

	EmptyFeatureIterator(IT it, IT itEnd) :
		m_it(it),
		m_itEnd(itEnd)
	{
	}

	template <typename IT>
	static PYXPointer<EmptyFeatureIterator<IT> > create(IT it, IT itEnd)
	{
		return PYXNEW(EmptyFeatureIterator<IT>, it, itEnd);
	}

public:

	virtual bool end() const
	{
		return true;
	}

	virtual void next()
	{		
	}

	virtual boost::intrusive_ptr<IFeature> getFeature() const
	{
		return boost::intrusive_ptr<IFeature>();
	}

private:

	IT m_it;
	IT m_itEnd;
};

//! Templated helper function.
template <typename IT>
inline
PYXPointer<EmptyFeatureIterator<IT> > createEmptyFeatureIterator(IT it, IT itEnd)
{
	return EmptyFeatureIterator<IT>::create(it, itEnd);
}

/*!
A collection of features which is itself a feature.
*/
//! A feature collection.
struct PYXLIB_DECL IFeatureCollection : public IFeature
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const = 0;

	// TODO: Remove; deprecated.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const = 0;

	//! Get the field definitions for the features in this data source
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const = 0;

	//! Get the field definitions for the features in this data source
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() = 0;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const = 0;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const = 0;	

	virtual bool STDMETHODCALLTYPE canRasterize() const = 0;

	/*!
	Send a hint to the feature collection that we are going to be actively reading 
	from the passed geometry.  The usual reaction to this call would be for the data
	set to cache as much of the data ahead of time if it makes sense, and would improve
	speed of access.

	\param	spGeom	Shared pointer to the geometry we are interested in
	*/
	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom) = 0;

	/*!
	Signal that the client is done reading from the passed geometry.  This indicates
	that any cached data could be flushed to free up memory resources.

	\param	spGeom	Shared pointer to the geometry we are interested in
	*/
	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom) = 0;
};

#define IFEATURECOLLECTION_IMPL() \
	IFEATURECOLLECTION_IMPL_WITHOUT_HINTS(); \
	IFEATURECOLLECTION_IMPL_HINTS();

#define IFEATURECOLLECTION_IMPL_WITHOUT_HINTS() \
protected: \
	std::vector<FeatureStyle> m_vecStyles; \
public: \
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const \
	{ \
		assert(false && "TODO not yet implemented"); \
		return PYXPointer<FeatureIterator>(); \
	} \
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const /* TODO: Remove; deprecated. */ \
	{ \
		assert(false && "TODO not yet implemented"); \
		return PYXPointer<FeatureIterator>(); \
	} \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const \
	{ \
		return PYXPointer<PYXTableDefinition>(); \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() \
	{ \
		return PYXPointer<PYXTableDefinition>(); \
	} \
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const \
	{ \
		return boost::intrusive_ptr<IFeature>(); \
	} \
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const \
	{ \
		return m_vecStyles; \
	} \
	virtual bool STDMETHODCALLTYPE canRasterize() const \
	{\
		return true;\
	}

#define IFEATURECOLLECTION_IMPL_HINTS() \
	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom) {} \
	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom) {}

#define IFEATURECOLLECTION_IMPL_PROXY(proxy) \
public: \
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const \
	{ \
		return (proxy).getIterator(); \
	} \
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const /* TODO: Remove; deprecated. */ \
	{ \
		return (proxy).getIterator(geometry); \
	} \
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const \
	{ \
		return (proxy).getFeatureDefinition(); \
	} \
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() \
	{ \
		return (proxy).getFeatureDefinition(); \
	} \
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const \
	{ \
		return (proxy).getFeature(strFeatureID); \
	} \
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const \
	{ \
		return (proxy).getFeatureStyles(); \
	} \
	virtual bool STDMETHODCALLTYPE canRasterize() const \
	{ \
		return (proxy).canRasterize(); \
	} \
	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom) \
	{ \
		return (proxy).geometryHint(spGeom); \
	} \
	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom) \
	{ \
		return (proxy).endGeometryHint(spGeom); \
	} \


#endif // guard
