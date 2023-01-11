/******************************************************************************
checksum_calculator.cpp

begin		: 2008-09-14
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"

#include "pyxis/utility/checksum_calculator.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

static PYXPointer<ChecksumCalculator> getChecksumCalculator();

PYXPointer<ChecksumCalculator> ChecksumCalculator::m_pCalculator;

PYXPointer<ChecksumCalculator> ChecksumCalculator::getChecksumCalculator()
{
	return m_pCalculator;
}

void ChecksumCalculator::setChecksumCalculator( PYXPointer<ChecksumCalculator> spCalculator)
{
	m_pCalculator = spCalculator;
}

std::string ChecksumCalculator::calculateCheckSum(const std::string& filePath)
{
	PYXTHROW(PYXException, "Invalid operation: must initialize with setChecksumCalculator()");
	return "";
}

std::string ChecksumCalculator::calculateFileCheckSum(const std::string& binaryFileData)
{
	PYXTHROW(PYXException, "Invalid operation: must initialize with setChecksumCalculator()");
	return "";
}
    
std::string ChecksumCalculator::findFileMatchingChecksum(const std::string &checksum)
{
	PYXTHROW(PYXException, "Invalid operation: must initialize with setChecksumCalculator()");
	return "";
}
