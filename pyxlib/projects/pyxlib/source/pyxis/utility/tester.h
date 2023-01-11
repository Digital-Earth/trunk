#ifndef PYXIS__UTILITY__TESTER_H
#define PYXIS__UTILITY__TESTER_H
/******************************************************************************
tester.h

begin		: 2003-12-11
copyright	: derived from tester.h (C) 2001 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/object.h"

// boost includes
#include <boost/filesystem/operations.hpp>

// standard includes
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <list>

enum TestRunMode {
	knRunTestInAnyMode, knRunTestOnlyInForeground
};

enum TestRunPriority {
	knRunTestFirst
};


/*!
This is the base class for the templated unit testing classes.
*/
//! Testing template for unit testing.
class PYXLIB_DECL TesterBase
{
public:

	//! Constructor
	TesterBase() {;}

	//! Explict virtual destructor to avoid compiler warnings
	virtual ~TesterBase() {;}
   
	/*!
	The method called by the TestFrame to run tests
	*/
	virtual void test() = 0;

	//! Return the name of the class being tested.
	virtual std::string getTypeName() const = 0;

	//! The directory for test data
	boost::filesystem::path getTestDataPath() const; 

	//! TRUE means that this unit test cannot run in a background thread.
	virtual bool getForceForeground() = 0;	

private:

	//! Disable copy constructor
	TesterBase::TesterBase(const TesterBase&);

	//! Disable copy assignment
	void operator=(const TesterBase&);
};

/*!
There should be only one TestFrame object in the application.

Registering a tester object with the testframe will add it to the vector of
testers. testUnit() and testDetailed() runs the various tests and returns
true only if all of them succeed, false otherwise.
It is possible to get the names of the tester classes (which
are a bit odd because they are template instantiations) via the getNames()
method, and to run a single test using the test(std::string& strTestClassName)
method.

For most applications, testUnit() will be run every time the application is run.
For debugging and development purposes, the test() method allows a developer to
deal with just the test of interest.

The TestFrame creates a log file called "test.log" containing the results of
all test that are run.
*/
//! Manages the vector of testers and allows tests to be run
class PYXLIB_DECL TestFrame
{
public:

	// Constants for detailed and unit tests names.
	static const std::string kstrUninitialized;

	//! Typedef for vector of tests
	typedef std::vector<TesterBase*> TestVector;

	//! Typedef for vector of test names
	typedef std::vector<std::string> TestNameVector;

	typedef std::map< std::string, std::list < std::string > > TestAssertsMap;

	//! Get singleton instance of test frame
	static TestFrame& TestFrame::getInstance();

	//! Run all unit tests.
	bool performTest();

	//! Run unit tests specified by passed parameter.
	enum TestMode { AllTests, ForegroundOnly, BackgroundOnly};
	bool performTest( TestMode mode);

	//! Register a tester object with the test frame - ownership is not taken
	void addTest(TesterBase* pTester);

	//! Register a tester object with the test frame to run it first - ownership is not taken
	void addTestAtFront(TesterBase* pTester);
   
	//! Called by TEST_ASSERT macro to report failures.
	bool testAssert(bool bCondition, char* szFile, int nLine, std::string strCondition);

	//! Called by TEST_ASSERT_EQUAL macro to report failures.
	template<typename T1,typename T2>
	bool testAssertEqual(const T1 & value1,const T2 & value2, char* szFile, int nLine, std::string strValue1,std::string strValue2)
	{
		std::ostringstream stream;
		stream << strValue1 << " (was " << value1 << " ) == " << strValue2 << " (was " << value2 << ")";
		return testAssert(value1 == value2,szFile,nLine,stream.str());
	}

	//! Get a vector of all test class names.
	std::vector<std::string> getTests() const;

	//! Get a vector of all test class names that had failed.
	std::vector<std::string> getFailedTests() const;

	//! Run a single test by its class name.
	bool test(const std::string& strTestClassName);

	std::vector<std::string> getTestLog(const std::string & strTestClassName);

private:

	//! Disable copy constructor
	TestFrame(const TestFrame&);

	//! Disable copy assignment
	void operator=(const TestFrame&);

	//! Hide constructor
	TestFrame()	{;}

	//! Hide Destructor
	virtual ~TestFrame() {;}

	//! Initialize static data.
	static void initStaticData() {;}

	//! Free static data.
	static void freeStaticData();

	//! Map between names and tester objects
	TestVector m_vecTester;

	//! point to the current test
	std::string m_currentTest;

	//! Number of tests that succeeded
	int m_nSuccesses;

	//! Number of tests that failed
	int m_nFailures;

	//! Number of tests performed by a module
	int m_nModuleTests;

	//! List of all failed tests so far
	TestNameVector m_failedTests;

	//! List of all assertions line results, used for displaying results for a single test
	TestAssertsMap m_testsAssertsLog;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

/*!
Check for success condition on a test. Defined as a macro so we can report the
file name and line number if an error occurs.
*/
#define TEST_ASSERT(bCondition) \
	do {\
	if (!TestFrame::getInstance().testAssert(bCondition, __FILE__, __LINE__, #bCondition)) \
		{ \
			TRACE_TEST("\n\n**********!!!!!!!!!! Testing Error !!!!!!!!!!**********\n\n"); \
		} \
	} while (false)


#define TEST_ASSERT_EQUAL(value1,value2) \
	do {\
	TestFrame::getInstance().testAssertEqual(value1,value2, __FILE__, __LINE__, #value1,#value2); \
	} while (false)

/*!
Checks to ensure that the specified exception is thrown when the STATEMENT is executed.
*/
#define TEST_ASSERT_EXCEPTION( STATEMENT, EXCEPTION) \
	do {\
		bool expectedExceptionThrown = false;\
		try {\
		STATEMENT; \
		} \
		catch (EXCEPTION &) \
		{ \
			expectedExceptionThrown = true; \
		} \
		if (!expectedExceptionThrown) \
		{\
			TRACE_TEST("\n\n**********!!!!!!!!!! Expected Testing Exception not thrown !!!!!!!!!!**********\n\n");\
		}\
		TestFrame::getInstance().testAssert( expectedExceptionThrown, __FILE__, __LINE__, #STATEMENT); \
	} while (false) 

/*!
This template allows test methods in any class in to have its tests run as part of
a testing framework. In order to enable a unit test in a class the developer must
include the line:

Tester<MyClassName> MyTester;

In addition the develper must provide a test() method. This is the method that will
be executed during unit testing. To record a test result use the TEST_ASSERT and
TEST_ASSERT_EXCEPTION macros.
*/
//! Testing template for unit testing.
template <class Testee>
class Tester : public TesterBase
{
public:

	//! Constructor
	Tester( TestRunMode mode = knRunTestInAnyMode)
	{
		m_bforceForeground = (mode == knRunTestOnlyInForeground);
		TestFrame::getInstance().addTest(this);
	}

	//! Constructor
	Tester(TestRunPriority priority, TestRunMode mode = knRunTestInAnyMode)
	{
		m_bforceForeground = (mode == knRunTestOnlyInForeground);
		TestFrame::getInstance().addTestAtFront(this);
	}


	//! Explict virtual destructor.
	virtual ~Tester() {;}
   
	/*!
	The method called by the TestFrame to run tests
	*/
	virtual void test()
	{
#ifdef INSTANCE_COUNTING
		InstanceCounter::InstanceCountMap snapshot1(InstanceCounter::takeSnapShot());
#endif
		
		Testee::test();

#ifdef INSTANCE_COUNTING
		InstanceCounter::traceObjectCountChange(snapshot1,InstanceCounter::takeSnapShot());
#endif
	}

	//! Return the name of the class being tested.
	virtual std::string getTypeName() const
	{
		// could also return typeid(*this).name() to get qualified name
		std::string strName = typeid(Testee).name();

		// strip any leading numbers, to make the name easier to read
		std::string::const_iterator it = strName.begin();
		for (; it != strName.end(); ++it)
		{
			if (!isdigit(*it))
			{
				break;
			}
		}

		return std::string(it, strName.end());
	}

	//! TRUE means that this unit test cannot run in a background thread.
	bool getForceForeground()
	{
		return m_bforceForeground;
	}	

private:
	//! TRUE means that this unit test cannot run in the background thread.
	bool m_bforceForeground;


	//! Disable copy constructor
	Tester::Tester(const Tester&);

	//! Disable copy assignment
	void operator=(const Tester&);
};

#endif // guard
