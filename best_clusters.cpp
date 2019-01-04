#include <iostream>
#include <vector>
#include <unordered_map>
#include "tweet.h"
#include "recommendation.h"
#include "clustering_recommender.h"
#include "file_io.h"

#define SENTIMENT_LEXICON "datasets/vader_lexicon.csv"
#define COINS_FILE        "datasets/coins_queries.csv"
#define ALPHA 15

using namespace std;

int main(int argc, char *argv[]) {
	// Vector tweets
	vector<Tweet> tweets;

	char const *inputFile = "datasets/tweets_dataset_small.csv";

	// Read input file
	unsigned int neighbors = 20;
	int res;
	if ((res = readInputFile(inputFile, tweets, neighbors)) == IO_GENERAL_ERROR) {
		cerr << "[-] Error while reading input file: " << inputFile << endl;
		return -1;
	} else if (res == IO_NOT_UNIQUE) {
		cerr << "[-] Input file tweet IDs are not unique" << endl;
		return -1;
	} else if (res == IO_NOT_INCREASING) {
		cerr << "[-] Input file tweet or user IDs are not in increasing order" << endl;
		return -1;
	}
	cout << "[+] Successfully read " << tweets.size() << " lines" << endl;
	cout << "[+] P = " << neighbors << "\n" << endl;


	// Create sentiment lexicon (map words to sentiment value)
	// using the vader lexicon
	std::unordered_map<std::string, double> sentimentMap;
	if (readSentimentLexicon(SENTIMENT_LEXICON, sentimentMap) == false) {
		cerr << "[-] Error while reading sentiment lexicon: " << SENTIMENT_LEXICON << endl;
		return -1;
	}

	// Read the file with the coins and their alternative names
	std::vector< std::vector<std::string> > coins;
	if (readCoins(COINS_FILE, coins) == false) {
		cerr << "[-] Error while reading coin list: " << COINS_FILE << endl;
		return -1;
	}

	// Calculate the sentiment for every cryptocoin for every tweet
	for (unsigned int i = 0; i < tweets.size(); i++) {
		tweets[i].calculateSentiments(sentimentMap, ALPHA, coins);
	}


	// Create recommendation system
	Recommendation *rec = new Recommendation(tweets, neighbors, 10, 10);
	vector<int> clusterOptions;
	clusterOptions.push_back(ClusteringRecommender::DEFAULT_CLUSTERS);
	clusterOptions.push_back(10);
	clusterOptions.push_back(50);
	clusterOptions.push_back(100);
	clusterOptions.push_back(250);

	vector<int> results = rec->findBestClusters(clusterOptions);
	cout << "Best number of clusters for real users: " << results[0] << endl;
	cout << "Best number of clusters for virtual users: " << results[1] << endl;

	delete rec;
	return 0;
}
