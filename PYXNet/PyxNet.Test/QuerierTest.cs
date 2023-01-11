using System;
using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// This class encapsulates a Querier class test.
    /// </summary>
    class QuerierTest : IDisposable
    {
        public readonly Pyxis.Utilities.TraceTool Tracer = new Pyxis.Utilities.TraceTool(false, "Querier Test: ");

        /// <summary>
        /// The list of stacks to test.
        /// </summary>
        private readonly List<Stack> m_stacks = null;

        /// <summary>
        /// The queries run during tests.
        /// </summary>
        private readonly List<Querier> m_queriers = new List<Querier>();

        public QuerierTest(List<Stack> stacks)
        {
            m_stacks = stacks;
        }

        #region Dispose

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    m_queriers.Clear();
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~QuerierTest()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        public void Run()
        {
            m_stacks.ForEach(delegate(Stack stack)
            {
                TestFromStack(stack);
            });
        }

        private void AttachTestStacks()
        {
            // Set events for each stack.
            m_stacks.ForEach(delegate(Stack receivingStack)
            {
                receivingStack.OnQueryHit += HandleQueryHit;
                receivingStack.OnQueryResult += HandleQueryResult;
            });
        }

        private void DetachTestStacks()
        {
            // Unhook events from each stack.
            m_stacks.ForEach(delegate(Stack receivingStack)
            {
                receivingStack.OnQueryHit -= HandleQueryHit;
                receivingStack.OnQueryResult -= HandleQueryResult;
            });
        }

        /// <summary>
        /// Run queries on stack as part of the test.
        /// </summary>
        /// <param name="stack">The stack to run query tests on.</param>
        private void TestFromStack(Stack stack)
        {
            Tracer.DebugWriteLine("Starting query on {0}", stack.NodeInfo.FriendlyName);

            // The timeout to be passed to the Querier constructor, in milliseconds.
            int timeOut = 1000;

            int stackCount = m_stacks.Count;

            // Set events for each stack.
            AttachTestStacks();

            m_stacks.ForEach(delegate(Stack stackToFind)
            {
                // Create a querier.
                Querier querier = new Querier(stack,
                    stackToFind.NodeInfo.FriendlyName, timeOut);

                // Add the querier to queriers list.
                lock (m_queriers)
                {
                    m_queriers.Add(querier);
                }

                // Start the querier.
                Tracer.DebugWriteLine("Starting query {0}", stackToFind.NodeInfo.FriendlyName);
                querier.Start();

                // Wait for it to finish, or fail after a certain amount of time.
                if (!WaitForTestFromStackToFinish(querier))
                {
                    querier.Stop();

                    NUnit.Framework.Assert.Fail(String.Format(
                        "Query {0} was not found, starting on stack {1}.  Log saved as '{2}'.",
                        querier.Query.Contents, stack.NodeInfo.FriendlyName, Pyxis.Utilities.TraceTool.SaveToTempFile()));
                }
            });

            int queriesNotFound;
            lock (m_queriers)
            {
                queriesNotFound = m_queriers.Count;

                m_queriers.ForEach(delegate(Querier querier)
                {
                    Tracer.DebugWriteLine("Couldn't find query {0}", querier.Query.Contents);
                });
            }
            if (0 < queriesNotFound)
            {
                NUnit.Framework.Assert.Fail(
                    queriesNotFound.ToString() + " queries were not found, starting on stack " +
                    stack.NodeInfo.FriendlyName);
            }

            // Unhook events from each stack.
            DetachTestStacks();
        }

        /// <summary>
        /// Wait for the query to finish, or time out.
        /// </summary>
        /// <param name="querier">The running query.</param>
        /// <returns>True if the querier finished; false if it timed out.</returns>
        private bool WaitForTestFromStackToFinish(Querier querier)
        {
            // If the querier is not in the querier list, the query has finished.
            DateTime startTime = DateTime.Now;
            for (; ; )
            {
                lock (m_queriers)
                {
                    if (!m_queriers.Contains(querier))
                    {
                        break;
                    }
                }

                // Wait for a moment.
                System.Threading.Thread.Sleep(2000);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // TODO: Expose this timeout as a tunable parameter.
                if (elapsedTime.TotalSeconds >= 40)
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// Handle a query hit.
        /// </summary>
        /// <param name="stack">The stack that contained the hit.</param>
        /// <param name="query">The query that was sent to the stack.</param>
        private void HandleQueryHit(Stack stack, Query query)
        {
            // Check the stack friendly name.
            if (stack.NodeInfo.FriendlyName == query.Contents)
            {
                Tracer.DebugWriteLine("************* {0} was hit for query {1} *************",
                    stack.NodeInfo.FriendlyName, query.Contents);

                // Construct query result.
                QueryResult queryResult = new QueryResult(
                    query.Guid, query.OriginNode, stack.NodeInfo, null);

                // Send it on its way.
                stack.ProcessQueryResult(null, queryResult);

                return;
            }

            NUnit.Framework.Assert.Fail("There was a hit that shouldn't have been a hit.");
        }

        /// <summary>
        /// TODO:
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleQueryResult(object sender, Stack.QueryResultEventArgs args)
        {
            Tracer.DebugWriteLine(
                "*********************** '{0}' had a RESULT for query from '{1}'. **********************",
                args.Result.ResultNode.FriendlyName,
                args.Result.QueryOriginNode.FriendlyName);

            int queriersFound;
            lock (m_queriers)
            {
                queriersFound = m_queriers.RemoveAll(delegate(Querier element)
                {
                    if (element.Stack.NodeInfo.Equals(args.Result.QueryOriginNode))
                    {
                        element.Stop();
                        return true;
                    }
                    return false;
                });
            }
            NUnit.Framework.Assert.IsTrue(1 == queriersFound,
                String.Format("Query from '{0}' was found {1} time(s).", args.Result.QueryOriginNode.FriendlyName, queriersFound));
        }
    }
}