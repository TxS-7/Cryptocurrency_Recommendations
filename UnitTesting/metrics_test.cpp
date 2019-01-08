#include <vector>
#include "metrics_test.h"
#include "../data_point.h"
#include "../metrics.h"
#include <cppunit/extensions/HelperMacros.h>

void MetricsTest::testEuclideanDistance(void) {
	std::vector<double> p1Vec(4); // (1, 1, 5, 7)
	std::vector<double> p2Vec(4); // (3, 3, 7, 5)
	p1Vec[0] = 1;
	p1Vec[1] = 1;
	p1Vec[2] = 5;
	p1Vec[3] = 7;
	p2Vec[0] = 3;
	p2Vec[1] = 3;
	p2Vec[2] = 7;
	p2Vec[3] = 5;

	DataPoint p1(p1Vec, "aaa");
	DataPoint p2(p2Vec, "bbb");

	// Test same point
	CPPUNIT_ASSERT( Metrics::euclideanDistance(p1, p1) == 0.0 );

	CPPUNIT_ASSERT( Metrics::euclideanDistance(p1, p2) == 4.0 );
}



void MetricsTest::testEuclideanSimilarity(void) {
	std::vector<double> p1Vec(4); // (1, 1, 5, 7)
	std::vector<double> p2Vec(4); // (3, 3, 7, 5)
	p1Vec[0] = 1;
	p1Vec[1] = 1;
	p1Vec[2] = 5;
	p1Vec[3] = 7;
	p2Vec[0] = 3;
	p2Vec[1] = 3;
	p2Vec[2] = 7;
	p2Vec[3] = 5;

	DataPoint p1(p1Vec, "aaa");
	DataPoint p2(p2Vec, "bbb");

	// Test same point similarity
	CPPUNIT_ASSERT( Metrics::euclideanSimilarity(p1, p1) == 1.0 );

	CPPUNIT_ASSERT( Metrics::euclideanSimilarity(p1, p2) == 0.2 );
}



void MetricsTest::testCosineDistance(void) {
	std::vector<double> p1Vec(6); // (1, 1, 0, 1, 1, 0), norm = 2
	p1Vec[0] = 1;
	p1Vec[1] = 1;
	p1Vec[2] = 0;
	p1Vec[3] = 1;
	p1Vec[4] = 1;
	p1Vec[5] = 0;
	std::vector<double> p2Vec(6); // (3, 0, 1, 2, 1, 1), norm = 4
	p2Vec[0] = 3;
	p2Vec[1] = 0;
	p2Vec[2] = 1;
	p2Vec[3] = 2;
	p2Vec[4] = 1;
	p2Vec[5] = 1;

	DataPoint p1(p1Vec, "aaa");
	DataPoint p2(p2Vec, "bbb");

	CPPUNIT_ASSERT( p1.getNorm() == 2 );
	CPPUNIT_ASSERT( p2.getNorm() == 4 );

	// Test same point
	CPPUNIT_ASSERT( Metrics::cosineDistance(p1, p1) == 0.0 );

	CPPUNIT_ASSERT( Metrics::cosineDistance(p1, p2) == 0.25 );
}



void MetricsTest::testCosineSimilarity(void) {
	std::vector<double> p1Vec(6); // (1, 1, 0, 1, 1, 0), norm = 2
	p1Vec[0] = 1;
	p1Vec[1] = 1;
	p1Vec[2] = 0;
	p1Vec[3] = 1;
	p1Vec[4] = 1;
	p1Vec[5] = 0;
	std::vector<double> p2Vec(6); // (3, 0, 1, 2, 1, 1), norm = 4
	p2Vec[0] = 3;
	p2Vec[1] = 0;
	p2Vec[2] = 1;
	p2Vec[3] = 2;
	p2Vec[4] = 1;
	p2Vec[5] = 1;
	std::vector<double> p3Vec(6);
	for (unsigned int i = 0; i < p3Vec.size(); i++) {
		p3Vec[i] = 0.0;
	}
	p3Vec[2] = 5;

	DataPoint p1(p1Vec, "aaa");
	DataPoint p2(p2Vec, "bbb");
	DataPoint p3(p3Vec, "ccc");

	CPPUNIT_ASSERT( p1.getNorm() == 2 );
	CPPUNIT_ASSERT( p2.getNorm() == 4 );
	CPPUNIT_ASSERT( p3.getNorm() == 5 );

	// Test same point similarity
	CPPUNIT_ASSERT( Metrics::cosineSimilarity(p1, p1) == 1.0 );

	// Test similarity with 0 dot product
	CPPUNIT_ASSERT( Metrics::cosineSimilarity(p1, p3) == 0.0 );

	CPPUNIT_ASSERT( Metrics::cosineSimilarity(p1, p2) == 0.75 ); // 1 - 0.25
}
