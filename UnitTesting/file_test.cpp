#include <vector>
#include <string>
#include <unordered_map>
#include "file_test.h"
#include "../file_io.h"
#include <cppunit/extensions/HelperMacros.h>

void FileTest::testReadSentimentLexicon(void) {
	std::unordered_map<std::string, double> sentimentMap;
	// Check files with errors
	CPPUNIT_ASSERT( readSentimentLexicon("UnitTesting/test_files/sl_invalid.csv", sentimentMap) == false );
	CPPUNIT_ASSERT( readSentimentLexicon("UnitTesting/test_files/sl_invalid2.csv", sentimentMap) == false );

	sentimentMap.clear();
	CPPUNIT_ASSERT( readSentimentLexicon("UnitTesting/test_files/sl_valid.csv", sentimentMap) == true );

	// Check if sentiments are correct
	CPPUNIT_ASSERT( sentimentMap["hello"] == 1.5 );
	CPPUNIT_ASSERT( sentimentMap["world"] == -10 );
	CPPUNIT_ASSERT( sentimentMap["test"] == 4.0 );
	CPPUNIT_ASSERT( sentimentMap["bla"] == 0.0 );
}



void FileTest::testReadCoins(void) {
	std::vector< std::vector<std::string> > coinNames;
	CPPUNIT_ASSERT( readCoins("UnitTesting/test_files/coins.csv", coinNames) == true );

	// Check if coin names are correct
	CPPUNIT_ASSERT( coinNames.size() == 2 );
	CPPUNIT_ASSERT( coinNames[0].size() == 6 );
	CPPUNIT_ASSERT( coinNames[0][0] == "mycoin" );
	CPPUNIT_ASSERT( coinNames[0][1] == "myCoin" );
	CPPUNIT_ASSERT( coinNames[0][2] == "MYCOIN" );
	CPPUNIT_ASSERT( coinNames[0][3] == "Mycoin" );
	CPPUNIT_ASSERT( coinNames[0][4] == "#mycoin" );
	CPPUNIT_ASSERT( coinNames[0][5] == "@mycoin" );

	CPPUNIT_ASSERT( coinNames[1].size() == 4 );
	CPPUNIT_ASSERT( coinNames[1][0] == "btc" );
	CPPUNIT_ASSERT( coinNames[1][1] == "BitCoin" );
	CPPUNIT_ASSERT( coinNames[1][2] == "#btc" );
	CPPUNIT_ASSERT( coinNames[1][3] == "@btc" );
}
