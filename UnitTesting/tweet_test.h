#ifndef TWEET_TEST_H
#define TWEET_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class TweetTest: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE( TweetTest );
	CPPUNIT_TEST( testInvalidFile );
	CPPUNIT_TEST( testTweet );
	CPPUNIT_TEST_SUITE_END();
public:
	void testInvalidFile(void);
	void testTweet(void);
};

#endif // TWEET_TEST_H
