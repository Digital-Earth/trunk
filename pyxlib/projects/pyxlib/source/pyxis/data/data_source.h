#ifndef PYXIS__DATA_SOURCE__DATA_SOURCE_H
#define PYXIS__DATA_SOURCE__DATA_SOURCE_H
/******************************************************************************
data_source.h

begin		: 2004-10-19
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature.h"
#include "pyxis/utility/notifier.h"

// forward declarations
class PYXGeometry;

/*!
PYXDataSource is the abstract base for classes that represent a data source.
A data source contains spatial data (features or coverages). Often the data
source corresponds to a layer.
*/
//! Abstract base for classes that represent a data source.
class PYXLIB_DECL PYXDataSource :	public PYXFeature
#ifndef SWIG
// TODO deal with the fact that C# doesn't have multiple inheritance
									,
									public Observer,
									public Notifier
#endif
{
public:

	// Constants
	static const std::string kstrScope;

	//! The various GIS data source types.
	enum eType
	{
		knTypeUnknown = 0,
		knDEM,
		knRaster,
		knVector
	};

	// Field names.
	static const std::string kstrDataSourceName;
	static const std::string kstrDataSourceType;
	
	//! Destructor
	virtual ~PYXDataSource() {}

	//!	Get the ID of the feature.
	const std::string& getID() const;

	//! Convenience method for getting the data source name.
	std::string getName() const;

	//! Convenience method for getting the data source type.
	eType getType() const;

	//! Convenience method for getting the resolution for the data source.
	int getResolution() const;

	/*!
	Call this method after the data source is opened to determine if the data
	source has a resolution. If not, a resolution must be supplied by calling
	setResolution() before data is accessed. PYXIS data sources always return
	true after their geometry is set.
	
	\return	true if the data source has a resolution, otherwise false
	*/
	virtual bool hasResolution() const {return getResolution() >= 0;}

	/*!
	Specify the resolution for the data source. Call this method to set the
	resolution if after the data source is opened hasResolution() returns
	false. PYXIS data sources always have a resolution after the geometry is
	set.

	\param	nRes	The resolution.
	*/
	virtual void setResolution(int nRes)
	{
		assert(false);
	}

	/*!
	Call this method on a data source that can supply data at multiple resolutions.

	Call the function to set the resolution subsequent data is to be returned at.

	The function can be called any number of times and the data source is to adjust
	the resolution of any internal data returned to reflect the change requested.
	
	Empty implementation except for data sources (such as WMS that downloads data
	at various resolutions) that require the functionality.

	\param	nResolution	Pyxis resolution of data to be returned by the data source.
	*/
	virtual void setRequestedDataResolution(int nResolution) {}

	//! Get a field value by index.
	virtual PYXValue getFieldValue(int nFieldIndex) const;

	//! Return the class name of this observer class.
	virtual std::string getObserverDescription() const 
	{
		return kstrScope + " " + getName();
	}
	
	//! Detaches from all notifiers. Does nothing.
	virtual void detachFromNotifiers() {}

	
	/*!
	For data sources that can have multiple resolutions of data (WMS for now)
	give the minumum available resolution we can return.

	\return minimum resolution this data source can return. -1 if not implemented.
	*/
	virtual int getMinimumAvailableResolution() { return -1; };

	/*!
	For data sources that can have multiple resolutions of data (WMS for now)
	give the maximum available resolution we can return.

	\return maximum resolution this data source can return. -1 if not implemented.
	*/
	virtual int getMaximumAvailableResolution() { return -1; };

protected:

	//! Constructor
	PYXDataSource();

	//! Handle notification events. Default implementation does nothing.
	virtual void updateObserverImpl(PYXPointer<NotifierEvent> spEvent) {}

	//! Set the field definitions for this data source
	virtual void setDefinition(PYXPointer<PYXTableDefinition> spDefn);

	//! Convenience method for setting the data source name.
	virtual void setName(const std::string& strName);

	//! Convenience method for setting the data source type.
	virtual void setType(PYXDataSource::eType nType);

	//! Set a field value by index.
	void setFieldValue(PYXValue value, int nFieldIndex);

	//! Update fields based on an input data source
	void updateFields(const PYXDataSource& ds);

protected:

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;
	
private:

	//! Disable copy constructor
	PYXDataSource(const PYXDataSource&);

	//! Disable copy assignment
	void operator=(const PYXDataSource&);

	//! Add required fields to the data source definition.
	void addRequiredDataSourceFields();

	//! Vector of values that describe the data source.
	std::vector<PYXValue> m_vecValues;
};

#endif // guard
