#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "tweet_test.h"
#include "metrics_test.h"
#include "file_test.h"

int runTests(void) {
	CPPUNIT_NS::TestResult testResult;
	CPPUNIT_NS::TestResultCollector collectedResults;
	testResult.addListener(&collectedResults);

	CPPUNIT_NS::BriefTestProgressListener progress;
	testResult.addListener(&progress);

	CPPUNIT_NS::TestRunner testRunner;
	testRunner.addTest( TweetTest::suite() );
	testRunner.addTest( MetricsTest::suite() );
	testRunner.addTest( FileTest::suite() );

	testRunner.run(testResult);

	CPPUNIT_NS::CompilerOutputter compilerOutputter(&collectedResults, std::cerr);
	compilerOutputter.write();

	return collectedResults.wasSuccessful() ? 0 : 1;
}


int main() {
	return runTests();
}
