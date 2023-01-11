/******************************************************************************
ManagedCommand.cs

begin      : November 5, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

//#define DIAGNOSTIC_TRACE

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// This command wraps up the logic needed to make a SWIG director
    /// behave nicely when passed back to the unmanaged (and reference
    /// counted) world.  Unfortunately, there appears to be no way to 
    /// integrate this logic into the SWIG-generated Command delegate.
    /// </summary>
    public class ManagedCommand : Command, IDirectorReferenceCounter
    {

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero) 
            {
                return 1;
            }
            return ReferenceManager.release(this);
        }

        #endregion

        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <remarks>
        /// Clients should call other constructors.
        /// </remarks>        
        private ManagedCommand()
        {
        }

        #region Default Implementation
        
        protected ManagedCommand(string name, int priority)
            : this()
        {
            this.Name = name;
            this.Priority = priority;
        }

        protected ManagedCommand(string name, string category,
            int priority)
            : this(name, priority)
        {
            this.Category = category;
        }

        /// <summary>
        /// The command's name.
        /// </summary>
        public string Name
        { get; set; }

        public override string getName()
        {
            return this.Name;
        }

        /// <summary>
        /// The command's priority.
        /// </summary>
        public int Priority
        { get; set; }

        public override int getPriority()
        {
            return this.Priority;
        }

        public string Category { get; set; }
        
        public override string getCategory()
        {
            return this.Category;
        }

        public override void setCategory(string category)
        {
            this.Category = category;
        }

        #endregion Default Implementation


    }

    /// <summary>
    /// A ManagedCommand that is undoable.
    /// </summary>
    public class UndoableManagedCommand : ManagedCommand
    {
        public UndoableManagedCommand(string name, string category,
            int priority)
            : base(name, category, priority)
        {            
        }

        public override bool getRecordable()
        {
            return true;
        }

        public override bool getUndoable()
        {
            return true;
        }
    }

    namespace Test
    {
        using NUnit.Framework;        
        
        /// <summary>
        /// A dummy command for testing purposes.
        /// </summary>
        internal class DummyTestCommand : ManagedCommand
        {
            CommandTreeTestContext m_context;

            public DummyTestCommand():
                base( "Test", "DummyCategory", 0)
            {
            }

            public DummyTestCommand(string name, string category, 
                int priority, CommandTreeTestContext context):
                base( name, category, priority)
            {
                m_context = context;
            }
        }        

        /// <summary>
        /// A dummy command for testing purposes.
        /// </summary>
        internal class DummyTestSubCommand : ManagedCommand
        {
            public DummyTestSubCommand():
                base( "Sub", "DummyCategory", 0)
            {
            }
        }


        /// <summary>
        /// Unit tests for ManagedCommand
        /// </summary>
        //[TestFixture] // Disabled because euclid dies on this unit test.
        public class ManagedCommandTests
        {
            /// <summary>
            /// This test checks to see the assigning of subordinates to 
            /// commands and the retrieving of these subordinates works 
            /// without errors, specifically ensuring that our Command, 
            /// Command_SPtr and Vector_Command objects are not being 
            /// prematurely GCed or destroyed in C++.
            /// </summary>
            //[Test] // Disabled because euclid dies on this unit test.
            public void LifeTimeManagement()
            {
                // we're using a menu item so as to mimic the functionality of 
                // saving a Command_SPtr in the Tag property
                System.Windows.Forms.ToolStripMenuItem item =
                    new System.Windows.Forms.ToolStripMenuItem();

                Command_SPtr command =
                    new Command_SPtr(new DummyTestCommand());

                command.addSubordinate(new Command_SPtr(
                    new DummyTestSubCommand()));

                item.Tag = command.getSubordinates()[0];

                for (int i = 0; i < 1000; ++i)
                {
                    GC.GetTotalMemory(true);
                    GC.WaitForPendingFinalizers();
                }

                Command_SPtr subordinate = item.Tag as Command_SPtr;                
                
                // checking to make sure the Vector_Command and the 
                // Command_SPtr objects in them aren't GCed
                int count = subordinate.getSubordinates().Count;

            }            
        }
    }
}
