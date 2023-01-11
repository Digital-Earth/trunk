/******************************************************************************
user_credentials.cpp

begin		: Dec 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "user_credentials.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <cassert>


//////////////////////////////////////////////////////////////////////////
// IUserCredentials
//////////////////////////////////////////////////////////////////////////

// {0D23EFBD-BB55-466f-A939-AE7FA724416F}
PYXCOM_DEFINE_IID(IUserCredentials, 
0xd23efbd, 0xbb55, 0x466f, 0xa9, 0x39, 0xae, 0x7f, 0xa7, 0x24, 0x41, 0x6f);

//////////////////////////////////////////////////////////////////////////
// IUserCredentialsError
//////////////////////////////////////////////////////////////////////////

// {DB24C824-55F1-454d-A384-20BA71006804}
PYXCOM_DEFINE_IID(IUserCredentialsError, 
0xdb24c824, 0x55f1, 0x454d, 0xa3, 0x84, 0x20, 0xba, 0x71, 0x0, 0x68, 0x4);

//////////////////////////////////////////////////////////////////////////
// UserCredentialsInitError
//////////////////////////////////////////////////////////////////////////

// {100B9867-D8E2-4a72-81DE-DE8FEB5187FD}
PYXCOM_DEFINE_CLSID(UserCredentialsInitError, 
0x100b9867, 0xd8e2, 0x4a72, 0x81, 0xde, 0xde, 0x8f, 0xeb, 0x51, 0x87, 0xfd);
PYXCOM_CLASS_INTERFACES(UserCredentialsInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);


//////////////////////////////////////////////////////////////////////////
// UserCredentialsList
//////////////////////////////////////////////////////////////////////////

// {F01D9B88-81D4-4398-85BE-015C08231703}
PYXCOM_DEFINE_CLSID(UserCredentialsList, 
0xf01d9b88, 0x81d4, 0x4398, 0x85, 0xbe, 0x1, 0x5c, 0x8, 0x23, 0x17, 0x3);
PYXCOM_CLASS_INTERFACES(UserCredentialsList, IUserCredentials::iid, PYXCOM_IUnknown::iid);

IID STDMETHODCALLTYPE UserCredentialsList::getCredentialsType() const
{	
	return IUserCredentials::iid;
}

int STDMETHODCALLTYPE UserCredentialsList::getCredentialsCount() const
{
	return m_credentialsList.size();
}

boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE UserCredentialsList::getCredentials(int index) const
{
	assert(m_credentialsList.size()>(unsigned int)index);
	if (m_credentialsList.size()>(unsigned int)index)
	{
		return m_credentialsList[index];
	}
	PYXTHROW(PYXException,"Index parameter is out of range");
}

void STDMETHODCALLTYPE UserCredentialsList::addCredential( const boost::intrusive_ptr<IUserCredentials> & credential )
{
	VALIDATE_ARGUMENT_NOT_NULL(credential);	
	m_credentialsList.push_back(credential);
}

boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE UserCredentialsList::findFirstCredentialOfType( const IID & type ) const
{
	for(std::vector< boost::intrusive_ptr<IUserCredentials> >::const_iterator it = m_credentialsList.begin();
		it != m_credentialsList.end();
		++it)
	{
		boost::intrusive_ptr<IUserCredentials> result = (*it)->findFirstCredentialOfType(type);

		if (result)
		{
			return result;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// IUsernameAndPasswordCredentials
//////////////////////////////////////////////////////////////////////////

// {C4530CB1-6618-408a-94D4-BFD07F91C7F0}
PYXCOM_DEFINE_IID(IUsernameAndPasswordCredentials, 
0xc4530cb1, 0x6618, 0x408a, 0x94, 0xd4, 0xbf, 0xd0, 0x7f, 0x91, 0xc7, 0xf0);

//////////////////////////////////////////////////////////////////////////
// UsernameAndPasswordCredentials
//////////////////////////////////////////////////////////////////////////

// {68AC6902-296D-4738-AC46-70B553603425}
PYXCOM_DEFINE_CLSID(UsernameAndPasswordCredentials, 
0x68ac6902, 0x296d, 0x4738, 0xac, 0x46, 0x70, 0xb5, 0x53, 0x60, 0x34, 0x25);
PYXCOM_CLASS_INTERFACES(UsernameAndPasswordCredentials, IUsernameAndPasswordCredentials::iid, IUserCredentials::iid, PYXCOM_IUnknown::iid);

IID STDMETHODCALLTYPE UsernameAndPasswordCredentials::getCredentialsType() const
{
	return IUsernameAndPasswordCredentials::iid;
}

int STDMETHODCALLTYPE UsernameAndPasswordCredentials::getCredentialsCount() const
{
	return 0;
}

boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE UsernameAndPasswordCredentials::getCredentials( int index /*= 0*/ ) const
{
	return 0;
}

std::string STDMETHODCALLTYPE UsernameAndPasswordCredentials::getUsername() const
{
	return m_username;
}

std::string STDMETHODCALLTYPE UsernameAndPasswordCredentials::getPassword() const
{
	return m_password;
}

void STDMETHODCALLTYPE UsernameAndPasswordCredentials::setUsernameAndPassword( const std::string & username, const std::string & password )
{
	m_username = username;
	m_password = password;
}

boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE UsernameAndPasswordCredentials::findFirstCredentialOfType( const IID & type ) const
{
	if (type == IUsernameAndPasswordCredentials::iid)
	{
		return const_cast<UsernameAndPasswordCredentials*>(this);
	}
	return NULL;
}
