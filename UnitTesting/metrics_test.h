#ifndef METRICS_TEST_H
#define METRICS_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class MetricsTest: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE( MetricsTest );
	CPPUNIT_TEST( testEuclideanDistance );
	CPPUNIT_TEST( testEuclideanSimilarity );
	CPPUNIT_TEST( testCosineDistance );
	CPPUNIT_TEST( testCosineSimilarity );
	CPPUNIT_TEST_SUITE_END();
public:
	void testEuclideanDistance(void);
	void testEuclideanSimilarity(void);
	void testCosineDistance(void);
	void testCosineSimilarity(void);
};

#endif // METRICS_TEST_H
