#define PYXLIB_SOURCE
#include "stdafx.h"

#include "win32_exception.h"
#include "eh.h"

Win32Exception::Handler::Handler()
{
	m_oldHandler = _set_se_translator(Win32Exception::translate);
}

Win32Exception::Handler::~Handler()
{
	_set_se_translator(m_oldHandler);
}

void Win32Exception::install_handler()
{
    _set_se_translator(Win32Exception::translate);
}

void Win32Exception::translate(unsigned code, EXCEPTION_POINTERS* info)
{
    // Windows guarantees that *(info->ExceptionRecord) is valid
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:
        throw Win32AccessViolationException(*(info->ExceptionRecord));
        break;
    default:
        throw Win32Exception(*(info->ExceptionRecord));
    }
}

Win32Exception::Win32Exception(const EXCEPTION_RECORD& info)
: mWhat("Win32 exception"), mWhere(info.ExceptionAddress), mCode(info.ExceptionCode)
{
    switch (info.ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        mWhat = "Access violation";
        break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        mWhat = "Division by zero";
        break;
    }
}

Win32AccessViolationException::Win32AccessViolationException(const EXCEPTION_RECORD& info)
: Win32Exception(info), mIsWrite(false), mBadAddress(0)
{
    mIsWrite = info.ExceptionInformation[0] == 1;
    mBadAddress = reinterpret_cast<Win32Exception::Address>(info.ExceptionInformation[1]);
}