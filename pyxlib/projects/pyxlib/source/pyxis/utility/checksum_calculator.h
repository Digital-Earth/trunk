#ifndef CHECKSUM_CALCULATOR_H
#define CHECKSUM_CALCULATOR_H
/******************************************************************************
checksum_calculator.h

begin		: 2008-09-14
copyright	: (C) 200by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include <string>

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/object.h"

class PYXLIB_DECL ChecksumCalculator : public PYXObject
{
public:
	//! Caluclates a checksum for the data represented by string. 
	virtual std::string calculateCheckSum(const std::string& str);

	//! Calculate a checksum for the file path specified.
	virtual std::string calculateFileCheckSum(const std::string& path);

    //! Finds the file matching the given checksum.
	virtual std::string findFileMatchingChecksum(const std::string &checksum);

	static PYXPointer<ChecksumCalculator> getChecksumCalculator();

	static void setChecksumCalculator(PYXPointer<ChecksumCalculator> spCalculator);

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~ChecksumCalculator()
	{}

private:

	//! Pointer to the call back function.
	static PYXPointer<ChecksumCalculator> m_pCalculator;

};

#endif // guard
