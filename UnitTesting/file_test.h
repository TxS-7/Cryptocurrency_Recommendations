#ifndef FILE_TEST_H
#define FILE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class FileTest: public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE( FileTest );
	CPPUNIT_TEST( testReadSentimentLexicon );
	CPPUNIT_TEST( testReadCoins );
	CPPUNIT_TEST_SUITE_END();
public:
	void testReadSentimentLexicon(void);
	void testReadCoins(void);
};

#endif // FILE_TEST_H
