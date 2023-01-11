#ifndef PYXIS__PROCS__USER_CREDENTIALS_H
#define PYXIS__PROCS__USER_CREDENTIALS_H
/******************************************************************************
user_credentials.h

begin		: Dec 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"


/*!

Interface User Credentials 

This object is the base class for presenting a user credentials for using remote and local resource.

User credentials object is generated using IUserCredentialsProvider interface.
Process can use given IUserCredinatials to generate a valid requests for remote resources.

*/
//! Interface User Credentials object
struct PYXLIB_DECL IUserCredentials : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! return the type of the credentials (user+password,key,certificate) etc..
	virtual IID STDMETHODCALLTYPE getCredentialsType() const = 0;

	//! return the number of credentials contains in this instance
	virtual int STDMETHODCALLTYPE getCredentialsCount() const = 0;

	//! return the credentials number #index.
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE getCredentials(int index = 0) const = 0;

	//! return the first credentials that match the given type.
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE findFirstCredentialOfType(const IID & type) const = 0;
};

struct PYXLIB_DECL IUserCredentialsError : public IProcessInitError
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! return the type of the credentials needed/been used (user+password,key,certificate, etc..)
	virtual IID STDMETHODCALLTYPE getNeededCredentialsType() const = 0;

	//! return the target for the credentials (domain, machine name etc..)
	virtual std::string STDMETHODCALLTYPE getCredentialsTarget() const = 0;
};

//! An error that indicates process could not be successfully initialized due to missing or invalid user credentials.
class PYXLIB_DECL UserCredentialsInitError : public IUserCredentialsError
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
		IUNKNOWN_QI_CASE(IUserCredentialsError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(UserCredentialsInitError)

public: //IProcessInitError
	UserCredentialsInitError() :
	m_strError("Missing or invalid user credentials."), m_credentialsType(IUserCredentials::iid),m_target("")
	{
	}

	UserCredentialsInitError(const IID & credentialsType, const std::string & target) :
	m_strError("Missing or invalid user credentials."), m_credentialsType(credentialsType), m_target(target)
	{
	}

	virtual const std::string& STDMETHODCALLTYPE getError() const
	{
		return m_strError;
	}

	virtual void STDMETHODCALLTYPE setError(const std::string& strError) {m_strError = strError;}

public: //IUserCredentialsError
	virtual IID STDMETHODCALLTYPE getNeededCredentialsType() const 
	{
		return m_credentialsType;
	}

	virtual std::string STDMETHODCALLTYPE getCredentialsTarget() const
	{
		return m_target;
	}

public: //UserCredentialsInitError
	void setCredentialsDetials(const IID & credentialsType, const std::string & target)
	{
		m_credentialsType = credentialsType;
		m_target = target;
	}

protected:
	std::string m_strError;
	IID m_credentialsType;
	std::string m_target;
};


/*!
An object represent a list of certificate.
/*!
*/
//! An object represent a list of certificates.
class PYXLIB_DECL UserCredentialsList :  public IUserCredentials
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IUserCredentials)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IUserCredentials

	virtual IID STDMETHODCALLTYPE getCredentialsType() const;
	virtual int STDMETHODCALLTYPE getCredentialsCount() const;
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE getCredentials(int index = 0) const;
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE findFirstCredentialOfType(const IID & type) const;

public: // UserCredentialsList
	void STDMETHODCALLTYPE addCredential( const boost::intrusive_ptr<IUserCredentials> & credential);

private:

	//! The storage of all of the associated credentials.
	std::vector< boost::intrusive_ptr<IUserCredentials> > m_credentialsList;
};

/*!

Interface User Credentials for username and password


*/
//! Interface IUsernameAndPasswordCredentials 
struct PYXLIB_DECL IUsernameAndPasswordCredentials : public IUserCredentials
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! return the username
	virtual std::string STDMETHODCALLTYPE getUsername() const = 0;

	//! return the password
	virtual std::string STDMETHODCALLTYPE getPassword() const = 0;
};


/*!
represent a username and password
/*!
*/
//! An object represent a list of certificates.
class PYXLIB_DECL UsernameAndPasswordCredentials :  public IUsernameAndPasswordCredentials
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IUserCredentials)
		IUNKNOWN_QI_CASE(IUsernameAndPasswordCredentials)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IUserCredentials

	virtual IID STDMETHODCALLTYPE getCredentialsType() const;
	virtual int STDMETHODCALLTYPE getCredentialsCount() const;
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE getCredentials(int index = 0) const;
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE findFirstCredentialOfType(const IID & type) const;

public: // IUsernameAndPasswordCredentials

	virtual std::string STDMETHODCALLTYPE getUsername() const;
	virtual std::string STDMETHODCALLTYPE getPassword() const;

public: //UsernameAndPasswordCredentials

	static boost::intrusive_ptr<IUserCredentials> create(const std::string & username, const std::string & password)
	{
		boost::intrusive_ptr<UsernameAndPasswordCredentials> result = new UsernameAndPasswordCredentials();
		result->setUsernameAndPassword(username,password);
		return result;
	}

	virtual void STDMETHODCALLTYPE setUsernameAndPassword(const std::string & username, const std::string & password );	

private:

	//! The storage of all of the associated credentials.
	std::string m_username;
	std::string m_password;
};

#endif // guard
