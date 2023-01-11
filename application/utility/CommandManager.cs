using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtilities
{
    /// <summary>
    /// Tests the CommandManager.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class CommandManagerTest
    {
        /// <summary>
        /// Test the normal operation of the CommandManager.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestNormalOperation()
        {
            // setup test commands
            TestCommand command1 = new TestCommand(1);
            TestCommand command2 = new TestCommand(2);
            TestCommand command3 = new TestCommand(3);

            CommandManager commandManager = CommandManager.Instance;
            NUnit.Framework.Assert.IsTrue(!commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanRedo);

            commandManager.ExecuteCommand(command1);
            NUnit.Framework.Assert.IsTrue(command1.ExecuteCount == 1);
            NUnit.Framework.Assert.IsTrue(command1.UndoCount == 0);
            NUnit.Framework.Assert.IsTrue(command1.RedoCount == 0);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 1);
            NUnit.Framework.Assert.IsTrue(commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetUndoCommand() == command1);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanRedo);

            commandManager.Undo();
            NUnit.Framework.Assert.IsTrue(command1.ExecuteCount == 1);
            NUnit.Framework.Assert.IsTrue(command1.UndoCount == 1);
            NUnit.Framework.Assert.IsTrue(command1.RedoCount == 0);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 1);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(commandManager.CanRedo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetRedoCommand() == command1);

            commandManager.Redo();
            NUnit.Framework.Assert.IsTrue(command1.ExecuteCount == 1);
            NUnit.Framework.Assert.IsTrue(command1.UndoCount == 1);
            NUnit.Framework.Assert.IsTrue(command1.RedoCount == 1);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 1);
            NUnit.Framework.Assert.IsTrue(commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetUndoCommand() == command1);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanRedo);

            commandManager.RecordCommand(command2);
            NUnit.Framework.Assert.IsTrue(command2.ExecuteCount == 0);
            NUnit.Framework.Assert.IsTrue(command2.UndoCount == 0);
            NUnit.Framework.Assert.IsTrue(command2.RedoCount == 0);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 1);

            commandManager.ExecuteCommand(command3);
            NUnit.Framework.Assert.IsTrue(command3.ExecuteCount == 1);
            NUnit.Framework.Assert.IsTrue(command3.UndoCount == 0);
            NUnit.Framework.Assert.IsTrue(command3.RedoCount == 0);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 3);

            NUnit.Framework.Assert.IsTrue(commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetUndoCommand() == command3);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanRedo);

            commandManager.Undo();
            NUnit.Framework.Assert.IsTrue(command3.ExecuteCount == 1);
            NUnit.Framework.Assert.IsTrue(command3.UndoCount == 1);
            NUnit.Framework.Assert.IsTrue(command3.RedoCount == 0);
            NUnit.Framework.Assert.IsTrue(TestCommand.GetLastCommand() == 3);
            NUnit.Framework.Assert.IsTrue(commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetUndoCommand() == command2);
            NUnit.Framework.Assert.IsTrue(commandManager.CanRedo);
            NUnit.Framework.Assert.IsTrue(commandManager.GetRedoCommand() == command3);

            commandManager.ClearAllCommands();
            NUnit.Framework.Assert.IsTrue(!commandManager.CanUndo);
            NUnit.Framework.Assert.IsTrue(!commandManager.CanRedo);
        }

        /// <summary>
        /// A command used for test purposes.
        /// </summary>
        /// <remarks>
        /// The TestCommand is used for testing the CommandManager. It provides
        /// methods for returning how many times each of its Execute, Undo and Redo
        /// have been called and for tracking the ID of the last command that was
        /// executed.
        /// </remarks>
        internal class TestCommand : Command
        {
            /// <summary>
            /// Constructor.
            /// </summary>
            /// <param name="commandID">The ID of this command.</param>
            public TestCommand(int commandID)
            {
                m_commandID = commandID;
            }

            /// <summary>
            /// The number of times the command has been executed.
            /// </summary>
            public int ExecuteCount
            {
                get
                {
                    return m_executeCount;
                }
            }

            /// <summary>
            /// The number of times the command has been undone.
            /// </summary>
             public int UndoCount
            {
                get
                {
                    return m_undoCount;
                }
            }

            /// <summary>
            /// The number of times the command has been redone.
            /// </summary>
            public int RedoCount
            {
                get
                {
                    return m_redoCount;
                }
            }

            /// <summary>
            /// The name of the command.
            /// </summary>
            public override string Name
            {
                get
                {
                    return "Test Command";
                }
            }

            /// <summary>
            /// This method increments the execute count and records this command
            /// as the last one executed.
            /// </summary>
            protected internal override void Execute()
            {
                ++m_executeCount;
                m_lastCommand = m_commandID;
            }

            /// <summary>
            /// This method increments the undo count and records this command
            /// as the last one executed.
            /// </summary>
            protected internal override void Undo()
            {
                ++m_undoCount;
                m_lastCommand = m_commandID;
            }

            /// <summary>
            /// This method increments the redo count and records this command
            /// as the last one executed.
            /// </summary>
            protected internal override void Redo()
            {
                ++m_redoCount;
                m_lastCommand = m_commandID;
            }

            /// <summary>
            /// The number of times this command has been executed.
            /// </summary>
            private int m_executeCount;

            /// <summary>
            /// The number of times this command has been undone.
            /// </summary>
            private int m_undoCount;

            /// <summary>
            /// The number of times this command has been redone.
            /// </summary>
            private int m_redoCount;

            /// <summary>
            /// The ID of this command.
            /// </summary>
            private int m_commandID;

            /// <summary>
            /// The ID of the last command executed, undone or redone.
            /// </summary>
            private static int m_lastCommand;

            /// <summary>
            /// Get the ID of the last command executed, undone or redone.
            /// </summary>
            /// <returns>The command ID.</returns>
            static public int GetLastCommand()
            {
                return m_lastCommand;
            }
        }
        
        /// <summary>
        /// Test the operation of the CommandManager under failure conditions.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestFailureOperation()
        {
            CommandManager commandManager = CommandManager.Instance;

            // verify the execute exception is handled by the command manager
            Command failCmd = new FailCommand(FailCommand.eFailType.Execute);
            try
            {
                commandManager.ExecuteCommand(failCmd);
                NUnit.Framework.Assert.IsTrue(false);
            }
            catch (InvalidOperationException)
            {
                // expected behaviour
            }

            NUnit.Framework.Assert.IsTrue(commandManager.CommandCount == 0);

            // verify the undo exception is handled by the command manager
            failCmd = new FailCommand(FailCommand.eFailType.Undo);
            commandManager.ExecuteCommand(failCmd);
            try
            {
                commandManager.Undo();
                NUnit.Framework.Assert.IsTrue(false);
            }
            catch (InvalidOperationException)
            {
                // expected behaviour
            }
            NUnit.Framework.Assert.IsTrue(commandManager.CommandCount == 0);

            // verify the redo exception is handled by the command manager
            failCmd = new FailCommand(FailCommand.eFailType.Redo);
            commandManager.ExecuteCommand(failCmd);
            commandManager.Undo();
            try
            {
                commandManager.Redo();
                NUnit.Framework.Assert.IsTrue(false);
            }
            catch (InvalidOperationException)
            {
                // expected behaviour
            }
            NUnit.Framework.Assert.IsTrue(commandManager.CommandCount == 0);
        }

        /// <summary>
        /// A command that throws exceptions.
        /// </summary>
        /// <remarks>
        /// This class should only ever be used for testing purposes.  The
        /// TestCommand throws an exception on any of the virtual methods in the
        /// base Command class.
        /// </remarks>
        internal class FailCommand : Command
        {
            /// <summary>
            /// Defines the method in which to throw an exception.
            /// </summary>
            public enum eFailType
            {
                Execute = 0,
                Undo,
                Redo
            }

            /// <summary>
            /// Default constructor.
            /// </summary>
            /// <param name="failType">The method on which to fail.</param>
            public FailCommand(eFailType failType)
            {
                m_failType = failType;
            }

            /// <summary>
            /// The name of the command.
            /// </summary>
            public override string Name
            {
                get
                {
                    return "Fail Command";
                }
            }

            /// <summary>
            /// Execute the command. Throw an exception if fail type is knExecute.
            /// </summary>
            protected internal override void Execute()
            {
                if (m_failType == eFailType.Execute)
                {
                    throw new InvalidOperationException("FailCommand exception.");
                }
            }

            /// <summary>
            /// Undo the command. Throw an exception if fail type is knUndo.
            /// </summary>
            protected internal override void Undo()
            {
                if (m_failType == eFailType.Undo)
                {
                    throw new InvalidOperationException("FailCommand exception.");
                }
            }

            /// <summary>
            /// Redo the command. Throw an exception if fail type is knRedo.
            /// </summary>
            protected internal override void Redo()
            {
                if (m_failType == eFailType.Redo)
                {
                    throw new InvalidOperationException("FailCommand exception.");
                }
            }

            /// <summary>
            /// The method in which to throw an exception.
            /// </summary>
            eFailType m_failType;
        }
    }

    /// <summary>
    /// Manages application commands.
    /// The CommandManager manages the application's command stack and provides undo
    /// and redo functionality. Under this design pattern, the application creates a
    /// command and passes it to the command manager for execution or recording. The
    /// CommandManager maintains a stack of the most recent commands.
    /// </summary>
    public sealed class CommandManager
    {
        /// <summary>
        /// The singleton instance of the command manager.
        /// </summary>
        private static CommandManager m_singleton;
        
	    /// <summary>
	    /// The singleton instance of the command manager.
	    /// </summary>
 	    public static CommandManager Instance
        {
            get
            {
                if (m_singleton == null)
                {
                    m_singleton = new CommandManager();
                }

                return m_singleton;
            }
        }

	    /// <summary>
	    /// Clear all commands in the command manager.
	    /// </summary>
 	    public void ClearAllCommands()
        {
            m_currentCommand = -1;
            m_commandStack.Clear();
        }

	    /// <summary>
	    /// Execute a command.
	    /// </summary>
	    /// <param name="c">The command.</param>
	    public void ExecuteCommand(Command c)
        {
            if (c == null)
            {
                throw new ArgumentNullException("c","Attempt to execute null command.");
            }
            try
            {
                c.Execute();
                RecordCommand(c);
            }
            catch (Exception e)
            {
                throw new InvalidOperationException(String.Format(
                    "Error executing command '{0}'.",
                    c.Name), e);
            }
        }

        /// <summary>
        /// The maximum number of commands that can exist on the command stack.
        /// </summary>
        private const int kmaxCommands = 100;

        /// <summary>
        /// The command stack.
        /// </summary>
        private System.Collections.Generic.List<Command> m_commandStack =
            new List<Command>();

        /// <summary>
        /// The index of the current command in the command stack or -1 if
        /// there is no current command.
        /// </summary>
        private int m_currentCommand = -1;

        /// <summary>
        /// Clear all commands that have been undone from the command stack.
        /// </summary>
        public void ClearUndone()
        {
            lock (this)
            {
                try
                {
                    m_commandStack.RemoveRange(
                        m_currentCommand + 1,
                        m_commandStack.Count - m_currentCommand - 1);
                }
                catch (ArgumentOutOfRangeException)
                {
                    // There might be no commands to remove. Don't worry.
                }
            }
        }

        /// <summary>
        /// Record a command without executing it. This is typically used for direct
        /// manipulation operations where the application responds as the user interacts
        /// with it (for example moving an object). Typically the interaction is performed
        /// directly and only the end result is recorded as a command. This facilitates
        /// undo and redo operations.
        /// </summary>
        /// <param name="c">The command.</param>
	    public void RecordCommand(Command c)
        {
            lock(this)
            {
	            // clear all commands that have been undone
                ClearUndone();

	            // ensure command list doesn't grow too large
                if (m_commandStack.Count >kmaxCommands)
                    m_commandStack.RemoveAt(0);

            	// add the command to the list
                m_commandStack.Add(c);
                ++m_currentCommand;
            }
        }

	    /// <summary>
	    /// Is there a command to be undone?
	    /// </summary>
	    public bool CanUndo
        {
            get
            {
                lock (this)
                {
                    return (GetUndoCommand() != null);
                }
            }
        }

        /// <summary>
        /// Get the current undo command.
        /// </summary>
        /// <returns>The current undo command or null if no undo command.</returns>
        internal Command GetUndoCommand()
        {
            lock (this)
            {
                try
                {
                    return m_commandStack[m_currentCommand];
                }
                catch (ArgumentOutOfRangeException)
                {
                    return null;
                }
            }
        }

        /// <summary>
        /// Get the current redo command.
        /// </summary>
        /// <returns>The current redo command or null if no redo command.</returns>
        internal Command GetRedoCommand()
        {
            lock (this)
            {
                try
                {
                    return m_commandStack[m_currentCommand + 1];
                }
                catch (ArgumentOutOfRangeException)
                {
                    return null;
                }
            }
        }

        /// <summary>
        /// Undo the most recent command.
        /// </summary>
	    public void Undo()
        {
            lock (this)
            {
                Command c = null;
                try 
                {
                    c = GetUndoCommand();
                    c.Undo();
                    --m_currentCommand;
                } 
                catch (NullReferenceException ex)
                {
                    ClearAllCommands();
                    throw new InvalidOperationException(
                        "Unable to find the command to undo.",
                        ex);
                }
                catch (Exception ex)
                {
		            // clean up memory and re-throw
                    ClearAllCommands();
                    throw new InvalidOperationException(String.Format(
                        "Error undoing command '{0}'.",
                        c.Name), ex);
                }
            }
        }

        /// <summary>
        /// The name of the command at the top of the undo stack.
        /// </summary>
	    public String NameOfUndoCommand
        {
            get
            { 
                return GetUndoCommand().Name;
            }
        }

        /// <summary>
        /// Is there a command to be redone?
        /// </summary>
	    public bool CanRedo
        {
            get
            {
                lock (this)
                {
                    return GetRedoCommand() != null;
                }
            }
        }

        /// <summary>
        /// Redo the most recently undone command.
        /// </summary>
	    public void Redo()
        {
            lock (this)
            {
                Command c = null;
                try
                {
                    c = GetRedoCommand();
                    c.Redo();
                    ++m_currentCommand;
                }
                catch (NullReferenceException ex)
                {
                    ClearAllCommands();
                    throw new InvalidOperationException(
                        "Unable to find the command to redo.",
                        ex);
                }
                catch (Exception ex)
                {
		            // clean up memory and re-throw
                    ClearAllCommands();
                    throw new InvalidOperationException(String.Format(
                        "Error redoing command '{0}'.",
                        c.Name), ex);
                }
            }
        }

 
        /// <summary>
        /// The name of the command at the top of the redo stack.
        /// </summary>
	    public String NameOfRedoCommand
        {
            get
            {
                return GetRedoCommand().Name;
            }
        }

	    /// <summary>
	    /// The number of commands in the command stack.
	    /// </summary>
	    internal int CommandCount
        { 
            get
            { 
                return m_commandStack.Count;
            }
        }
    }
}
