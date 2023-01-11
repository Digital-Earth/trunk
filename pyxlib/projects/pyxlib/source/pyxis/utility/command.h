#ifndef COMMAND_H
#define COMMAND_H
/******************************************************************************
command.h

begin		: 2005-03-30
copyright	: (C) 2005 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxis library includes
#include "pyxis/pipe/process.h"

#include "pyxis/utility/exception.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <string>
#include <vector>

// forward declarations
class PYXCommandException;

//! An event that a command should raise once it completes execution on a non-UI thread.
class PYXLIB_DECL CommandExecutedEvent : public NotifierEvent
{
public:

	static PYXPointer<CommandExecutedEvent> create(ProcRef procRef)
	{
		return PYXNEW(CommandExecutedEvent, procRef);
	}

	//! Getter for ProcRef.
	ProcRef getProcRef()
	{
		return m_procRef;
	}

private:

	explicit CommandExecutedEvent(ProcRef procRef) : 
		m_procRef(procRef)
	{
	}

	//! The ProcRef of the pipeline the command executes on.
	ProcRef m_procRef;
};

/*!
Command is the abstract base for all classes that perform application commands.
Commands are typically invoked from menu items, upon completion of dialogs or
as the result of manipulation operations. A command must contain all logic and
data necessary to execute, undo and redo the command. Commands should be passed
to the CommandManager for execution. An exception should be thrown if the
command fails to execute/undo/redo.

\sa CommandManager
*/
//! Abstract base class for application commands.
class PYXLIB_DECL Command : public PYXObject
{
// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Destructor
	virtual ~Command();

	/*!
	The command name should be a short string suitable for use in an undo/redo
	menu item. For example Undo "Change Font" or Redo "Change Font".
	*/
	virtual std::string getName() const = 0;

	/*!
	This is specifically for display when hovering the mouse over the 
	Undo/Redo buttons.  By default, it calls getName().
	*/
	virtual std::string getLongName() const
	{
		return getName();
	}

	//! Execute the command.  Returns whether the command was executed or whether 
	//! the user cancelled out of it.
	virtual bool execute() = 0;
	
	//! Drag the button. The default implementation returns empty string.
	virtual std::string drag() { return ""; }
	
	//! Undo the command.  Returns whether the command was undone or whether 
	//! the user cancelled out of it.
	virtual bool undo() = 0;

	//! Redo the command. The default implementation calls execute().  Returns 
	//! whether the command was redone or whether the user cancelled out of it.
	virtual bool redo() 
	{
		return execute();
	}	

	//! The category that the command belongs to. IE: "Open", "Save", etc.
	virtual std::string getCategory() const;

	virtual void setCategory(const std::string& strCategory);

	//! The command's priority within its category. The higher the number, 
	//! the more important it is.  Default is 0.
	virtual int getPriority() const
	{
		return 0;
	}

    //! Set the priority of this command.
	virtual void setPriority( int priority) 
	{
	}

	//! The command's subordinate commands (all belonging to the same category), 
	//! ordered by priority (ascending).
	virtual std::vector<PYXPointer<Command> > &getSubordinates() ;

	//! Assign a subordinate.
	virtual void addSubordinate(PYXPointer<Command> spCommand);

	//! Whether the command requires to be put in the Command Stack.
	virtual bool getRecordable() const
	{
		return false;
	}

	//! Whether the command is undoable at the point of time this method is called.
	virtual bool getUndoable() const
	{
		return false;
	}

	//! Whether the command is enabled at the point of time this method is called.
	virtual bool Command::getEnabled() const
	{
		return true;
	}
	
	//! Whether the command is checked at the point of time this method is called.
	virtual bool Command::getChecked() const
	{
		return false;
	}

	//! The list of possible command types.
	enum CommandType
	{
		knComboButton,
		knCheckBox,
		knMenuItem,
		knMenuSeparator, 
		knUnspecified
	};	
	
	//! The command type, i.e. whether ComboButton, checkbox.  Default is 
	//! knUnspecified.
	virtual CommandType getType() const
	{
		return knUnspecified;
	}

	//! The command's Tooltip text.  By default, it returns the name.
	virtual std::string getToolTip() const
	{
		return getName();
	}

	//! Whether the command has any subordinates.
	virtual bool hasSubordinates() const
	{
		return !m_vecSubordinates.empty();
	}

	/*! 
	Returns the last exception the command encountered.

	\return The last exception the command encountered.
	*/
	PYXCommandException* getLastException()
	{
		return m_pLastException;
	}

	/*! 
	Sets the last exception the command encountered.

	\param	pLastException	The last exception the command encountered.
	*/
	void setLastException(PYXCommandException* pLastException)
	{
		m_pLastException = pLastException;
	}

	//! Return the 'Executed' notifier.
	Notifier& Executed()
	{
		return m_executed;
	}

	/*! 
	A command must call this method to raise the CommandExecutedEvent.

	\param	procRef	The ProcRef of the pipeline the command executed on.
	*/
	void notifyCommandExecuted(ProcRef procRef)
	{
		m_executed.notify(CommandExecutedEvent::create(procRef));
	}

protected:

	//! Default constructor
	Command() : m_executed("Command Execution Complete.") 
	{
		m_pLastException = 0;
	}	
	
private:

	//! Disable copy constructor
	Command(const Command&);

	//! Disable copy assignment
	void operator=(const Command&);

private:

	//! The command's category.
	std::string m_strCategory;

	//! The command's subordinate commands.
	std::vector<PYXPointer<Command> > m_vecSubordinates;

	//! The last exception encountered by a command.
	PYXCommandException* m_pLastException;

	//! Used to raise the CommandExecutedEvent when a command has completed 
	//! execution on a non-UI thread.
	Notifier m_executed;
};

#endif // guard
