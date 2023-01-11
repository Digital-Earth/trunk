/******************************************************************************
file_resolver.h

begin      : 03/10/2007 2:34:21 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#ifndef FILE_RESOLVER_H
#define FILE_RESOLVER_H

// pyxlib includes
#include "pyxlib.h"

#include <vector>
#include "app_services.h"
#include "exception.h"

class PYXLIB_DECL Checksum{
};

class PYXLIB_DECL ResolverAgent{
public:
	virtual std::string ResolveFile (const std::string &filename)
	{
		return "";
	}
	virtual void OnFileResolved (const std::string &filename)
	{
	}
};

class PYXLIB_DECL FileResolver{

	typedef std::vector<ResolverAgent *> ResolverList;
	static ResolverList m_registeredResolvers;

	static void RegisterResolver( ResolverAgent *agent)
	{
		m_registeredResolvers.push_back(agent);
	}

	static std::string ResolveFile( const std::string &filename)
	{
		for (ResolverList::iterator it = m_registeredResolvers.begin(); 
			it != m_registeredResolvers.end(); 
			++it)
		{
			std::string result = (*it)->ResolveFile(filename);
			if (!result.empty())
			{
				for (ResolverList::iterator it2 = m_registeredResolvers.begin(); 
					it2 != m_registeredResolvers.end(); 
					++it2)
				{
					(*it)->OnFileResolved(filename);
				}
				return result;
			}
		}
		PYXTHROW( PYXException, "Unable to find file " << filename);
	}
};

#endif // guard
