#ifndef PYXIS__UTILITY__CACHE_STATUS_H
#define PYXIS__UTILITY__CACHE_STATUS_H
/******************************************************************************
cache_status.h

begin		: 2005-12-02
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <ctime>

/*!
This class is used by other classes to keep track of creation, access and other
time related information.
*/
//! Keeps track of time information.
class PYXLIB_DECL CacheStatus
{
public:

	//! The state of the timer object
	enum eTimeState
	{
		knUninitialized = 0,	//!< The object has never been initialized
		knValid,				//!< A the time is valid for the tile.
		knExpired				//!< he time has expired on the object.
	};

	//! Default constructor.
	CacheStatus();

	//! Destructor.
	virtual ~CacheStatus();

	/*!
	Return the time at object creation and initialization.  If the object is 
	not initialized the return value is 0.

	\return The accessed time.
	*/
	//! Return the time the object was created and initialized.
	std::time_t getCreateTime() const {return m_nCreateTime;}

	/*!
	Return the last accessed time.  If the object is not initialized the return
	value is 0.

	\return The accessed time.
	*/
	//! Return the last accessed time
	std::time_t getAccessedTime() const {return m_nAccessTime;}

	//! Initialize the timer to be in a valid state with the current time.
	void initialize();

	/*!
	This method only sets the accessed time to the current time.  The method
	is not responsible for verifying the state of the object.
	*/
	//! Set the accessed time of the tile to the current time.
	void setAccessed() {std::time(&m_nAccessTime);}

	//! Determine if the object has been accessed since a given time stamp.
	bool accessedSince(std::time_t nTime) const {return nTime < m_nAccessTime;}

	//! Determine if the time has expired on the object.
	bool hasExpired() const {return m_nState == knExpired;}

	//! Set the state of the object.
	void setState(eTimeState nState);

protected:

	//! Disable copy constructor
	CacheStatus(const CacheStatus&);

	//! Disable copy assignment
	void operator =(const CacheStatus&);

private:

	//! The current time state of the object.
	eTimeState m_nState;

	//! The time the object was initialized.
	std::time_t m_nCreateTime;

	//! The time the object was last accessed.
	std::time_t m_nAccessTime;
};

#endif // guard
