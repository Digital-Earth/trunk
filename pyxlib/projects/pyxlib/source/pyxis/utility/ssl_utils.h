#ifndef PYXIS__UTILITY__SSL_UTILS_H
#define PYXIS__UTILITY__SSL_UTILS_H
/******************************************************************************
ssl_utils.h

begin		: 2008-05-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "wire_buffer.h"

// standard includes
#include <string>
#include <vector>

// boost includes
#include <boost/array.hpp>
#include <boost/filesystem/path.hpp>


/*!
This class wraps the SSL library and exposes functionality as needed.
*/
struct SSLUtils
{
	//! Call exactly once to initialize.
	static void initialize();

	//! Call exactly once to uninitialize.
	static void uninitialize();

	/*!
	This class contains various utilities for generating a hash or checksum for a file. 
	*/
	//! Checksum utility
	class PYXLIB_DECL Checksum
	{
		struct SSLState;

		SSLState * m_state;

	public:

		//! Test method
		static void test();

	public:

		/*!
		Construct a checksum object of the given type (e.g. "MD5", "SHA1").
		Initially an empty checksum.
		*/
		//! Construct a checksum object of the given type (e.g. "MD5", "SHA1").
		explicit Checksum(const std::string& strType);

		virtual ~Checksum();

	public:

		//! Generate the checksum from the source string.
		bool generate(const std::string& strSource);

		//! Generate the checksum from the source memory.
		bool generate(const PYXConstBufferSlice & source);

		//! Generate the checksum from the source strings.
		bool generate(const std::vector<const std::string>& vecSources);

		//! Generate the checksum for the file at the given path.
		bool generate(const boost::filesystem::path& path);

		//! Return the type of checksum (e.g. "MD5", "SHA1").
		const std::string& getType() const;

		//! Return the number of bytes in the checksum.
		unsigned int getLength() const;

		//! Return the byte at the given 0-based offset in the checksum.
		unsigned char getByte(unsigned int nIndex) const;

		//! Return a hex string representation of the checksum.
		std::string toHexString() const;

		//! Return a base 64 string representation of the checksum.
		std::string toBase64String() const;
	};
};

#endif
