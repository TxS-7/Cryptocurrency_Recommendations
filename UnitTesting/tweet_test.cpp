#include <vector>
#include <string>
#include <unordered_map>
#include <cmath> // std::abs
#include "tweet_test.h"
#include "../tweet.h"
#include "../file_io.h"
#include <cppunit/extensions/HelperMacros.h>

void TweetTest::testInvalidFile(void) {
	std::vector<Tweet> tweets;
	unsigned int P;
	int res;

	// Try reading file with repeating tweet IDs
	res = readInputFile("UnitTesting/test_files/tweet_rep_id.csv", tweets, P);
	CPPUNIT_ASSERT( res == IO_NOT_UNIQUE );
	tweets.clear();

	// Try reading file with user IDs in wrong order
	res = readInputFile("UnitTesting/test_files/tweet_user_order.csv", tweets, P);
	CPPUNIT_ASSERT( res == IO_NOT_INCREASING );
	tweets.clear();

	// Try reading file with tweet IDs in wrong order
	res = readInputFile("UnitTesting/test_files/tweet_order.csv", tweets, P);
	CPPUNIT_ASSERT( res == IO_NOT_INCREASING );
	tweets.clear();

	// Try reading file with different separators
	res = readInputFile("UnitTesting/test_files/tweet_sep.csv", tweets, P);
	CPPUNIT_ASSERT( res == IO_GENERAL_ERROR );
	tweets.clear();

	// Try reading file with tab as separator (valid)
	res = readInputFile("UnitTesting/test_files/tweet_valid.csv", tweets, P);
	CPPUNIT_ASSERT( (unsigned int) res == tweets.size() );
}



void TweetTest::testTweet(void) {
	std::vector<Tweet> tweets;
	unsigned int P = 0;
	int res = readInputFile("UnitTesting/test_files/tweet_test.csv", tweets, P);

	CPPUNIT_ASSERT( res > 0 ); // No error while reading
	CPPUNIT_ASSERT( P == 45 ); // Number of neighbors is correct
	CPPUNIT_ASSERT( tweets.size() == 3 ); // 3 tweets read


	// Create a sentiment map and a coin list to test tweet sentiments
	std::unordered_map<std::string, double> sentimentMap;
	sentimentMap["good"] = 3;
	sentimentMap["bad"] = -3;
	sentimentMap["awful"] = -10;
	std::vector< std::vector<std::string> > coins;
	std::vector<std::string> btc;
	btc.push_back("btc");
	btc.push_back("bitcoin");
	coins.push_back(btc);
	std::vector<std::string> eth;
	eth.push_back("eth");
	eth.push_back("ethereum");
	coins.push_back(eth);


	std::vector<std::string> tokens;
	std::vector<double> sentiments;
	// First tweet checks
	tokens = tweets[0].getTokens();
	CPPUNIT_ASSERT( tokens.size() == 4 );
	CPPUNIT_ASSERT( tweets[0].getID() == 1 );
	CPPUNIT_ASSERT( tweets[0].getUser() == 78 );
	tweets[0].calculateSentiments(sentimentMap, 15, coins);
	sentiments = tweets[0].getSentiment();
	CPPUNIT_ASSERT( sentiments.size() == 2 );
	CPPUNIT_ASSERT( sentiments[0] == 0 );
	CPPUNIT_ASSERT( sentiments[1] == Tweet::SENTIMENT_NOT_SET );

	// Second tweet checks
	tokens = tweets[1].getTokens();
	CPPUNIT_ASSERT( tokens.size() == 4 );
	CPPUNIT_ASSERT( tweets[1].getID() == 2 );
	CPPUNIT_ASSERT( tweets[1].getUser() == 78 );
	tweets[1].calculateSentiments(sentimentMap, 15, coins);
	sentiments = tweets[1].getSentiment();
	CPPUNIT_ASSERT( sentiments.size() == 2 );
	CPPUNIT_ASSERT( std::abs(sentiments[0] + 0.95837) < 0.0001 );
	CPPUNIT_ASSERT( std::abs(sentiments[1] + 0.95837) < 0.0001 );

	// Third tweet checks
	tokens = tweets[2].getTokens();
	CPPUNIT_ASSERT( tokens.size() == 2 );
	CPPUNIT_ASSERT( tweets[2].getID() == 3 );
	CPPUNIT_ASSERT( tweets[2].getUser() == 100 );
	tweets[2].calculateSentiments(sentimentMap, 15, coins);
	sentiments = tweets[2].getSentiment();
	CPPUNIT_ASSERT( sentiments.size() == 2 );
	CPPUNIT_ASSERT( sentiments[0] == Tweet::SENTIMENT_NOT_SET );
	CPPUNIT_ASSERT( std::abs(sentiments[1] - 0.61237) < 0.0001 );
}
