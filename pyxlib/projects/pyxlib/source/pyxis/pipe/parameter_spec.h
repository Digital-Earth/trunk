#ifndef PYXIS__PIPE__PARAMETER_SPEC_H
#define PYXIS__PIPE__PARAMETER_SPEC_H
/******************************************************************************
parameter_spec.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"

// standard includes
#include <cassert>
#include <string>

/*!
*/
//! A specification for a parameter for a process.
class PYXLIB_DECL ParameterSpec : public PYXObject
{
public:
	//! Create method
	static PYXPointer<ParameterSpec> create(
		REFIID riid,
		int nMinOccurs,
		int nMaxOccurs,
		const std::string& strName,
		const std::string& strDescription	)
	{
		return PYXNEW(ParameterSpec,
			riid,
			nMinOccurs,
			nMaxOccurs,
			strName,
			strDescription	);
	}

private:

	ParameterSpec(
		REFIID riid,
		int nMinOccurs,
		int nMaxOccurs,
		const std::string& strName,
		const std::string& strDescription	)
	:
		m_iid(riid),
		m_nMinOccurs(nMinOccurs),
		m_nMaxOccurs(nMaxOccurs),
		m_strName(strName),
		m_strDescription(strDescription)
	{
		assert(0 <= nMinOccurs);
		assert(-1 <= nMaxOccurs);
	}

public:

	REFIID getInterface() const
	{
		return m_iid;
	}

	int getMinOccurs() const
	{
		return m_nMinOccurs;
	}

	int getMaxOccurs() const
	{
		return m_nMaxOccurs;
	}

	const std::string& getName() const
	{
		return m_strName;
	}

	const std::string& getDescription() const
	{
		return m_strDescription;
	}

private:

	IID m_iid;
	int m_nMinOccurs;
	int m_nMaxOccurs;
	std::string m_strName;
	std::string m_strDescription;
};

#endif // guard
