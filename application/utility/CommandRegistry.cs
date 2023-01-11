/******************************************************************************
CommandRegistry.cs

begin      : October 19, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    #region Interfaces

    /// <summary>
    /// A Command Factory is used to create a set of commands for a given 
    /// context.
    /// </summary>
    /// <remarks>
    /// See https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=User_Story/Command_Infrastructure"
    /// </remarks>
    /// <seealso ref="SimpleCommandFactory"/>
    /// <example>
    /// /// Sample test factory - associates the command "MyCommand" with the
    /// /// context "MyContext".
    /// class MyTestFactory : ICommandFactory
    /// {
    ///    public IEnumerable&lt;ICommandInstance&gt; GetCommands(ICommandContext context)
    ///    {
    ///        if (context is MyTestContext)
    ///        {
    ///            yield return new CommandInstance( new MyTestCommand(), 100);
    ///        }
    ///    }
    /// }
    /// </example>
    public interface ICommandFactory
    {
        /// <summary>
        /// Gets the commands.
        /// </summary>
        /// <param name="context">The context.</param>
        /// <returns></returns>
        IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context);
    }

    #endregion /* Interfaces */

    /// <summary>
    /// The Command Registry serves as a single repository of all available 
    /// Command-Factories.  The registry will return an ordered list of 
    /// command-instances corresponding to any given command-context.
    /// </summary>
    /// <remarks>
    /// See https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=User_Story/Command_Infrastructure"
    ///</remarks>
    public class CommandRegistry
    {
        /// <summary>
        /// The one and only Command Registry.
        /// </summary>
        public static CommandRegistry GlobalCommandRegistry
        {
            get { return CommandRegistry.m_globalCommandRegistry; }
        }   
        private static CommandRegistry m_globalCommandRegistry =
            new CommandRegistry();

        /// <summary>
        /// Maintains the list of registered command factories.
        /// </summary>
        private List<ICommandFactory> m_factories =
            new List<ICommandFactory>();

        /// <summary>
        /// Registers the command factory.
        /// </summary>
        /// <param name="commandFactory">The command factory.</param>
        public void RegisterCommandFactory(ICommandFactory commandFactory)
        {
            m_factories.Add(commandFactory);
        }        

        /// <summary>
        /// Gets all commands for the given context.
        /// </summary>
        /// <param name="context">The context.</param>
        /// <returns>The list of all parent commands.</returns>
        public List<Command_SPtr> GetCommands(Pyxis.Utilities.ICommandContext context)
        {
            List<Command> availableCommands = new List<Command>();

            foreach (ICommandFactory f in m_factories)
            {
                // The call to factory.GetCommands(context) will return null 
                // if the context we pass in doesn't apply to the command we 
                // want.  Therefore, only commands with the appropriate 
                // context will be added to availableCommands.
                availableCommands.AddRange(f.GetCommands(context));
            }            

            return SortCommands(availableCommands);
        }

        /// <summary>
        /// Sorts the given commands, grouping them by category, then 
        /// ordering them within category by priority.
        /// </summary>
        /// <param name="list">A list of all commands.</param>
        /// <returns>A list of all parent commands whose subordinate 
        /// associations have been set.</returns>
        private List<Command_SPtr> SortCommands(List<Command> list)
        {
            // Step 1: group commands by category, and in the category, 
            // order by priority.
            var commandList = new Dictionary<string, List<Command>>();
            foreach (Command c in list)
            {
                if (commandList.ContainsKey(c.getCategory()))
                {
                    commandList[c.getCategory()].Add(c);
                }
                else
                {
                    List<Command> newList = new List<Command>();
                    newList.Add(c);
                    commandList.Add(c.getCategory(), newList);
                }
            }

            // Step 2: Assign all commands their subordinates in the correct 
            // order (by priority) and compile a list of all parent commands.
            List<Command_SPtr> resultList = new List<Command_SPtr>();

            foreach (List<Command> item in commandList.Values)
            {
                try
                {
                    item.Sort(
                        (l, r) => (l.getPriority() == r.getPriority())
                                      ? l.getName().CompareTo(r.getName())
                                      : l.getPriority().CompareTo(r.getPriority()));
                }
                catch (InvalidOperationException ex)
                {
                    Trace.error(string.Format(
                        "Error when sorting commands (ignored): {0}", ex.Message));
                }


                Command_SPtr currentSublist = null;
                int commandCount = item.Count - 1;

                for (int i = commandCount; i >= 0; --i)
                {
                    Command_SPtr command = new Command_SPtr(item[i]);
                    
                    if (currentSublist == null)
                    {
                        currentSublist = command;
                    }
                    else
                    {
                        currentSublist.addSubordinate(command);
                    }

                    if (i == commandCount)
                    {
                        resultList.Add(currentSublist);
                    }
                }
            }

            //resultList.Sort((l, r) => (l.getPriority() == r.getPriority())
            //                              ? l.getName().CompareTo(r.getName())
            //                              : l.getPriority().CompareTo(r.getPriority()));

            /* Step 3: Step 2 assigned commands their subordinates within the 
             *  same category.  This step assigns commands their subordinates 
             * across categories.  For e.g. assigning the parent command in 
             * category 'PipelineCommands/Export' as suborindate to the 
             * parent command in category 'PipelineCommands'.
            */

            // list of child commands across categories
            List<Command_SPtr> childCommands = new List<Command_SPtr>();

            foreach (Command_SPtr childCommand in resultList)
            {
                string[] categories =
                    childCommand.getCategory().Split(new char[] { '/' });

                if (categories.GetLength(0) > 1)
                {
                    string childCategory = 
                        categories[categories.GetLength(0) - 2];

                    foreach (Command_SPtr parentCommand in resultList)
                    {
                        string[] cats = 
                            parentCommand.getCategory().Split(
                            new char[] { '/' });
                        string parentCategory = cats[cats.GetLength(0) - 1];

                        if (childCategory == parentCategory)
                        {
                            parentCommand.addSubordinate(childCommand);
                            childCommands.Add(childCommand);
                            break;
                        }
                    }
                }
            }

            // remove the child commands found across categories from the 
            // list of the top-level commands to be returned
            foreach (Command_SPtr c in childCommands)
            {
                resultList.Remove(c);
            }

            return resultList;
        }

        /// <summary>
        /// Using the category, finds a command's parent and assigns the 
        /// command to its parent.
        /// </summary>
        /// <param name="targetCommand">The command to check to see if it, or it's 
        /// subordinate is the parent.</param>
        /// <param name="category">The child command's category.</param>
        /// <param name="sourceCommand">The command to find the parent for.</param>
        private void FindParent(Command_SPtr targetCommand,
            string category, Command_SPtr sourceCommand)
        {
            foreach (Command_SPtr childCommand in targetCommand.getSubordinates())
            {
                FindParent(childCommand, category, sourceCommand);
            }

            if (category ==
                targetCommand.getName())
            {
                targetCommand.addSubordinate(sourceCommand);
                return;
            }
        }        

#if false // This code is for debugging the command tree.
        /// <summary>
        /// Prints the command tree for every command in the list.  Each tree 
        /// is printed bottom up, and lower priority children are listed 
        /// before higher priority children.
        /// </summary>
        /// <param name="resultList">The list of parent commands.</param>
        private void PrintCommandTree(List<Command_SPtr> resultList)
        {
            foreach (Command_SPtr command in resultList)
            {
                PrintCommandTree(command);

            }
        }

        /// <summary>
        /// Prints the command tree for every command in the list.  Each tree 
        /// is printed bottom up, and lower priority children are listed 
        /// before higher priority children.
        /// </summary>
        /// <param name="command"></param>
        private void PrintCommandTree(Command_SPtr command)
        {
            foreach (Command_SPtr childCommand in command.getSubordinates())
            {
                PrintCommandTree(childCommand);
            }
            Trace.debug(command.getName());
        }
#endif
    }

    #region Helper classes, default implementations.

    /// <summary>
    /// Generic command factory.  Generates the specified command for the 
    /// given context.  Priority can also be specified.
    /// </summary>
    /// <typeparam name="Cmd"></typeparam>
    /// <typeparam name="Context"></typeparam>
    public class SimpleCommandFactory<Cmd, Context> : ICommandFactory
        where Cmd : Command, new()
        where Context : Pyxis.Utilities.ICommandContext
    {
        private int m_priority = 0;

        /// <summary>
        /// Construct with the specific priority.
        /// </summary>
        /// <param name="priority">
        /// The priority assigned to each generated CommandInstance.
        /// </param>
        public SimpleCommandFactory(int priority)
        {
            m_priority = priority;
        }

        /// <summary>
        /// Default constructor (priority defaults to 0.)
        /// </summary>
        public SimpleCommandFactory()
        {
        }

        #region ICommandFactory Members

        public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
        {
            if (context is Context)
            {
                yield return new Cmd();
            }
        }

        #endregion /* ICommandFactory Members */
    }

    /// <summary>
    /// This class 
    /// </summary>
    /// <typeparam name="CommandClass"></typeparam>
    public class AutoCommandFactory<CommandClass> : ICommandFactory
        where CommandClass : ManagedCommand
    {
        private int m_priority = 0;

        /// <summary>
        /// A delegate that allows the creator of an AutoCommandFactory for a 
        /// Command to specify a condition that needs to be tested when 
        /// performing a GetCommands().
        /// </summary>
        /// <param name="context">
        /// The Command Context of the command the condition is for.
        /// </param>
        /// <returns>Whether the condition is met or not.</returns>
        public delegate bool Condition(Pyxis.Utilities.ICommandContext context);

        /// <summary>
        /// An instance of a condition.
        /// </summary>
        Condition m_condition;

        /// <summary>
        /// Default constructor (priority defaults to 0.)
        /// </summary>
        public AutoCommandFactory()
        {
        }
        
        /// <summary>
        /// Construct with the specific priority.
        /// </summary>
        /// <param name="priority">
        /// The priority assigned to each generated CommandInstance.
        /// </param>
        public AutoCommandFactory(int priority)
        {
            m_priority = priority;
        }

        /// <summary>
        /// Construct with a condition.
        /// </summary>
        /// <param name="condition">
        /// The condition to test when performing a GetCommands().
        /// </param>
        public AutoCommandFactory(Condition condition)
        {
            m_condition = condition;            
        }

        #region ICommandFactory Members

        /// <summary>
        /// Determines whether the given object (o) instance can cast to the specified type (t).
        /// </summary>
        /// <param name="o">The object to cast.</param>
        /// <param name="t">The type to cast to.</param>
        /// <returns>
        /// 	<c>true</c> if this instance can cast the specified o; otherwise, <c>false</c>.
        /// </returns>
        private static bool CanCast(object o, Type t)
        {
            if (t.IsInterface)
            {
                return o.GetType().GetInterface(t.FullName) != null;
            }
            else
            {
                Type oBase = o.GetType();
                while (oBase != null)
                {
                    if (oBase.Equals(t))
                    {
                        return true;
                    }
                    oBase = oBase.BaseType;
                }
                return false;
            }
        }

        /// <summary>
        /// Gets all the commands that correspond to the given context.
        /// </summary>
        /// <param name="context">The context.</param>
        /// <returns></returns>
        public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
        {
            // TODO: Consider factoring this into a property, so it can be cached.
            Type targetType = typeof(CommandClass);
            foreach (System.Reflection.ConstructorInfo c in targetType.GetConstructors())
            {
                List<object> parameters = new List<object>();

                foreach (System.Reflection.ParameterInfo p in c.GetParameters())
                {
                    if (CanCast(context, p.ParameterType))
                    {
                        parameters.Add(context);
                    }
                    else
                    {
                        parameters = null;
                        break;
                    }
                }

                if (parameters != null)
                {
                    Command result = null;
                    try
                    {
                        // if the condition is met
                        if ((m_condition == null || (m_condition(context))))
                        {
                            result = c.Invoke(parameters.ToArray()) as Command;
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new Exception(
                            "An exception occurred while attempting to retreive commands!", 
                            ex.InnerException);
                    }

                    if (result != null)
                    {
                        //result.setPriority(m_priority);
                        yield return result;

                        // Only let the "first" version through.  Ideally, this 
                        // should be the one with the most matched params.
                        yield break;
                    }
                }
            }
        }

        #endregion /* ICommandFactory Members */
    }

    #endregion /* Helper classes, default implementations. */
    
    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// A context for test purposes.
        /// </summary>
        class FirstTestContext : Pyxis.Utilities.ICommandContext { }

        /// <summary>
        /// A context for test purposes.
        /// </summary>
        class SecondTestContext : Pyxis.Utilities.ICommandContext { }

        /// <summary>
        /// A context specific for the CommandTree unit test.
        /// </summary>
        class CommandTreeTestContext : Pyxis.Utilities.ICommandContext { }

        class AutoCommand : ManagedCommand
        {
            public AutoCommand(FirstTestContext param):
                base( "AutoCommand", "Dummy Category", 0)
            {
            }
        }

        /// <summary>
        /// Unit tests for the CommandRegistry.
        /// </summary>
        [TestFixture]
        public class CommandRegistryTests
        {
            [Test]  
            public void SimpleTest()
            {
                CommandRegistry registry = new CommandRegistry();
                Assert.AreEqual(0, registry.GetCommands(new FirstTestContext()).Count,
                    "Registry must be empty.");
                Assert.AreEqual(0, registry.GetCommands(new SecondTestContext()).Count,
                    "Registry must be empty.");

                // Create and register a command-factory.
                var LocalTestFactory =
                    new SimpleCommandFactory<DummyTestCommand, FirstTestContext>();
                registry.RegisterCommandFactory(LocalTestFactory);                

                // Check to ensure that the command-factory returns the command.
                Assert.AreEqual(1, registry.GetCommands(new FirstTestContext()).Count,
                    "This context must match the factory.");
                Assert.AreEqual(0, registry.GetCommands(new SecondTestContext()).Count,
                    "This context must not match the factory.");
            }

            [Test]
            public void AutoFactoryTest()
            {
                var factory = new AutoCommandFactory<AutoCommand>();
                IEnumerable<Command> singleItemList = factory.GetCommands(new FirstTestContext());
                Assert.IsNotNull(singleItemList);

                CommandRegistry registry = new CommandRegistry();
                Assert.AreEqual(0, registry.GetCommands(new FirstTestContext()).Count,
                    "Registry must be empty.");
                Assert.AreEqual(0, registry.GetCommands(new SecondTestContext()).Count,
                    "Registry must be empty.");

                // Create and register a command-factory.
                registry.RegisterCommandFactory( new AutoCommandFactory<AutoCommand>());

                // Check to ensure that the command-factory returns the command.
                Assert.AreEqual(1, registry.GetCommands(new FirstTestContext()).Count,
                    "This context must match the factory.");
                Assert.AreEqual(0, registry.GetCommands(new SecondTestContext()).Count,
                    "This context must not match the factory.");
            }

            [Test]
            public void CommandTreeTest()
            {
                CommandRegistry registry = new CommandRegistry();
                
                /* 
                 * register commands with the following tree (commands can be 
                 * registered in any order):
                 *              Command 1
                 *              /       \
                 *      Command 1.1     Command 1.2
                 *          /       \
                 *  Command 1.1.1   Command 1.1.2
                 */
                registry.RegisterCommandFactory(new Command12CommandFactory());
                registry.RegisterCommandFactory(new Command1CommandFactory());
                registry.RegisterCommandFactory(new Command11CommandFactory());
                registry.RegisterCommandFactory(new Command112CommandFactory());
                registry.RegisterCommandFactory(new Command111CommandFactory());
                                
                List<Command_SPtr> topLevelCommands = registry.GetCommands(
                    new CommandTreeTestContext());

                Assert.AreEqual(1, topLevelCommands.Count, "There should be " 
                     + "only 1 top level command");

                Command_SPtr command = topLevelCommands[0];

                Assert.AreEqual("Command 1", command.getName(),
                    "The one and only top level command must be Command 1.");

                Assert.AreEqual(2, command.getSubordinates().Count, 
                    "Command 1 must have exactly 2 subordinates.");

                Assert.AreEqual("Command 1.1", 
                    command.getSubordinates()[0].getName(),
                    "Command 1's 1st subordinate must be Command 1.1.");

                Assert.AreEqual("Command 1.2",
                    command.getSubordinates()[1].getName(),
                    "Command 1's 2nd subordinate must be Command 1.2.");

                Assert.AreEqual(2,
                    command.getSubordinates()[0].getSubordinates().Count,
                    "Command 1.1 must have 2 subordinates.");

                Vector_Command subordinates = 
                    command.getSubordinates()[0].getSubordinates();

                Assert.AreEqual("Command 1.1.1",
                    subordinates[0].getName(),
                    "Command 1.1's 1st subordinate must be Command 1.1.1.");

                Assert.AreEqual("Command 1.1.2",
                    subordinates[1].getName(),
                    "Command 1.1's 2nd subordinate must be Command 1.1.2.");

                Assert.AreEqual(0,
                    subordinates[0].getSubordinates().Count,
                    "Command 1.1.1 must have no subordinates.");

                Assert.AreEqual(0,
                    subordinates[1].getSubordinates().Count,
                    "Command 1.1.2 must have no subordinates.");

                Assert.AreEqual(0,
                    command.getSubordinates()[1].getSubordinates().Count,
                    "Command 1.2 must have no subordinates.");
            }
        }

        #region CommandTreeTest supporting classes

        class Command1CommandFactory : ICommandFactory
        {
            public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
            {
                yield return new DummyTestCommand("Command 1", "SomeCategory", 
                    1, new CommandTreeTestContext());
            }
        }

        class Command11CommandFactory : ICommandFactory
        {
            public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
            {
                yield return new DummyTestCommand("Command 1.1", 
                    "SomeCategory/1.1", 0, new CommandTreeTestContext());
            }
        }

        class Command12CommandFactory : ICommandFactory
        {
            public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
            {
                yield return new DummyTestCommand("Command 1.2",
                    "SomeCategory", -1, new CommandTreeTestContext());
            }
        }

        class Command111CommandFactory : ICommandFactory
        {
            public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
            {
                yield return new DummyTestCommand("Command 1.1.1",
                    "SomeCategory/1.1", -1,
                    new CommandTreeTestContext());
            }
        }

        class Command112CommandFactory : ICommandFactory
        {
            public IEnumerable<Command> GetCommands(Pyxis.Utilities.ICommandContext context)
            {
                yield return new DummyTestCommand("Command 1.1.2",
                    "SomeCategory/1.1", -2,
                    new CommandTreeTestContext());
            }
        }

        #endregion
    }
}
