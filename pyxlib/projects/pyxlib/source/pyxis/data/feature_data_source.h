#ifndef PYXIS__DATA_SOURCE__FEATURE_DATA_SOURCE_H
#define PYXIS__DATA_SOURCE__FEATURE_DATA_SOURCE_H
/******************************************************************************
feature_data_source.h

begin		: 2004-10-19
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/data_source.h"

// boost declarations

// forward declarations
class PYXFeatureIterator;
class PYXSpatialReferenceSystem;
class StyleMapper;

/*!
PYXFeatureDataSource is a data source that contains multiple features. It
provides the ability to iterate over the features that intersect a given
geometry.

PYXFeatureDataSource provides a Boost recursive_mutex to serialize concurrent
accesses by multiple threads, though as a pure-virtual class it does not
actually use it.  Every virtual function implementation which represents a
critical section of code (which should not be interruptible) should start with
the line "boost::recursive_mutex::scoped_lock lock(m_mutex);".  The recursive_mutex,
which allows a given thread to lock/unlock multiple times, is used so that
virtual functions implemented in derived classes may call implementations in
parent classes without having to first unlock the mutex.
*/
//! A data source that contains multiple features.
class PYXLIB_DECL PYXFeatureDataSource : public PYXDataSource
{
public:

	//! Destructor
	virtual ~PYXFeatureDataSource() {}

	/*!
	Call this method after the data source is opened to determine if the data
	source contains a spatial reference. If not, a spatial reference must be
	supplied by calling setSpatialReference() before getFeatureIterator() is
	called.
	
	\return	true if the data source has a spatial reference, otherwise false
	*/
	virtual bool hasSpatialReferenceSystem() const;

	/*!
	Specify the spatial reference for the data source. Call this method to set
	the spatial reference if after the data source is opened
	hasSpatialReference() returns false.

	\param	spSRS	The spatial reference system.
	*/
	virtual void setSpatialReferenceSystem(
		PYXPointer<PYXSpatialReferenceSystem> spSRS	);

	//! Get the field definitions for the features in this data source
	PYXPointer<PYXTableDefinition> getFeatureDefinition() const {return m_spFeatureDefn;}

	/*!
	Get an iterator to all features in the data source.

	\return	The iterator.
	*/
	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator() const = 0;

	/*!
	Get an iterator to the features in the data source that meet the specified
	spatial qualification.

	\param	geometry	The spatial qualification.

	\return	The iterator.
	*/
	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(
		const PYXGeometry& geometry	) const = 0;

	/*!
	Get an iterator to the features in the data source that meet the specified
	attribute qualification.

	\param	strWhere	The attribute qualification.

	\return	The iterator.
	*/
	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(
		const std::string& strWhere	) const = 0;

	/*!
	Get an iterator to the features in the data source that meet the specified
	spatial and attribute qualification.

	\param	geometry	The spatial qualification.
	\param	strWhere	The attribute qualification.

	\return	The iterator.
	*/
	//! Get an iterator to the features.
	virtual PYXPointer<PYXFeatureIterator> getFeatureIterator(
		const PYXGeometry& geometry,
		const std::string& strWhere	) const = 0;

	/*!
	Get a specific feature based on its ID.  Does not use any spatial qualification
	or filters.

	\param	strFeatureID	ID of the feature to search for.

	\return Shared pointer to the feature or empty shared pointer if not found.
	*/
	//! Get a specific feature based on its ID.
	virtual PYXPointer<const PYXFeature> getFeature(const std::string& strFeatureID) const = 0;

	/*!
	Create a new feature given a feature ID. Default implementation does nothing.
	Only those data sources that are persistable need implement this method.

	\param	strFeatureID	ID of the feature to create.
	*/
	//! Create a new feature given a feature ID.
	virtual void createFeature(const std::string& strFeatureID) const {}
	
	/*!
	Flush the current in memory features.  Removes them from memory and persists them.
	Default implementation does nothing. Only those data sources that are persistable need implement
	this method.
	*/
	//! Flush the current in memory features.
	virtual void flushFeatures() {}

	/*!
	Determine the currently loaded mapper for the data source and return it 
	to the caller.

	\return A smart pointer to the style mapper for the feature data source.
	*/
	//! Get the style mapper for the data source
	virtual PYXPointer<StyleMapper> getStyleMapper() = 0;

protected:

	//! Constructor
	PYXFeatureDataSource() :
		 m_spFeatureDefn(PYXTableDefinition::create())
	{
		// set the data source type to vector
		setType(knVector);
	}

	//! Set the field definitions for the features in this data source
	void setFeatureDefinition(PYXPointer<PYXTableDefinition> spDefn)
	{
		m_spFeatureDefn = spDefn;
	}

private:

	//! Disable copy constructor
	PYXFeatureDataSource(const PYXFeatureDataSource&);

	//! Disable copy assignment
	void operator=(const PYXFeatureDataSource&);

private:

	//! Describes the fields of the features in this data source
	mutable PYXPointer<PYXTableDefinition> m_spFeatureDefn;
};

#endif // guard
