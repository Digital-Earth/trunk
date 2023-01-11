#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H
/******************************************************************************
command_manager.h

begin		: 2005-03-30
copyright	: (C) 2005 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/pointer.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <list>
#include <vector>

// local forward declarations
class Command;

//! Event sent to observers when command state changes.
class CommandManagerEvent : public NotifierEvent
{
public:

	//! Creator
	static PYXPointer<CommandManagerEvent> create(const std::string& strErrorMsg)
	{
		return PYXNEW(CommandManagerEvent, strErrorMsg);
	}

	std::string getErrorMsg()
	{
		return m_strErrorMsg;
	}

protected:

	//! Construct a new event.
	explicit CommandManagerEvent(const std::string& strErrorMsg) : 
		m_strErrorMsg(strErrorMsg)
	{
	}

private:

	//! The error message, if any, to provide when raising the event.
	std::string m_strErrorMsg;
};

/*!
The CommandManager manages the application's command stack and provides undo
and redo functionality. Under this design pattern, the application creates a
command and passes it to the command manager for execution or recording. The
CommandManager maintains a stack of the most recent commands.

\sa Command
*/
//! Manages application commands.
class PYXLIB_DECL CommandManager
{
public:

	//! Test method
	static void test();

	//! Default Constructor
	CommandManager();

	//! Destructor
	virtual ~CommandManager();

	//! Clear all commands in the command manager.
	void clearAllCommands();

	//! Execute a command.
	bool executeCommand(PYXPointer< Command > spCommand);

	//! Record a command.
	void recordCommand(PYXPointer< Command > spCommand);

	//! Undo the most recent command.
	bool undoCommand();

	//! Redo the most recent command.
	bool redoCommand();

	//! Is there a command to be undone?
	bool canUndo() const;

	//! Return the name of the command at the top of the undo stack.
	std::string getUndoCommandName() const;

	//! Is there a command to be redone?
	bool canRedo() const;

	//! Return the name of the command at the top of the redo stack.
	std::string getRedoCommandName() const;

	//! Get all the commands that can be undone.
	std::vector<PYXPointer<Command > > getUndoableCommands() const;

	//! Get all the commands that can be redone.
	std::vector<PYXPointer<Command > > getRedoableCommands() const;

	//! Get the number of commands in the list
	inline int size() const {return static_cast<int>(m_lstCommands.size());}

	//! Sends a notification to all observers.
	void sendNotification(const std::string& strErrorMsg);

	//! Return the notifier associated with the Command Manager.
	Notifier* getNotifier()
	{
		return &m_notifier;
	}

private:

	//! Disable copy constructor
	CommandManager(const CommandManager&);

	//! Disable copy assignment
	void operator=(const CommandManager&);

	//! Get the command to be undone.
	PYXPointer< const Command > getUndoCommand() const;	

	//! Get the command to be redone.
	PYXPointer< const Command > getRedoCommand() const;

	//! Clear all commands that have been undone
	void clearUndone();	

private:

	//! Typedef for a list of commands
	typedef std::list< PYXPointer< Command > > CommandList;	

private:

	//! Thread protection.
	mutable boost::recursive_mutex m_mutex;

	//! The command list
	CommandList m_lstCommands;

	//! The current command
	CommandList::iterator m_itCurrent;

	//! The notification object for the class
	Notifier m_notifier;

	/*! 
	Indicates whether a command is currently in the middle of execution and 
	if so, stops other commands from executing.
	 */
	bool m_bIsExecuting;	
};

#endif // guard
