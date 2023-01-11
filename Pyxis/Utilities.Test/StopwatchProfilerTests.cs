using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>   
    /// Unit tests for StopwatchProfiler
    /// </summary>
    [TestFixture]
    public class StopwatchProfilerTests
    {
        [Test]
        public void SampleOperation()
        {
            StopwatchProfiler myStopwatchProfiler = new StopwatchProfiler( "Test name - usually the function being profiled.");
            // Place some code here...
            myStopwatchProfiler.LogCompletion("Step one.");
            // Place some more code here...
            myStopwatchProfiler.LogCompletion("Step two.");

            Assert.Greater(myStopwatchProfiler.Output.Length, 0, "Profiler did not generate any output.");
        }

        [Test]
        public void UseStopwatchWithTrace()
        {
            TraceTool tracer = new TraceTool(true);

            StopwatchProfiler myStopwatchProfiler = new StopwatchProfiler("Test name", tracer);
            // Place some code here...
            myStopwatchProfiler.LogCompletion("Step one.");

            int outputLength = myStopwatchProfiler.Output.Length;
            Assert.Greater(outputLength, 0, "Profiler did not generate any output.");

            tracer.Enabled = false;

            // Place some more code here...
            myStopwatchProfiler.LogCompletion("Step two.");

            Assert.Greater(myStopwatchProfiler.Output.Length, outputLength, "Profiler did not generate output when trace disabled.");
        }

        [Test]
        public void GetOutputAndClear()
        {
            StopwatchProfiler myStopwatchProfiler = new StopwatchProfiler("Test name.");

            // Place some code here...

            myStopwatchProfiler.LogCompletion("Step one.");

            // Test "clear".
            Assert.Greater(myStopwatchProfiler.GetOutputAndClear().Length, 0, "GetOutputAndClear did not return a reasonable string.");
            Assert.AreEqual(myStopwatchProfiler.GetOutputAndClear().Length, 0, "GetOutputAndClear did not clear the output.");
        }
    }
}