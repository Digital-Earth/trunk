#ifndef PYXIS__PIPE__PROCESS_SPEC_H
#define PYXIS__PIPE__PROCESS_SPEC_H
/******************************************************************************
process_spec.h

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/pipe/parameter_spec.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"

// standard includes
#include <algorithm>
#include <iosfwd>
#include <vector>

//! A specification for a process.
class PYXLIB_DECL ProcessSpec : public PYXObject
{
public:

	//! Create method
	static PYXPointer<ProcessSpec> create(
		REFCLSID rclsid,
		const std::vector<IID>& vecOutputIIDs,
		const std::vector<PYXPointer<ParameterSpec> >& vecParamSpec,
		const std::string& strName,
		const std::string& strDescription	)
	{
		return PYXNEW(ProcessSpec,
			rclsid,
			vecOutputIIDs,
			vecParamSpec,
			strName,
			strDescription	);
	}

	static PYXPointer<ProcessSpec> create(
		const std::string& xmlDefinition)
	{
		return PYXNEW(ProcessSpec, xmlDefinition);
	}

private:

	ProcessSpec(
		REFCLSID rclsid,
		const std::vector<IID>& vecOutputIIDs,
		const std::vector<PYXPointer<ParameterSpec> >& vecParamSpec,
		const std::string& strName,
		const std::string& strDescription	)
	:
		m_clsid(rclsid),
		m_vecOutputIID(vecOutputIIDs),
		m_vecParamSpec(vecParamSpec),
		m_strName(strName),
		m_strDescription(strDescription)
	{
		// TODO: reasonable assertions
	}

	explicit ProcessSpec(const std::string& xmlDefinition)
	{
		// Using 'this' is safe here because there are no virtual method calls.
		read(xmlDefinition, *this);
	}

public:

	//! Writes a process spec to a stream.
	static void write(std::ostream& out, const ProcessSpec& procSpec);

	//! Reads a process spec from a stream.
	static void read(std::istream& in, ProcessSpec& procSpec);

	//! Writes a process spec to a string.
	static std::string write(const ProcessSpec& procSpec);

	//! Reads a process spec from a string.
	static void read(const std::string& xmlDefinition, ProcessSpec& procSpec);

public:

	//! Returns all interfaces that are available from the process output object.
	const std::vector<IID>& getOutputInterfaces() const
	{
		return m_vecOutputIID;
	}

	//! Return the unique identifier for the implemented class.
	REFCLSID getClass() const
	{
		return m_clsid;
	}

	//! Return the total number of unique inputs (a single parameter can still have multiple vaues). 
	int getParameterCount() const
	{
		return static_cast<int>(m_vecParamSpec.size());
	}

	//! Retrieve a single specific parameter.
	PYXPointer<ParameterSpec> getParameter(int n) const
	{
		assert(0 <= n && n <= std::numeric_limits<int>::max() && 
			"Out of range parameter requested");
		return m_vecParamSpec[n];
	}

	//! Reutrn the specification name.
	const std::string& getName() const
	{
		return m_strName;
	}

	//! Return the specification description.
	const std::string& getDescription() const
	{
		return m_strDescription;
	}

	//! Return the category for this process.
	const std::string& getCategory() const
	{
		return m_strCategory;
	}

	/*!
	Examine the specification and determine if the process output implements a 
	particualr interface type. Although the actual output object of the process 
	can be queried for this information that object is not always available. For 
	instance, before a process is initialized the output is not necessarily 
	available.

	\param riid	This id is compared against all know output interfaces for 
				the specificaiton.
	\return true if the process is specified to implement the interface, otherwise false.
	*/
	//! Determin if the process is able to provide a particular output type.
	bool providesOutputType(REFIID riid)
	{
		if (std::find(m_vecOutputIID.begin(), m_vecOutputIID.end(), riid) !=
			m_vecOutputIID.end())
		{
			return true;
		}
		return false;
	}

	//! Determine if a particular interface can be used as an input for any of the parameters.
	bool hasInputType(REFIID riid) const
	{
		int nParamCount = getParameterCount();
		for (int nParamNum = 0; nParamNum != nParamCount; ++nParamNum)
		{
			if (m_vecParamSpec[nParamNum]->getInterface() == riid)
			{
				return true;
			}
		}
		return false;
	}

public:

	//! Set all of the output interface ID's.
	void setOutputIIDs(const std::vector<IID>& vecOutputIIDs)
	{
		m_vecOutputIID = vecOutputIIDs;
	}

	//! Set the unique class identifier.
	void setClass(REFCLSID rclsid)
	{
		m_clsid = rclsid;
	}

	//! Set the functional name of the process spec
	void setName(const std::string& strName)
	{
		m_strName = strName;
	}

	//! Set the functional description of the process spec
	void setDescription(const std::string& strDescription)
	{
		m_strDescription = strDescription;
	}

	//! Set the process's category.
	void setCategory(const std::string& strCategory)
	{
		m_strCategory = strCategory;
	}

	//! Replace the entire complement of parameter specificaitons.
	void setParameterSpecs(const std::vector<PYXPointer<ParameterSpec> >& vecParamSpec)
	{
		m_vecParamSpec = vecParamSpec;
	}

private:

	//! The unique class ID of process that the specification defines.
	CLSID m_clsid;

	//! The list of interfaces that the process output is capable of providing.
	std::vector<IID> m_vecOutputIID;

	//! The list of parameters (input processes) for the specified process.
	std::vector<PYXPointer<ParameterSpec> > m_vecParamSpec;

	//! The name of the process.
	std::string m_strName;

	//! The description of the process.
	std::string m_strDescription;

	//! The category of the process.
	std::string m_strCategory;
};

#endif // guard
