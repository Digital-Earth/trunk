#ifndef PYXIS__PIPE__PROCESS_IDENTITY_H
#define PYXIS__PIPE__PROCESS_IDENTITY_H
/******************************************************************************
process_identity.h

begin		: 2008-06-02
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom.h"

// std includes
#include <map>
#include <vector>

class Parameter;

/*!
This class uses a functor pattern, whereby the instance
is constructed, additional properties are set,
and the () operator is called to get the resulting string,
which is the process identity xml corresponding to the arguments
provided.
*/
//! Encapsulates a process identity.
class PYXLIB_DECL ProcessIdentity
{
	//! The class id value, formatted for xml.
	const std::string m_strXMLClassID;

	//! The data string value, formatted for xml.
	std::string m_strXMLData;

	//! The checksum node. 
	std::string m_strChecksum;

	//! The attributes node.
	std::string m_strXMLAttributes;

	//! The input nodes.
	std::string m_strXMLInputs;

public:

	//! Test method
	static void test();

public:

	//! Construct a process identity.
	ProcessIdentity(const IID& clsID);

	virtual ~ProcessIdentity();

public:

	//! Set the data in the identity.
	void setData(const std::string& strData);

	//! Set the checksum in the identity. 
	void setChecksum(const std::string& strData);

	//! Set the attributes in the identity.
	void setAttributes(
		const std::map<std::string, std::string>& mapAttributes);

	//! Add an input to the identity.
	void addInput(const Parameter& paramInput,
		bool bIsValueOrderSignificant = true);

	/*!
	Return an xml node "identity" which has the following format:

	\code
		<identity clsid="...">
			<data>...</data>
	
			<attributes>
				<attribute name="...">...</attribute>
				...
			</attributes>
	
			<inputs>
				<input>
					<identity>...<identity>
					...
				</input>
				...
			</inputs>
		</identity>
	\endcode
	*/
	//! Return the string identity in XML form.
	std::string operator ()();
};

#endif
