/******************************************************************************
user_credentials_provider.cpp

begin		: Dec 07, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "user_credentials_provider.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/xml_transform.h"

// standard includes
#include <cassert>


//////////////////////////////////////////////////////////////////////////
// UserCredentialsProviderImplementation
//////////////////////////////////////////////////////////////////////////
class UserCredentialsProviderImplementation : public PYXObject
{
private: //Static members
	static PYXPointer<UserCredentialsProviderImplementation> UserCredentialsProviderImplementation::m_spInstance;

public: //Static functions
	static const PYXPointer<UserCredentialsProviderImplementation> & getInstance() 
	{
		if (!m_spInstance)
		{
			m_spInstance = PYXNEW(UserCredentialsProviderImplementation);
		}
		return m_spInstance;
	}

private:

	class Wallet : public PYXObject
	{
	public:
		static PYXPointer<Wallet> create(const std::string & name)
		{
			return PYXNEW(Wallet,name);
		}

	public:
		Wallet(const std::string & name) : m_name(name)
		{
		}
		~Wallet() {}

	public:
		boost::intrusive_ptr<IUserCredentials> getCredentials( const std::string & target, const IID & credentialsType ) 
		{
			if (credentialsType != IUsernameAndPasswordCredentials::iid)
			{
				PYXTHROW(PYXException,"unsupported credentialsType");
			}

			if (m_targets.find(target) != m_targets.end())
			{
				return m_targets[target];
			}

			return 0;
		}

		void confirmCredentials( const std::string & target, boost::intrusive_ptr<IUserCredentials> credential, bool valid ) 
		{
			if (valid)
			{
				m_targets[target] = new UserCredentialsList();
				m_targets[target]->addCredential(credential);
			}
			else
			{
				m_targets.erase(target);
			}
		}

	private:
		std::string m_name;
		std::map<std::string,boost::intrusive_ptr<UserCredentialsList>> m_targets;
	};

	std::map<std::string,PYXPointer<Wallet>> m_wallets;
	boost::recursive_mutex m_mutex;

private:
	PYXPointer<Wallet> getWallet(const std::string & name)
	{
		if (m_wallets.find(name) == m_wallets.end())
		{
			m_wallets[name] = Wallet::create(name);
		}
		return m_wallets[name];
	}

public:
	boost::intrusive_ptr<IUserCredentials> getCredentials( const std::string & wallet, const std::string & target, const IID & credentialsType )
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return getWallet(wallet)->getCredentials(target,credentialsType);
	}

	void confirmCredentials( const std::string & wallet, const std::string & target, boost::intrusive_ptr<IUserCredentials> credential, bool valid ) 
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		getWallet(wallet)->confirmCredentials(target,credential,valid);
	}
};


PYXPointer<UserCredentialsProviderImplementation> UserCredentialsProviderImplementation::m_spInstance;

//////////////////////////////////////////////////////////////////////////
// IUserCredentialsProvider
//////////////////////////////////////////////////////////////////////////

// {0D23EFBD-BB55-466f-A939-AE7FA724416F}
PYXCOM_DEFINE_IID(IUserCredentialsProvider, 
0xd23efbd, 0xbb55, 0x466f, 0xa9, 0x39, 0xae, 0x7f, 0xa7, 0x24, 0x41, 0x6f);

//////////////////////////////////////////////////////////////////////////
// UserCredentialsProviderProcess
//////////////////////////////////////////////////////////////////////////

// {34A7E82B-E78F-48fd-8B19-9B024D5E4E7F}
PYXCOM_DEFINE_CLSID(UserCredentialsProviderProcess, 
0x34a7e82b, 0xe78f, 0x48fd, 0x8b, 0x19, 0x9b, 0x2, 0x4d, 0x5e, 0x4e, 0x7f);
PYXCOM_CLASS_INTERFACES(UserCredentialsProviderProcess, IUserCredentialsProvider::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(UserCredentialsProviderProcess, "User Credentials", "user credentials provider", "Utility",
	IUserCredentialsProvider::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END


Tester<UserCredentialsProviderProcess> gTester;

void UserCredentialsProviderProcess::test()
{
	boost::intrusive_ptr<IUserCredentialsProvider> provider = PYXCOMCreateInstance<IUserCredentialsProvider>(UserCredentialsProviderProcess::clsid);

	TEST_ASSERT(provider);

	boost::intrusive_ptr<IProcess> process = provider->QueryInterface<IProcess>();

	TEST_ASSERT(process);

	TEST_ASSERT(process->initProc() == IProcess::knInitialized );

	provider->confirmCredentials("test",UsernameAndPasswordCredentials::create("user","password"),true);

	boost::intrusive_ptr<IUserCredentials> credentials = provider->getCredentials("test",IUsernameAndPasswordCredentials::iid);
	TEST_ASSERT(credentials);

	TEST_ASSERT((credentials->getCredentialsType() == IUserCredentials::iid)!=0);
	TEST_ASSERT(credentials->getCredentialsCount() > 0);
	TEST_ASSERT(credentials->getCredentials(0));
	TEST_ASSERT((credentials->getCredentials(0)->getCredentialsType() == IUsernameAndPasswordCredentials::iid)!=0);

	boost::intrusive_ptr<IUsernameAndPasswordCredentials> userAndPassword = credentials->getCredentials(0)->QueryInterface<IUsernameAndPasswordCredentials>();

	TEST_ASSERT(userAndPassword);

	TEST_ASSERT(userAndPassword->getUsername() == "user");
	TEST_ASSERT(userAndPassword->getPassword() == "password");

	provider->confirmCredentials("test",UsernameAndPasswordCredentials::create("user2","password2"),true);
	credentials = provider->getCredentials("test",IUsernameAndPasswordCredentials::iid);
	TEST_ASSERT(credentials);

	TEST_ASSERT(credentials->getCredentialsCount() > 0);
	TEST_ASSERT(credentials->getCredentials(0));
	TEST_ASSERT((credentials->getCredentials(0)->getCredentialsType() == IUsernameAndPasswordCredentials::iid)!=0);

	userAndPassword = credentials->getCredentials(0)->QueryInterface<IUsernameAndPasswordCredentials>();

	TEST_ASSERT(userAndPassword);

	TEST_ASSERT(userAndPassword->getUsername() == "user2");
	TEST_ASSERT(userAndPassword->getPassword() == "password2");

	provider->confirmCredentials("test",UsernameAndPasswordCredentials::create("user2","password2"),false);

	credentials = provider->getCredentials("test",IUsernameAndPasswordCredentials::iid);
	TEST_ASSERT(!credentials);
}

std::string STDMETHODCALLTYPE UserCredentialsProviderProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:element name=\"UserCredentialsProviderProcess\">"
		 "<xs:complexType>"
		  "<xs:sequence>"
		   "<xs:element name=\"Wallet\" type=\"xs:string\">"
		    "<xs:annotation>"
		     "<xs:appinfo>"
		      "<friendlyName>Wallet</friendlyName>"
		      "<description>Wallet to use. (default wallet is empty string)</description>"
		     "</xs:appinfo>"
		    "</xs:annotation>"
		   "</xs:element>"
		  "</xs:sequence>"
		 "</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";	
}

std::map<std::string, std::string> STDMETHODCALLTYPE UserCredentialsProviderProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["Wallet"] = m_wallet;
	return mapAttr;
}

void STDMETHODCALLTYPE UserCredentialsProviderProcess::setAttributes( const std::map<std::string, std::string>& mapAttr )
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Wallet",m_wallet);	
}

IProcess::eInitStatus UserCredentialsProviderProcess::initImpl()
{
	return IProcess::knInitialized;
}

boost::intrusive_ptr<IUserCredentials> STDMETHODCALLTYPE UserCredentialsProviderProcess::getCredentials( std::string target,const IID & credentialsType ) const
{
	return UserCredentialsProviderImplementation::getInstance()->getCredentials(m_wallet,target,credentialsType);
}

void STDMETHODCALLTYPE UserCredentialsProviderProcess::confirmCredentials( std::string target,boost::intrusive_ptr<IUserCredentials> credential,bool valid ) const
{
	return UserCredentialsProviderImplementation::getInstance()->confirmCredentials(m_wallet,target,credential,valid);
}
