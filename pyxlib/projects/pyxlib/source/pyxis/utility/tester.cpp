/******************************************************************************
tester.cpp

begin		: 2003-12-11
copyright	: derived from tester.cpp (C) 2001 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/tester.h"

// local includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/properties.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <ctime>
#include <iomanip>

/*! 
The directory that is configured to hold information required by unit test.
*/
boost::filesystem::path TesterBase::getTestDataPath() const
{
	std::string strDefaultPath = "..\\..\\..\\test_data";
	std::string strDirectory = getAppProperty(
		"Tester",
		"TestDataDirectory",
		strDefaultPath,
		"The root directory where unit test data files exist");
	boost::filesystem::path path = FileUtils::stringToPath(strDirectory);

	// turn the relative path into an absolute
	path = boost::filesystem::complete(path);

	return path;
}

// Constants for detailed and unit tests names.
const std::string TestFrame::kstrUninitialized = "No log file";

//! Test frame must be global static for correct startup initialization
TestFrame* gpTestFrame = 0;

/*!
Get singleton instance of test frame.

\return	The test frame.
*/
TestFrame& TestFrame::getInstance()
{
	/*
	Static allocation performed in constructor to ensure global TestFrame is
	available when static Testers are constructed.
	*/
	if (0 == gpTestFrame)
	{
		gpTestFrame = new TestFrame();
	}

	return *gpTestFrame;
}

/*!
Free static data when application exits
*/
void TestFrame::freeStaticData()
{
	if (0 != gpTestFrame)
	{
		delete gpTestFrame;
		gpTestFrame = 0;
	}
}

/*!
Register a tester object with the test frame - ownership is not taken

\param	pTester	The test class.
*/
void TestFrame::addTest(TesterBase* pTester)
{
	m_vecTester.push_back(pTester);
}

void TestFrame::addTestAtFront(TesterBase* pTester)
{
	m_vecTester.insert(m_vecTester.begin(), pTester);
}

/*!
Run unit tests on all classes that are known to the test frame.

\return	true if all tests succeeded, otherwise false.
*/
bool TestFrame::performTest()
{
	return performTest( AllTests);
}

/*!
Run unit tests on all classes that are known to the test frame.
\param	mode	Select which tests to perform.
\return	true if all specified tests succeeded, otherwise false.
*/
bool TestFrame::performTest(TestMode mode)
{

	TRACE_TEST("*****************************************************************");
#ifdef _DEBUG
	TRACE_TEST("Automated Debug testing started: " << StringUtils::now());
#endif

#ifndef _DEBUG
	TRACE_TEST("Automated Release testing started: " << StringUtils::now());
#endif
	TRACE_TEST("Testing all " 
		<< ((mode == AllTests) ? "" :
		((mode == BackgroundOnly) ? "background " : "foreground "))
		<< "tests found in "
		<< static_cast<int>(m_vecTester.size()) << " classes.");
	TRACE_TEST("*****************************************************************");

	// count number of successful and failed tests
	m_nSuccesses = 0;
	m_nFailures = 0;

	std::string strLastTest;
	for (TestVector::size_type nIndex = 0; nIndex < m_vecTester.size(); ++nIndex)
	{
		if ((mode == BackgroundOnly) && m_vecTester[nIndex]->getForceForeground())
		{
			continue;
		}

		if ((mode == ForegroundOnly) && !m_vecTester[nIndex]->getForceForeground())
		{
			continue;
		}

		m_nModuleTests = 0;
		TRACE_TEST("*****************************************************************");
		TRACE_TEST(m_vecTester[nIndex]->getTypeName());
		TRACE_TEST("*****************************************************************");

		int oldFailedCount = m_nFailures;
		clock_t nStart = clock();
		try
		{
			m_currentTest = m_vecTester[nIndex]->getTypeName();
			m_testsAssertsLog[m_currentTest].clear();
			m_vecTester[nIndex]->test();
		}
		catch (PYXException&)
		{
			TRACE_TEST("\n\n*****!!! Unexpected PYXIS Exception during automated testing of '" << 
				m_vecTester[nIndex]->getTypeName() << 
				"' !!!*****\n\n");

			m_testsAssertsLog[m_currentTest].push_back("[FAILED] Exception");
			m_nFailures++;
		}
		catch (...)
		{
			TRACE_TEST("\n\n*****!!! Unexpected Generic Exception during automated testing '" << 
				m_vecTester[nIndex]->getTypeName() << 
				"' !!!*****\n\n");
			m_testsAssertsLog[m_currentTest].push_back("[FAILED] Exception");
			m_nFailures++;
		}

		if (oldFailedCount != m_nFailures)
		{
			m_failedTests.push_back(m_vecTester[nIndex]->getTypeName());
		}

		double fSeconds = (static_cast<double>(clock()) - nStart) / CLOCKS_PER_SEC;
		TRACE_TEST("-> " << std::setprecision(4) << fSeconds << " seconds for " << m_nModuleTests << " tests \n");
	}

	int nTotal = m_nSuccesses + m_nFailures;

	TRACE_TEST("*****************************************************************");
	TRACE_TEST(nTotal << " test(s) completed at: " << StringUtils::now());
	TRACE_TEST(m_nFailures << " test(s) failed.");
	TRACE_TEST("*****************************************************************");

	return (0 == m_nFailures);
}

/*!
Called by TEST_ASSERT macro to report failures.

\param	bCondition	The test condition
\param	szFile		The file where the test occurred.
\param	nLine		The line where the test occurred.

\return The passed condition.
*/
bool TestFrame::testAssert(bool bCondition, char* szFile, int nLine, std::string strCondition)
{
	if (bCondition)
	{
		//m_testsAssertsLog[m_currentTest].push_back(StringUtils::format("[PASSED] %1 (%2) : %3",szFile,StringUtils::toString(nLine),strCondition));
		++m_nSuccesses;
	}
	else
	{
		m_testsAssertsLog[m_currentTest].push_back(StringUtils::format("[FAILED] %1 (%2) : %3",szFile,StringUtils::toString(nLine),strCondition));

		std::string strFile(szFile);

        // We want to trace with the caller's location, not ours.
        // (So we need to reach into the IMPL instead of TRACE_INFO).
		TRACE_IMPL( Trace::knTest, szFile, nLine, "********  Unit test failed. ******** " << strCondition);

		++m_nFailures;
	}

	++m_nModuleTests;

	return bCondition;
}

/*!
Get a vector of all test class names.

\return	Vector containing all test class names.
*/
std::vector<std::string> TestFrame::getTests() const
{
	TestNameVector vecName;
	for (TestNameVector::size_type nIndex = 0; nIndex < m_vecTester.size(); ++nIndex)
	{
		/*
		Call getTypeName rather than typeid on the pointer itself because
		typeid returns only the base type name when called on a base type
		pointer, although this violates the standard.
		*/
		vecName.push_back(m_vecTester[nIndex]->getTypeName());
	}

	return vecName;
}
/*!
Get a vector of all test class names that have failed.

\return	Vector containing all test class names.
*/
std::vector<std::string> TestFrame::getFailedTests() const
{
	return m_failedTests;
}

std::vector<std::string> TestFrame::getTestLog(const std::string& strTestClassName)
{	
	const std::list<std::string> & log = m_testsAssertsLog[strTestClassName];
	return std::vector<std::string>(log.begin(),log.end());
}

/*!
Run a single test by its class name.

\return	true if the test succeeded, otherwise false.
*/
bool TestFrame::test(const std::string& strTestClassName)
{
	m_nSuccesses = 0;
	m_nFailures = 0;

	// first look up test linearly in vector
	unsigned int nIndex;
	for (nIndex = 0; nIndex < m_vecTester.size(); ++nIndex)
	{
		if (strTestClassName == m_vecTester[nIndex]->getTypeName())
		{
			break;
		}			
	}
	
	// run the test
	m_currentTest = m_vecTester[nIndex]->getTypeName();
	m_testsAssertsLog[m_currentTest].clear();
	m_vecTester[nIndex]->test();

	return (0 == m_nFailures);
}

