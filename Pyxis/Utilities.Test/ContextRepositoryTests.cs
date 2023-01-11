using System;

namespace Pyxis.Utilities.Test
{
    using NUnit.Framework;
    using System.Text;

    [TestFixture]
    public class ContextRepositoryTests 
    {
        Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        #region derived context for testing

        class TestContext : Pyxis.Utilities.ContextRepository
        {
            static private TestContext m_ctx = new TestContext();

            public TestContext()
            {
                HostCtx = this;
            }

            static private TestContext HostCtx
            {
                get { return m_ctx; }
                set { m_ctx = value; }
            }

            static public void Load()
            {
                HostCtx.LoadContext();
            }

            static public void Save()
            {
                HostCtx.SaveContext();
            }
                       
            static public void Reset()
            {
                HostCtx.Init();
            }

            #region TestContext Properties

            static public string StackName
            {
                get { return ((string)(HostCtx["StackName"])); }
                set { HostCtx["StackName"] = value; }
            }

            static public int MilliSeconds
            {
                get { return ((int)(HostCtx["MilliSeconds"])); }
                set { HostCtx["MilliSeconds"] = value; }
            }

            static public int Spinach
            {
                get { return ((int)(HostCtx["spinach"])); }
                set { HostCtx["spinach"] = value; }
            }

            #endregion

            protected override void OnLoad()
            {
                //--
                //-- Spinach is not loaded intentionally.
                //--
                MilliSeconds = 123;
                StackName = "basic-stack-name";
            }

            protected override void OnSave()
            {
            }
        }

        #endregion

        [Test]
        public void SimpleTest()
        {
            //--
            //-- simple test of default load/save mechanisms.
            //--
            Trace.DebugWriteLine("Simple Test");
            TestContext.Reset();

            int milliSeconds = TestContext.MilliSeconds;
            NUnit.Framework.Assert.IsTrue(milliSeconds == 123);

            string name = TestContext.StackName;
            NUnit.Framework.Assert.IsTrue(name == "basic-stack-name");
        }

        //-- 
        //-- auxilary load for secondary test
        //--
        private void OnLoad()
        {
            TestContext.MilliSeconds = 9876;
            TestContext.StackName = "secondary-stack-name";
        }

        //--
        //-- auxilary save for secondary test.
        //--
        private void OnSave()
        {
            NUnit.Framework.Assert.IsTrue(TestContext.MilliSeconds == 5555);
            NUnit.Framework.Assert.IsTrue(TestContext.StackName == "modified-stack-name");
        }

        [Test]
        public void SecondaryTest()
        {
            //--
            //-- secondary test overrides the default load/save mechanisms
            //--
            Trace.DebugWriteLine("Secondary Test");
            TestContext.Reset();
               
            var ctx = new TestContext();
            ctx.OnHostLoad = OnLoad;
            ctx.OnHostSave = OnSave;

            NUnit.Framework.Assert.IsTrue(TestContext.MilliSeconds == 9876);
            NUnit.Framework.Assert.IsTrue(TestContext.StackName == "secondary-stack-name");

            TestContext.MilliSeconds = 5555;
            TestContext.StackName = "modified-stack-name";

            TestContext.Save();
        }

        [Test]
        public void ExceptionTest()
        {
            //--
            //-- spinach was not loaded intentionally,
            //-- so reading it before writing to it should
            //-- trigger an exception.
            //--
            Trace.DebugWriteLine("Exception Test");
            TestContext.Reset();

            try
            {
                Trace.DebugWriteLine("Exception Test: Phase 1");
                int xxx = TestContext.Spinach;
                NUnit.Framework.Assert.Fail();
            }
            catch (Exception ex)
            {
                Trace.DebugWriteLine(ex.Message);
            }

            try
            {
                Trace.DebugWriteLine("Exception Test: Phase 2");
                TestContext.Spinach = 3434;
                int xxx = TestContext.Spinach;
                NUnit.Framework.Assert.IsTrue(xxx == 3434);
            }
            catch (Exception ex)
            {
                Trace.DebugWriteLine(ex.Message);
                NUnit.Framework.Assert.Fail();
            }
        }
    }

}