/******************************************************************************
command_manager.cpp

begin		: 2005-03-30
copyright	: (C) 2005 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "command_manager.h"

// pyxlib includes
#include "pyxis/utility/command.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <cassert>

// static definitions
//! The maximum number of commands allowed in the list.
static int knMaxCommands = 20;

//! Command for test purposes
class TestCommand : public Command
{
public:

	//! Dynamic creator
	static PYXPointer<TestCommand> create(int nCommandID, int* pnLastCommand)
	{
		return PYXNEW(TestCommand, nCommandID, pnLastCommand);
	}

	//! Constructor
	TestCommand(int nCommandID, int* pnLastCommand) :
	  m_nExecuted(0),
	  m_nUndone(0),
	  m_nRedone(0),
	  m_nCommandID(nCommandID),
	  m_pnLastCommand(pnLastCommand) {;}

	//! Destructor
	virtual ~TestCommand() {;}

	//! Get the number of times the command was executed
	int getExecuted() const {return m_nExecuted;}

	//! Get the number of times the command was undone
	int getUndone() const {return m_nUndone;}

	//! Get the number of times the command was redone
	int getRedone() const {return m_nRedone;}

	//! Get name method
	virtual std::string getName() const {return "Test Command";}

	//! Execute method
	virtual bool execute()
	{
		++m_nExecuted;
		if (m_pnLastCommand != 0)
		{
			*m_pnLastCommand = m_nCommandID;
		}
		return true;
	}

	//! Undo method
	virtual bool undo()
	{
		++m_nUndone;
		if (m_pnLastCommand != 0)
		{
			*m_pnLastCommand = m_nCommandID;
		}
		return true;
	}

	//! Redo method
	virtual bool redo()
	{
		++m_nRedone;
		if (m_pnLastCommand != 0)
		{
			*m_pnLastCommand = m_nCommandID;
		}
		return true;
	}

	virtual int getPriority() const
	{
		return 0;
	}

	virtual bool getRecordable() const
	{
		return true;
	}

	virtual bool getUndoable() const
	{
		return true;
	}

	virtual Command::CommandType getType() const
	{
		return knUnspecified;
	}

	virtual std::string getToolTip() const
	{
		return "";
	}

	virtual bool getEnabled() const
	{
		return true;
	}

	virtual bool getChecked() const
	{
		return false;
	}

private:

	//! The name
	static const std::string m_strName;

	//! Record the command state
	int m_nExecuted;
	int m_nUndone;
	int m_nRedone;

	//! Allows recording order of commands
	int m_nCommandID;
	int* m_pnLastCommand;	
};

//! Command that throws exceptions when any method (except 'getName()') is invoked.
class FailCommand : public Command
{
public:

	enum eFailType
	{
		knExecute = 0,
		knUndo,
		knRedo
	};

	//! Dynamic creator
	static PYXPointer<FailCommand> create(eFailType nFailType)
	{
		return PYXNEW(FailCommand, nFailType);
	}

	//! Default constructor.
	FailCommand(eFailType nFailType) :
		m_nFailType(nFailType) {}


	//! Default destructor.
	virtual ~FailCommand() {;}

	//! Return an uninitialized string
	virtual std::string getName() const {return "Fail Command";}

	//! Execute the command.
	bool execute() 
	{
		if (m_nFailType == knExecute)
		{
			PYXTHROW(PYXCommandExecuteException, "Fail Command exception.");
		}
		return true;
	}

	//! Undo the command.
	bool undo() 
	{
		if (m_nFailType == knUndo)
		{
			PYXTHROW(PYXCommandUndoException, "Fail Command exception.");
		}
		return true;
	}

	//! Redo the command.
	bool redo() 
	{
		if (m_nFailType == knRedo)
		{
			PYXTHROW(PYXException, "Fail Command exception.");
		}
		return true;
	}

	virtual int getPriority() const
	{
		return 0;
	}

	virtual bool getRecordable() const
	{
		return true;
	}

	virtual bool getUndoable() const
	{
		return true;
	}
	
	virtual Command::CommandType getType() const
	{
		return knUnspecified;
	}

	virtual std::string getToolTip() const
	{
		return "";
	}

	virtual bool getEnabled() const
	{
		return true;
	}

	virtual bool getChecked() const
	{
		return false;
	}

private:

	//! Disable default constructor
	FailCommand();

	//! The type of failure to create
	eFailType m_nFailType;
};

//! Tester class
Tester<CommandManager> gTester;

//! Test method
void CommandManager::test()
{
	int nLastCommand = -1;

	// setup test commands
	PYXPointer< TestCommand > spCommand1(TestCommand::create(1, &nLastCommand));
	PYXPointer< TestCommand > spCommand2(TestCommand::create(2, &nLastCommand));
	PYXPointer< TestCommand > spCommand3(TestCommand::create(3, &nLastCommand));

	CommandManager commandManager;

	TEST_ASSERT(!commandManager.canUndo());
	TEST_ASSERT(!commandManager.canRedo());

	TEST_ASSERT(commandManager.executeCommand(spCommand1));
	TEST_ASSERT(spCommand1->getExecuted() == 1);
	TEST_ASSERT(spCommand1->getUndone() == 0);
	TEST_ASSERT(spCommand1->getRedone() == 0);
	TEST_ASSERT(nLastCommand == 1);
	TEST_ASSERT(commandManager.canUndo());
	TEST_ASSERT(commandManager.getUndoCommand() == spCommand1);
	TEST_ASSERT(!commandManager.canRedo());

	TEST_ASSERT(commandManager.undoCommand());
	TEST_ASSERT(spCommand1->getExecuted() == 1);
	TEST_ASSERT(spCommand1->getUndone() == 1);
	TEST_ASSERT(spCommand1->getRedone() == 0);
	TEST_ASSERT(nLastCommand == 1);
	TEST_ASSERT(!commandManager.canUndo());
	TEST_ASSERT(commandManager.canRedo());
	TEST_ASSERT(commandManager.getRedoCommand() == spCommand1);

	TEST_ASSERT(commandManager.redoCommand());
	TEST_ASSERT(spCommand1->getExecuted() == 1);
	TEST_ASSERT(spCommand1->getUndone() == 1);
	TEST_ASSERT(spCommand1->getRedone() == 1);
	TEST_ASSERT(nLastCommand == 1);
	TEST_ASSERT(commandManager.canUndo());
	TEST_ASSERT(commandManager.getUndoCommand() == spCommand1);
	TEST_ASSERT(!commandManager.canRedo());

	commandManager.recordCommand(spCommand2);
	TEST_ASSERT(spCommand2->getExecuted() == 0);
	TEST_ASSERT(spCommand2->getUndone() == 0);
	TEST_ASSERT(spCommand2->getRedone() == 0);
	TEST_ASSERT(nLastCommand == 1);

	TEST_ASSERT(commandManager.executeCommand(spCommand3));
	TEST_ASSERT(spCommand3->getExecuted() == 1);
	TEST_ASSERT(spCommand3->getUndone() == 0);
	TEST_ASSERT(spCommand3->getRedone() == 0);
	TEST_ASSERT(nLastCommand == 3);

	TEST_ASSERT(commandManager.canUndo());
	TEST_ASSERT(commandManager.getUndoCommand() == spCommand3);
	TEST_ASSERT(!commandManager.canRedo());

	TEST_ASSERT(commandManager.undoCommand());
	TEST_ASSERT(spCommand3->getExecuted() == 1);
	TEST_ASSERT(spCommand3->getUndone() == 1);
	TEST_ASSERT(spCommand3->getRedone() == 0);
	TEST_ASSERT(nLastCommand == 3);
	TEST_ASSERT(commandManager.canUndo());
	TEST_ASSERT(commandManager.getUndoCommand() == spCommand2);
	TEST_ASSERT(commandManager.canRedo());
	TEST_ASSERT(commandManager.getRedoCommand() == spCommand3);

	commandManager.clearAllCommands();
	TEST_ASSERT(!commandManager.canUndo());
	TEST_ASSERT(!commandManager.canRedo());

	// test the failure handling of the commands
	TEST_ASSERT(commandManager.executeCommand(spCommand1));
	TEST_ASSERT(commandManager.undoCommand());
	TEST_ASSERT(commandManager.canRedo());
	PYXPointer< Command > spFailCmd = FailCommand::create(FailCommand::knExecute);
	TEST_ASSERT(!commandManager.executeCommand(spFailCmd));
	TEST_ASSERT(commandManager.canRedo());
	spFailCmd = FailCommand::create(FailCommand::knRedo);
	TEST_ASSERT(commandManager.executeCommand(spFailCmd));
	TEST_ASSERT(!commandManager.canRedo());
	TEST_ASSERT(commandManager.canUndo());
	TEST_ASSERT(commandManager.undoCommand());
	TEST_ASSERT(commandManager.canRedo());
	TEST_ASSERT(!commandManager.redoCommand());

	// test the exception handling of the commands

	// verify the execute exception is handled by the command manager
	spFailCmd = FailCommand::create(FailCommand::knExecute);
	TEST_ASSERT(!commandManager.executeCommand(spFailCmd));
	TEST_ASSERT(commandManager.size() == 0);
}

/*!
Constructor initializes variables.
*/
CommandManager::CommandManager() :
	m_lstCommands(),
	m_itCurrent(m_lstCommands.end()),
	m_notifier("Command Manager"), 
	m_bIsExecuting(false)
{
}

/*!
Destructor cleans up memory.
*/
CommandManager::~CommandManager()
{
	clearAllCommands();
}

/*!
Clear all commands in the command manager.
*/
void CommandManager::clearAllCommands()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	TRACE_INFO("Clearing all commands from command manager.");
	m_lstCommands.clear();
	m_itCurrent = m_lstCommands.end();
}

// Simple utility class to ensure exception-safe reset of in-process flag.
struct Guard{
	bool &ref;

	Guard( bool &reference):
        ref( reference)
	{
		ref = true;
	}
	~Guard()
	{
		ref = false;
	}
};

/*!
Execute and record a command.

\param	spCommand	The command to execute and add to the command stack.

\return true if the command is executed successfully, otherwise false.
*/
bool CommandManager::executeCommand(PYXPointer< Command > spCommand)
{
	assert((spCommand != 0) && "Invalid command.");
	bool bResult = false;

	// do not execute another command if already executing one
	if(!m_bIsExecuting)
	{
		Guard setExecutingFlag(m_bIsExecuting);

		// get the name of the command being executed
		std::string strName = spCommand->getLongName();

		// Storage for error messages.
		std::string strNotificationMsg;

		try
		{
			TRACE_INFO("Executing command: '" << strName << "'.");

			bResult = spCommand->execute();

			// the user didn't cancel out of the command
			if (bResult)
			{
				if (spCommand->getRecordable())
				{
					recordCommand(spCommand);
				}
				else
				{
					// so that clients that need to refresh can do so.
					sendNotification("");					
				}
			}
			return true;
		}
		catch (PYXCommandExecuteException& e)
		{
			strNotificationMsg = "The command '" + strName + "' failed with " + 
				"the following error:\n" + e.getFullErrorString();
		}		
		catch(...)
		{
			if(spCommand->getLastException() == 0)
			{
				strNotificationMsg = "Unknown error caused the command '" + 
					strName + 
					"' to fail while the CommandManager tried to execute it.";
			}
			else
			{
				PYXCommandExecuteException* executeException = 
					dynamic_cast<PYXCommandExecuteException*>(
					spCommand->getLastException());

				if(executeException != 0)
				{
					strNotificationMsg = 
						"The command '" + strName + "' failed with " + 
					"the following error:\n" + 
						executeException->getFullErrorString();
				}
				else
				{
					strNotificationMsg = "Unknown error caused the command '" + 
						strName + 
						"' to fail while the CommandManager tried to execute it.";					
				}

				// clear the last exception
				spCommand->setLastException(0);
			}
		}

		// notify all observers of command execution, even though it failed.
		TRACE_ERROR(strNotificationMsg);			
		sendNotification(strNotificationMsg);
		return false;
	}
	else
	{
		TRACE_INFO("Can't execute command because command is already being executed : " << spCommand->getLongName());
		return true;
	}
}

/*!
Record a command without executing it. This is typically used for direct
manipulation operations where the application responds as the user interacts
with it (for example moving an object). Typically the interaction is performed
directly and only the end result is recorded as a command. This facilitates
undo and redo operations.

\param spCommand	The command to record on the command stack without executing.
*/
void CommandManager::recordCommand(PYXPointer< Command > spCommand)
{
	assert(spCommand != 0 && "Invalid argument.");	
	
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// clear all commands that have been undone
	clearUndone();

	// ensure command list doesn't grow too large
	if (m_lstCommands.size() > static_cast<CommandList::size_type>(knMaxCommands))
	{
		m_lstCommands.pop_back();
	}

	// add the command to the list
	m_lstCommands.push_front(spCommand);
	m_itCurrent = m_lstCommands.begin();	

	TRACE_INFO("Command Manager history list has " << m_lstCommands.size() << " commands");

	// notify all observers of command execution: it was successful
	sendNotification("");
}

/*!
Is there a command to be undone? Although this method is thread safe
it should not be relied upon for critical decisions since the result
is only valid for a single point in time.

\return	true if there is a command to be undone, otherwise false.
*/
bool CommandManager::canUndo() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	CommandList::const_iterator it(m_itCurrent);
	return (it != m_lstCommands.end());
}

/*!
Undo the most recent command.

\return true if the command is successfully undone, otherwise false.
*/
bool CommandManager::undoCommand()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool bResult = false;	

	if (canUndo())
	{
		PYXPointer< Command > spCommand = *m_itCurrent;
		assert((spCommand != 0) && "No command for undo.");

		std::string strName = spCommand->getLongName();
		std::string strNotificationMsg;

		try
		{
			TRACE_INFO("Undoing command: '" << spCommand->getLongName() << "'.");
			bResult = spCommand->undo();

			// the user didn't cancel out of the undo
			if (bResult)
			{
				++m_itCurrent;
			}
			return true;
		}
		catch(PYXCommandUndoException& e)
		{
			clearAllCommands();

			std::string strNotificationMsg = "Unable to undo command '" + 
				strName + "' for the following reason:\n" + 
				e.getFullErrorString();
		}
		catch(...)
		{
			clearAllCommands();

			if(spCommand->getLastException() == 0)
			{
				strNotificationMsg = "Unable to undo command '" + 
					strName + 
					"' for unknown reason.";
			}
			else
			{
				PYXCommandUndoException* undoException = 
					dynamic_cast<PYXCommandUndoException*>(
					spCommand->getLastException());

				if(undoException != 0)
				{
					strNotificationMsg = 
						"Unable to undo command '" + 
						strName + "' " + 
						"for the following reason:\n" + 
						undoException->getFullErrorString();
				}
				else
				{
					strNotificationMsg = "Unable to undo command '" + 
						strName + 
						"' for unknown reason.";
				}

				// clear the last exception
				spCommand->setLastException(0);
			}
		}

		// notify all observers of command undo, even though it failed.
		TRACE_ERROR(strNotificationMsg);			
		sendNotification(strNotificationMsg);
	}	
	return false;
}

/*! 
Return the name of the command at the top of the undo stack. The name that
is returned is only valid for the one point in time at which it is called.

\return The name of the command that is next to be undone or an empty string
		if no command is available to be redone.
*/
std::string CommandManager::getUndoCommandName() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::string strCommandName;
	if (canUndo())
	{
		assert(getUndoCommand() != 0);
		strCommandName = getUndoCommand()->getLongName();
	}
	return strCommandName;
}

/*!
Get the command to be undone.  This method is not thread safe and should
only be called from a method that is already locked.

\return	The current undo command or 0 if no undo command.
*/
PYXPointer< const Command > CommandManager::getUndoCommand() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	PYXPointer< Command > spUndoCommand;

	if (canUndo())
	{
		spUndoCommand = *m_itCurrent;
	}

	return spUndoCommand;
}

/*!
Get all the commands that can be undone.  This method is not thread safe and should
only be called from a method that is already locked.

\return	All the undoable commands or an empty vector.
*/
std::vector<PYXPointer<Command > > CommandManager::getUndoableCommands() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::vector<PYXPointer<Command > > vecUndoableCommands;
	CommandList::iterator iterator;

	for(iterator = m_itCurrent; 
		iterator != m_lstCommands.end(); ++iterator)
	{
		vecUndoableCommands.push_back(*iterator);
	}

	return vecUndoableCommands;
}

/*!
Get all the commands that can be redone.  This method is not thread safe and should
only be called from a method that is already locked.

\return	All the redoable commands or an empty vector.
*/
std::vector<PYXPointer<Command > > CommandManager::getRedoableCommands() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::vector<PYXPointer<Command > > vecRedoableCommands;

	if(m_itCurrent != m_lstCommands.begin())
	{
		CommandList::iterator iterator = m_itCurrent;		

		do
		{
			--iterator;
			vecRedoableCommands.push_back(*iterator);			
		} 
		while(iterator != m_lstCommands.begin());
	}

	return vecRedoableCommands;
}

/*! 
Is there a command to be redone? Although this method is thread safe
it should not be relied upon for critical decisions since the result
is only valid for a single point in time.

\return true if there is a command to be redone, otherwise false.
*/
bool CommandManager::canRedo() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	CommandList::const_iterator it(m_itCurrent);
	return	(it != m_lstCommands.begin());
}

/*!
Redo the most recent command. If there is a command available to be redone and
the operation fails then the command stack is cleared.
*/
bool CommandManager::redoCommand()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (canRedo())
	{
		--m_itCurrent;
		PYXPointer< Command > spCommand = *m_itCurrent;
		assert((spCommand != 0) && "No command for redo.");

		std::string strName = spCommand->getLongName();
		std::string strNotificationMsg;		

		try
		{
			TRACE_INFO("Redoing command: '" << strName << "'.");
			spCommand->redo();

			return true;

		}
		catch(PYXCommandRedoException& e)
		{
			// TODO[kabiraman]: we can probably get away with not doing this
			clearAllCommands();

			std::string strNotificationMsg = "Unable to redo command '" + 
				strName + "' for the following reason:\n" + 
				e.getFullErrorString();
		}
		catch(...)
		{
			// TODO[kabiraman]: we can probably get away with not doing this
			clearAllCommands();

			if(spCommand->getLastException() == 0)
			{
				strNotificationMsg = "Unable to redo command '" + 
					strName + 
					"' for unknown reason.";
			}
			else
			{
				PYXCommandRedoException* redoException = 
					dynamic_cast<PYXCommandRedoException*>(
					spCommand->getLastException());

				if(redoException != 0)
				{
					strNotificationMsg = 
						"Unable to redo command '" + 
						strName + "' " + 
						"for the following reason:\n" + 
						redoException->getFullErrorString();
				}
				else
				{
					strNotificationMsg = "Unable to redo command '" + 
						strName + 
						"' for unknown reason.";
				}

				// clear the last exception
				spCommand->setLastException(0);
			}
		}

		// notify all observers of command redo, even though it failed.
		TRACE_ERROR(strNotificationMsg);
		sendNotification(strNotificationMsg);
	}
	
	return false;
}

/*! 
Return the name of the command at the top of the undo stack. The name that
is returned is only valid for the one point in time at which it is called.

\return The name of the command that is next to be undone or an empty string
		if no command is available to be redone.
*/
std::string CommandManager::getRedoCommandName() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::string strCommandName;
	if (canRedo())
	{
		assert(getRedoCommand() != 0);
		strCommandName = getRedoCommand()->getLongName();
	}
	return strCommandName;
}

/*!
Get the command to be redone.

\return	The command to be redone or 0 if no redo command.
*/
PYXPointer< const Command > CommandManager::getRedoCommand() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	PYXPointer< Command > spRedoCommand;

	if (canRedo())
	{
		CommandList::const_iterator it = m_itCurrent;
		--it;
		spRedoCommand = *it;
	}

	return spRedoCommand;
}

/*!
Clear all of the commands that have been undone.  This is required when a
series of commands is undone and then a new command is added to the list.
*/
void CommandManager::clearUndone()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_lstCommands.erase(m_lstCommands.begin(), m_itCurrent);
}

/*!
Sends a notification to all observers.

\param strErrorMsg The error message to provide with the notification.
*/
void CommandManager::sendNotification(const std::string& strErrorMsg)
{
	PYXPointer<CommandManagerEvent> spEvent(
		CommandManagerEvent::create(strErrorMsg));	

	getNotifier()->notify(spEvent);	
}
