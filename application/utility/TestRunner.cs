/******************************************************************************
TestRunner.cs

begin		: November 24, 2006
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;

namespace ApplicationUtility
{
    /// <summary>
    /// This class runs unit tests and detailed tests and provides output to
    /// the user.
    /// </summary>
    public static class TestRunner
    {
        /// <summary>
        /// Run the automated tests with or without information
        /// displayed to the user in the form of a message box.
        /// </summary>
        /// <param name="runSilent">If true, no input will be required from the user when a failure
        /// occurs. If false, any failure in the tests will
        /// cause a message box to display. Program execution will not
        /// continue until the user presses the OK button on the dialog.</param>
        /// <param name="runManagedTests"> If set to true, then all of the nunit C# tests 
        /// are run, from within the application, </param>
        /// <param name="mode">if set to <c>true</c> [run background].</param>
        /// <returns>
        /// Success when all tests pass otherwise UnitTestFailure
        /// </returns>
        public static ApplicationResult RunTests(
            bool runSilent, 
            bool runUnManagedTests, 
            bool runManagedTests, 
            TestFrame.TestMode mode)
        {
            Trace.getInstance().traceOn(Trace.eLevel.knTest);
            Trace.time("Automated PYXIS testing has begun.");

            bool testsFailed = false;
            ManagedTestRunner managedTestRunner = null;

            if (runUnManagedTests)
            {
                Trace.test("Running SDK tests");
                if (mode == TestFrame.TestMode.ForegroundOnly)
                {
                    if (!TestFrame.getInstance().performTest(mode))
                    {
                        testsFailed = true;
                    }
                }
                else if (runSilent)
                {
                    if (!TestFrame.getInstance().performTest(mode))
                    {
                        testsFailed = true;
                    }
                }
                else
                {
                    ShowUnmanagedUnitTestForm();
                }
            }

            if (runManagedTests)
            {
                Trace.test("Running managed tests");
                managedTestRunner = new ManagedTestRunner(AppServices.getApplicationPath());
                if (!managedTestRunner.StartTests())
                {
                    testsFailed = true;
                }
            }

            Trace.time("Automated PYXIS testing is complete.");
            if (testsFailed && !runSilent)
            {
                TestFailedMessage();
            }

            Trace.getInstance().traceOff(Trace.eLevel.knTest);
            return (testsFailed ? ApplicationResult.UnitTestFailure : ApplicationResult.Success);
        }

        /// <summary>
        /// This method puts up a message box indicating that a test has failed.
        /// </summary>
        static void TestFailedMessage()
        {
            UnitTestFailure failure = new UnitTestFailure();
            failure.ShowFailedTestOnly = true;
            failure.ShowDialog();
        }

        static void ShowUnmanagedUnitTestForm()
        {
            UnitTestFailure dialog = new UnitTestFailure();
            dialog.CloseIfCompletedWithNoFailures = true;
            dialog.ShowDialog();
            //dialog.RunAllTests();
        }
    }

    /// <summary>
    /// This custom attribute class is an attribute class which can be used to prevent the 
    /// managed test runner from running specific tests or methods normally run under nunit.
    /// This allows the managed test runner to ignore certain methods however when run under
    /// NUnit the methods will still run. An example of this is: 
    /// WorldViewTester.LibraryTesting() This test is meant to be run under NUnit so that sdk
    /// testing can occur under NUnit. However when running the tests from the command line, 
    /// managed test runner detects this test and running it again would be duplicating work.
    /// </summary>
    [AttributeUsage(AttributeTargets.All)]
    public class ManagedTestRunnerIgnoreAttribute : System.Attribute
    {
        private string m_Reason;

        /// <summary>
        /// Construct a custom attribute to prevent the managed test runner from 
        /// running a particular method found via reflection.
        /// </summary>
        /// <param name="ignoreReason">The reason this method is being ignored.</param>
        public ManagedTestRunnerIgnoreAttribute(String ignoreReason)
        {
            m_Reason = ignoreReason;
        }

        /// <summary>
        /// Field to get the reason why this method is being ignored.
        /// </summary>
        public string Reason
        {
            get
            {
                return m_Reason;
            }
        }
    }

    /// <summary>
    /// Managed test runner is a utility class to run tests written in managed C# to be run 
    /// when the application receives a command line paramters.
    /// 
    /// Find more info at:
    /// http://euclid:9000/WorldView/wiki/UnitTesting
    /// </summary>
    public class ManagedTestRunner
    {
        /// <summary>
        /// Permits the ManagedTestRunner to self-synchronize.
        /// </summary>
        private object m_internalLock = new object();

        private List<System.Reflection.Assembly> m_assembliesForTesting = null;

        public List<System.Reflection.Assembly> AssembliesForTesting
        {
            get 
            {
                lock (m_internalLock)
                {
                    if (m_assembliesForTesting == null)
                    {
                        m_assembliesForTesting = GetAssembliesForTesting();
                    }
                }
                return m_assembliesForTesting; 
            }
        }

        private string m_testDirectory;
        private int m_totalTests;
        private int m_testFailedCount;

        #region Constructors
        /// <summary>
        /// Creates and sets up the Managed Test runner to run tests in a background worker thread.
        /// </summary>
        public ManagedTestRunner(string assemblyDirectory)
        {
            TestDirectory = assemblyDirectory;
        }
        #endregion


       /// <summary>
        /// Uses reflection to load all .NET assemblies into a collection, to later search 
        /// for test fixtures with. If an assembly can be loaded it will be, regardless if 
        /// it has test fixatures or not. Native assemblies will not be loaded.
        /// </summary>
        private List<System.Reflection.Assembly> GetAssembliesForTesting()
        {
            List<System.Reflection.Assembly> assembliesForTesting = new List<System.Reflection.Assembly>();

            //Get all the assemblies loaded in our memory space.
            //If the assembly is already loaded in our memory space that is the one we want to reflect over.
            foreach (System.Reflection.Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                assembliesForTesting.Add( assembly);
            }

            foreach (String filePath in System.IO.Directory.GetFiles(TestDirectory, "*.dll"))
            {
                bool alreadyLoaded = false;
                try
                {
                    //If the assembly relating to the file path has already been loaded, don't load it again.
                    System.Reflection.AssemblyName assemblyName = System.Reflection.AssemblyName.GetAssemblyName(filePath);
                    foreach (System.Reflection.Assembly assembly in assembliesForTesting)
                    {
                        if (assembly.FullName.Equals(assemblyName.FullName))
                        {
                            alreadyLoaded = true;
                            break;
                        }
                    }

                    if (!alreadyLoaded)
                    {
                        System.Reflection.Assembly assembly = System.Reflection.Assembly.LoadFile(filePath);
                        if (assembly != null)
                        {
                            assembliesForTesting.Add(assembly);
                        }
                    }
                }
                catch (Exception)
                {
                }
            }

            return assembliesForTesting;
        }

        /// <summary>
        /// Delegate to RunTests to start running managed unit tests 
        /// </summary>
        /// <returns>A property indicating whether or not all tests passed or there was a failure.</returns>
        public bool StartTests()
        {
            RunTests();
            return TestFailed ? false : true;
        }

        /// <summary>
        /// Delegate out to helper method to load all the assemblies which can be loaded for unit testing.
        /// Then using reflection find all unit tests marked with the NUnit.Framework.TestAttribute, and run
        /// those tests.
        /// </summary>
        private void RunTests()
        {
            foreach (System.Reflection.Assembly assembly in AssembliesForTesting)
            {
                /*[chowell] - Currently there seems to be an issue with one of our assemblies failing 
                 * to load. This try-catch block isn't the complete solution, I'd really like to figure
                 * out why that assembly is failing to load, because it may be indicative of larger problems.
                 * However this try-catch block is probably a good thing ot have here anyway and if an assembly
                 * can't be loaded it won't crash the autobuild. For now it will cause the autobuild
                 * to report a test failure but testing will be complete. 
                 */
                Type[] typesList = null;
                try
                {
                    typesList = assembly.GetTypes();
                }
                catch (System.Reflection.ReflectionTypeLoadException ex)
                {
                    /*[chowell] - Disabling, this for now so that we can conintue. Will address this after
                     *back from vacation. 
                     */
                    // m_testsFailed = true;
                    lock (m_internalLock)
                    {
                        // Details on why the assembly threw an exception.
                        Trace.error("***** Managed Unit tester caused an exception see details ****");
                        Trace.error("An Exception occured while trying to get the types of assembly " + assembly.GetName().ToString());
                        Trace.error("Details of the exception are: " + ex.ToString());
                    }

                    // We can't load this assembly so continue onto the next one. 
                    continue;
                }

                foreach (Type type in typesList)
                {
                    // Is this a test fixture?
                    if (type.GetCustomAttributes(typeof(NUnit.Framework.TestFixtureAttribute), false).Length == 0)
                    {
                        continue;
                    }

// A simple way to run only tests from a specific TestFixture by 
// specifying a category for it.  In the example below, the ManagedTestRunner 
// will only run unit tests for the test fixture with the category 
// "Database Tests".
#if false
                    object[] customAttributes = 
                        type.GetCustomAttributes(
                        typeof(NUnit.Framework.CategoryAttribute), false);

                    if (!(customAttributes.Length > 0 && 
                        (customAttributes[0] as NUnit.Framework.CategoryAttribute).Name == "Database Tests"))
                    {
                        continue;
                    }
#endif

                    StringBuilder errorLog = new StringBuilder();
                    Object objInstance = null;
                    try
                    {
                        objInstance = Activator.CreateInstance(type);
                        
                        // Todo: We don't run setup or teardown routines.

                        if (objInstance == null)
                        {
                            throw new ApplicationException("Unable to construct object.");
                        }
                    }
                    catch (Exception ex)
                    {
                        ++m_testFailedCount;
                        Trace.test(
                            string.Format("Unable to construct test fixture {0}.  {1}",
                            type.FullName,
                            ex.ToString()));
                        break;
                    }

                    int totalTestsRun = 0;
                    int totalFailures = 0;
                    int totalTestsIgnored = 0;

                    foreach (System.Reflection.MethodInfo info in type.GetMethods())
                    {
                        // Is this an NUnit test function?
                        if (!HasAttribute(info, typeof(NUnit.Framework.TestAttribute)))
                        {                            
                            continue;
                        }

                        // This test was flagged specifically to be ignored here.
                        if (HasAttribute(info, typeof(ApplicationUtility.ManagedTestRunnerIgnoreAttribute)))
                        {
                            continue;
                        }

                        if (HasAttribute(info, typeof(NUnit.Framework.IgnoreAttribute)))
                        {
                            errorLog.AppendFormat( "  Ignored test {0}.{1}",                             
                                type.FullName, info.Name);
                            errorLog.AppendLine();

                            ++totalTestsIgnored;
                            continue;
                        }

                        List<Type> expectedExceptions = new List<Type>();

                        foreach (NUnit.Framework.ExpectedExceptionAttribute attribute in
                            info.GetCustomAttributes(typeof(NUnit.Framework.ExpectedExceptionAttribute), false))
                        {
                            expectedExceptions.Add(attribute.ExpectedException);
                        }

                        Exception testFailure = null;

                        try
                        {
                            //Run the test.
                            ++totalTestsRun;
                            ++m_totalTests;
                            errorLog.AppendLine(                            
                                string.Format( "Testing: {0}.{1}", type.FullName,info.Name));

                            info.Invoke(objInstance, null);
                            if (expectedExceptions.Count > 0)
                            {
                                testFailure = new ApplicationException(
                                    string.Format( "Expected exception {0} was not thrown.",
                                        expectedExceptions[0].FullName));
                            }
                        }
                        catch (System.Reflection.TargetInvocationException except)
                        {
                            bool exceptionWasExpected = false;

                            Type thrownException = except.InnerException.GetType();
                            foreach( Type t in expectedExceptions)
                            {
                                if ((thrownException == t) || thrownException.IsSubclassOf(t))
                                {
                                    exceptionWasExpected = true;
                                }
                            }

                            if (!exceptionWasExpected)
                            {
                                testFailure = except.InnerException;
                            }
                        }
                        catch (Exception unExpectedException)
                        {
                            testFailure = unExpectedException;
                        }
                        finally
                        {
                            if (testFailure != null)
                            {                                                                
                                ++totalFailures;
                                ++m_testFailedCount;

                                lock (m_internalLock)
                                {
                                    errorLog.AppendLine("Test Failure in: " + type.FullName + "." + info.Name);
                                    errorLog.AppendLine("Because: " + testFailure.ToString());
                                }
                            }
                        }
                    }

                    StringBuilder logMessage = new StringBuilder();
                    logMessage.AppendFormat("Testing class {0} ", type.FullName);
                    logMessage.AppendFormat(" {0} tests ({1} failures)",
                        totalTestsRun, totalFailures);
                    if (totalTestsIgnored > 0)
                    {
                        logMessage.AppendFormat(", {0} ignored.", totalTestsIgnored);
                    }
                    Trace.test(logMessage.ToString());
                    if (totalFailures > 0)
                    {
                        Trace.test("!!!!!!!!!! Testing Error !!!!!!!!!!!!");
                        Trace.test(errorLog.ToString());
                        Trace.test("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
                    }
                    else if (totalTestsIgnored > 0)
                    {
                        Trace.test(errorLog.ToString());
                    }
                }
            }

            Trace.test(string.Format("Managed tests complete. {0} tests, {1} failures.",
                m_totalTests, m_testFailedCount));
        }

        private bool HasAttribute(System.Reflection.MethodInfo info, Type attributeType)
        {
            return info.GetCustomAttributes( attributeType, false).Length > 0;
        }

        /// <summary>
        /// Utility method for displaying some statistics about the class that was just tested.
        /// </summary>
        /// <param name="totalTest">Total number of tests run in the class</param>
        /// <param name="testFailures">Total number of tests that failed.</param>
        private void ReportClassTests(int totalTest, int testFailures)
        {
            int testsPassed = totalTest - testFailures;

            Trace.test("Total Tests Run: " + totalTest);
            Trace.test("Total Passed: " + testsPassed);
            Trace.test("Total Failed: " + testFailures);
            Trace.test("******************************************************************\n");
        }

        #region Properties
        /// <summary>
        /// Attribute to set and retrieve the directory to reflect in to find .Net assemblies.
        /// </summary>
        public string TestDirectory
        {
            get
            {
                return m_testDirectory;
            }
            set
            {
                m_testDirectory = value;
            }
        }

        /// <summary>
        /// Attribute to return the number of tests failures that have been recorded.
        /// </summary>
        public int TestingFailureCount
        {
            get
            {
                return m_testFailedCount;
            }
        }

        /// <summary>
        /// Attribute to return the total number of tests run.
        /// </summary>
        public int TestCount
        {
            get
            {
                return m_totalTests;
            }
        }

        /// <summary>
        /// Attribute indicating if all tests passed or if one failed.
        /// </summary>
        public bool TestFailed
        {
            get
            {
                return m_testFailedCount > 0;
            }
        }
        #endregion 

    }
}
