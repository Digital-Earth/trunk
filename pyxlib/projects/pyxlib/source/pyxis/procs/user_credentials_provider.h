#ifndef PYXIS__PROCS__USER_CREDENTIALS_PROVIDER_H
#define PYXIS__PROCS__USER_CREDENTIALS_PROVIDER_H
/******************************************************************************
user_credentials_provider.h

begin		: Dec 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/procs/user_credentials.h"

/*
Interface User Credentials Provider process

This process interface can be used to retrieves credentials for the user.

Use getCredentials to retrieve cached credentials, in now cached credentials exists, a dialog will popup for the user to enter the credentials.
Validation of getCredentials will be performed by the calling class.
One validation completed (successful or not) you should call confirmCredentials to store / delete the credentials from the cache.

*/
//! Interface User Credentials Provider object
struct PYXLIB_DECL IUserCredentialsProvider : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! retrieves user credentials that of a given type (Interface) for the given target (e.g. : domain).
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE getCredentials(std::string target,const IID & credentialsType) const = 0;

	//! specify that a given credential is valid (or not) for the given target.
	virtual void STDMETHODCALLTYPE confirmCredentials(std::string target,boost::intrusive_ptr<IUserCredentials> credential,bool valid) const = 0;
};

/*!
provide a process that output IUserCredentialsProvider.

this process have a single attribute called "Wallet". default value is empty string.
a user can generate several wallets to easily switch between wallets.
for example to check how a pipeline would like for user A and user B.

*/
//! A process that defines a local or network file or directory resource.
class PYXLIB_DECL UserCredentialsProviderProcess : public ProcessImpl<UserCredentialsProviderProcess>, public IUserCredentialsProvider
{
	PYXCOM_DECLARE_CLASS();

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IUserCredentialsProvider)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( UserCredentialsProviderProcess, IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IUserCredentialsProvider*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IUserCredentialsProvider*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IUserCredentialsProvider
	virtual boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE getCredentials(std::string target,const IID & credentialsType) const;
	virtual void STDMETHODCALLTYPE confirmCredentials(std::string target,boost::intrusive_ptr<IUserCredentials> credential,bool valid) const;

private:

	std::string m_wallet;
};

#endif // guard
