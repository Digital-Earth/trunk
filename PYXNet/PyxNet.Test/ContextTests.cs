using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class ContextTests
    {
        Pyxis.Utilities.TraceTool Trace = new Pyxis.Utilities.TraceTool(true);

        [Test]
        public void SimpleTest()
        {
            PyxNet.Context.Reset();

            Trace.DebugWriteLine("Simple Test: before");
            Trace.DebugWriteLine("StackName: {0}", PyxNet.Context.StackName);
            Trace.DebugWriteLine("PrivateKey: {0}", PyxNet.Context.PrivateKey);
            Trace.DebugWriteLine("NodeID: {0}", PyxNet.Context.NodeID);
            Trace.DebugWriteLine("ExternalAddresses: {0}", PyxNet.Context.ExternalAddresses);
            Trace.DebugWriteLine("ConnectionRetryWaitMilliseconds: {0}", PyxNet.Context.ConnectionRetryWaitMilliseconds);

            PyxNet.Context.StackName = "my-stack";
            PyxNet.Context.PrivateKey = "my-private-key";
            PyxNet.Context.NodeID = "my-node-id";
            PyxNet.Context.ExternalAddresses = "192.168.199.199";
            PyxNet.Context.ConnectionRetryWaitMilliseconds = 54321;

            Trace.DebugWriteLine("Simple Test: after");
            Trace.DebugWriteLine("StackName: {0}", PyxNet.Context.StackName);
            Trace.DebugWriteLine("PrivateKey: {0}", PyxNet.Context.PrivateKey);
            Trace.DebugWriteLine("NodeID: {0}", PyxNet.Context.NodeID);
            Trace.DebugWriteLine("ExternalAddresses: {0}", PyxNet.Context.ExternalAddresses);
            Trace.DebugWriteLine("ConnectionRetryWaitMilliseconds: {0}", PyxNet.Context.ConnectionRetryWaitMilliseconds);
        }

        //-- 
        //-- auxilary load for secondary test
        //--
        private void OnLoad()
        {
            PyxNet.Context.ConnectionRetryWaitMilliseconds = 9876;
            PyxNet.Context.StackName = "secondary-stack-name";
        }

        //--
        //-- auxilary save for secondary test.
        //--
        private void OnSave()
        {
            NUnit.Framework.Assert.IsTrue(PyxNet.Context.ConnectionRetryWaitMilliseconds == 5555);
            NUnit.Framework.Assert.IsTrue(PyxNet.Context.StackName == "modified-stack-name");
        }

        [Test]
        public void SecondaryTest()
        {
            //--
            //-- secondary test overrides the default load/save mechanisms
            //--
            Trace.DebugWriteLine("Secondary Test");
            PyxNet.Context.Reset();

            var ctx = new PyxNet.Context();
            ctx.OnHostLoad = OnLoad;
            ctx.OnHostSave = OnSave;

            NUnit.Framework.Assert.IsTrue(PyxNet.Context.ConnectionRetryWaitMilliseconds == 9876);
            NUnit.Framework.Assert.IsTrue(PyxNet.Context.StackName == "secondary-stack-name");

            PyxNet.Context.ConnectionRetryWaitMilliseconds = 5555;
            PyxNet.Context.StackName = "modified-stack-name";

            PyxNet.Context.Save();
        }

    }
}