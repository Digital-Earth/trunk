#ifndef PYXILIZATION_ALPHA_DRIVER_H
#define PYXILIZATION_ALPHA_DRIVER_H
/******************************************************************************
pyxilization_driver.h

begin		: 2007-26-01
copyright	: (C) 2007 by Stephen Scovil, Nick Lipson, Sopheap Hok, Dale Offord
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyx_driver.h"	// PYXDriver

// standard includes


//! Stub driver for Pyxilization game
/*!
This class essentially looks for someone trying to start the game, and loads data source if so
*/
class PYXILIZATIONAlphaDriver : public PYXDriver
{
public:

	//! Test method
	static void test();

	//! Constructor.
	PYXILIZATIONAlphaDriver();

	//! Destructor
	virtual ~PYXILIZATIONAlphaDriver();

	//! Get the name of the driver.
	virtual const std::string& getName() const;

	//! Open a data source.
	virtual boost::shared_ptr<PYXDataSource> openForRead(const std::string& strDataSourceName) const;

	//! Fill supplied FileInfo structure with information about the specific file.
	bool getFileInfo(std::string& strURI, PYXDriver::FileInfo* pFileInfo, bool * pbExit);

protected:

private:

	//! Disable copy constructor
	PYXILIZATIONAlphaDriver(const PYXILIZATIONAlphaDriver&);

	//! Disable copy assignment
	void operator=(const PYXILIZATIONAlphaDriver&);
};

#endif
