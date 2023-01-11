/******************************************************************************
TimedTest.cs

begin      : Auguest 8, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Utility Function
    /// </summary>
    public static class TimedTest
    {
        /// <summary>
        /// The function to be tested.
        /// </summary>
        /// <returns>true iff the test is okay.</returns>
        public delegate bool TestFunction();

        /// <summary>
        /// Verify that the given test passes within the allotted time (in 
        /// ms).  Failure is reported via NUnit assert.
        /// </summary>
        /// <param name="test">The test that must pass.</param>
        /// <param name="timeout">Amount of time to test before failure is assumed.</param>
        /// <param name="message">Message to submit on failure.</param>
        public static void Verify(TestFunction test, TimeSpan timeout, string message)
        {
            DateTime end = DateTime.Now + timeout;
            
            while (test() == false && (DateTime.Now < end))
            {
                System.Threading.Thread.Sleep(100);
            }
            NUnit.Framework.Assert.IsTrue(test(), message);
        }

        /// <summary>
        /// Verify that the given test passes within the allotted time (in 
        /// ms).  Failure is reported via NUnit assert.
        /// </summary>
        /// <param name="test">The test that must pass.</param>
        /// <param name="timeout">Amount of time to test before failure is assumed.</param>
        public static void Verify(TestFunction test, TimeSpan timeout)
        {
            Verify(test, timeout, "Timed test failure");
        }
    }

    namespace Test
    {
        /// <summary>
        /// Tests for TimedTest class.  Note that we don't normally test for failure.
        /// </summary>
        [NUnit.Framework.TestFixture]
        public class TimedTestTest
        {
            [NUnit.Framework.Test]
            public void SuccessfulTest()
            {
                TimedTest.Verify(                        
                    delegate() { return true; }, TimeSpan.FromSeconds(1), 
                    "Unable to verify simplest delegate.");

                int count = 0;
                TimedTest.Verify(
                    delegate() { return (++count > 1); }, TimeSpan.FromSeconds(1), 
                    "Test should pass on third loop.");
            }

            /// <summary>
            /// Note that this test is not normally executed, since it would cause 
            /// an NUnit assertion.
            /// </summary>
            //[NUnit.Framework.Test]
            public void FailureTest()
            {
                TimedTest.Verify(
                    delegate() { return false; }, TimeSpan.FromSeconds(1), "This test is supposed to fail.");
            }
        }
    }
}
