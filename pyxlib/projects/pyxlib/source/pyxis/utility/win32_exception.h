#ifndef PYXIS__UTILITY__WIN32_EXCEPTION_H
#define PYXIS__UTILITY__WIN32_EXCEPTION_H
/******************************************************************************
win32_exception.h

begin		: 2007-03-08
copyright	: (C) 2007 by the PYXIS innovation inc.  
(portions copyright from the web page 
http://www.thunderguy.com/semicolon/2002/08/15/visual-c-exception-handling/)

web			: www.pyxisinnovation.com
******************************************************************************/

#pragma once

#include <windows.h>
#include <exception>

/*!
This class wraps up Window's SEH exceptions, along with a static install_handler
that connects SEH to standard exceptions.  Any app using this must compile with 
/EHa (or /EHac) under VC++ 2005.

Note that the handler must be installed, either through a call to 
install_handler (installs permanently) or creating a Handler object
(which temporarily installs the handler.)

http://www.thunderguy.com/semicolon/2002/08/15/visual-c-exception-handling/
contains an excellent overview of the issues involved.  Please read it!
(Most of this code is sourced from that article.)
*/
//! Wraps up Windows SEH exceptions.
class Win32Exception: public std::exception
{
public:
	class Handler{
		_se_translator_function m_oldHandler;
	public:
		Handler();
		~Handler();
	private:
		Handler( const Handler &);
		Handler &operator=( const Handler &);
	};

    typedef const void* Address; // OK on Win32 platform

	//! Must be called in every thread that wants its SEH exceptions handled.
    static void install_handler();
    virtual const char* what() const { return mWhat; };
    Address where() const { return mWhere; };
    unsigned code() const { return mCode; };
protected:
    Win32Exception(const EXCEPTION_RECORD& info);
    static void translate(unsigned code, EXCEPTION_POINTERS* info);
private:
    const char* mWhat;
    Address mWhere;
    unsigned mCode;

	friend class Handler;
};

/*!
This class wraps up Window's SEH Access Violation exception.  Note that these 
exceptions will only be generated if you compile with /EHa (or /EHac) under 
VC++ 2005, _and_ your thread has called Win32Exception::register_handler()
*/
//! Wraps up Windows SEH access violation exceptions.
class Win32AccessViolationException: public Win32Exception
{
public:
    bool isWrite() const { return mIsWrite; };
    Address badAddress() const { return mBadAddress; };
private:
    bool mIsWrite;
    Address mBadAddress;
    Win32AccessViolationException(const EXCEPTION_RECORD& info);
    friend void Win32Exception::translate(unsigned code, EXCEPTION_POINTERS* info);
};

#endif /* PYXIS__UTILITY__WIN32_EXCEPTION_H */