/******************************************************************************
command.cpp

begin		: 2005-03-30
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "command.h"
#include "pyxis/utility/exceptions.h"

Command::~Command()
{
	delete m_pLastException;
}

std::string Command::getCategory() const
{
	return m_strCategory;
}

void Command::setCategory(const std::string& strCategory)
{
	m_strCategory = strCategory;
}

std::vector<PYXPointer<Command> > &Command::getSubordinates() 
{
	return m_vecSubordinates;
}

void Command::addSubordinate(PYXPointer<Command> spCommand)
{
	m_vecSubordinates.push_back(spCommand);

	/* TODO[kabiraman]: Temporary code to ensure proper order of subordinates 
	when nesting across multiple categories.
	*/
	PYXPointer<Command> spTempCommand;

    for (int i = 0; i < (static_cast<int>(m_vecSubordinates.size()) - 1); ++i)
    {
		for(int j = 1; j < static_cast<int>(m_vecSubordinates.size()); ++j)
		{
			if (m_vecSubordinates[j]->getPriority() > 
				m_vecSubordinates[j - 1]->getPriority())
			{
				spTempCommand = m_vecSubordinates[j];
                m_vecSubordinates[j] = m_vecSubordinates[j - 1];
                m_vecSubordinates[j - 1] = spTempCommand;
			}
		}
	}
}