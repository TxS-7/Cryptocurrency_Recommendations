#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <utility> // pair
#include <cstring> // strcmp, strncpy
#include <climits> // PATH_MAX
#include <ctime> // clock
#include "tweet.h"
#include "recommendation.h"
#include "file_io.h"
#include "util.h"

#define SENTIMENT_LEXICON "datasets/vader_lexicon.csv"
#define COINS_FILE        "datasets/coins_queries.csv"
#define ALPHA 15

using namespace std;

static void usage(char *);

int main(int argc, char *argv[]) {
	// Parse command line arguments
	bool got_input_file = false;
	bool got_output_file = false;
	bool got_validate = false;

	char inputFile[PATH_MAX];
	char outputFile[PATH_MAX];

	if (argc > 6) {
		usage(argv[0]);
		return -1;
	}


	int i;
	for (i = 1; i < argc; i += 2) {
		if (strcmp(argv[i], "-d") == 0 && !got_input_file) {
			got_input_file = true;
			strncpy(inputFile, argv[i+1], PATH_MAX-1);
			inputFile[PATH_MAX-1] = '\0';

			if (!fileAccessible(inputFile)) {
				cerr << "[-] Input file: " << inputFile << " is not accessible" << endl;
				return -1;
			}
		} else if (strcmp(argv[i], "-o") == 0 && !got_output_file) {
			got_output_file = true;
			strncpy(outputFile, argv[i+1], PATH_MAX-1);
			outputFile[PATH_MAX-1] = '\0';
		} else if (strcmp(argv[i], "-validate") == 0 && !got_validate) {
			got_validate = true;
		} else {
			usage(argv[0]);
			return -1;
		}
	}


	if (!got_input_file || !got_output_file) {
		usage(argv[0]);
		return -1;
	}


	// Vector tweets
	vector<Tweet> tweets;

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

	set<unsigned int> userIDs;
	// Calculate the sentiment for every cryptocoin for every tweet
	for (unsigned int i = 0; i < tweets.size(); i++) {
		tweets[i].calculateSentiments(sentimentMap, ALPHA, coins);
		userIDs.insert(tweets[i].getUser());
	}


	// Create recommendation system
	Recommendation *rec = new Recommendation(tweets, neighbors, ClusteringRecommender::DEFAULT_CLUSTERS, 10);

	// Remove previous contents of the output file
	if (emptyFile(outputFile) == false) {
		cerr << "[-] Couln't write to the output file: " << outputFile << endl;
		return -1;
	}


	clock_t start;
	clock_t end;

	// Cosine LSH results
	double cosineLSHTime = 0.0;
	vector< pair<unsigned int, vector<string> > > cosineLSHResults;
	cout << "\n[*] Running Cosine LSH Recommendations" << endl;
	for (auto user : userIDs) {
		start = clock();
		vector<string> results = rec->cosineLSHRecommendations(user, coins);
		end = clock();
		cosineLSHTime += ((double) (end - start)) / (CLOCKS_PER_SEC / 1000);
		cosineLSHResults.push_back(make_pair(user, results));
	}

	// Clustering results
	double clusteringTime = 0.0;
	vector< pair<unsigned int, vector<string> > > clusteringResults;
	cout << "\n[*] Running Clustering Recommendations" << endl;
	for (auto& user : userIDs) {
		start = clock();
		vector<string> results = rec->clusteringRecommendations(user, coins);
		end = clock();
		clusteringTime += ((double) (end - start)) / (CLOCKS_PER_SEC / 1000);
		clusteringResults.push_back(make_pair(user, results));
	}

	if (writeOutputFile(outputFile, cosineLSHResults, cosineLSHTime, clusteringResults, clusteringTime) == false) {
		cerr << "[-] Error while writing to the output file: " << outputFile << endl;
		return -1;
	}


	if (got_validate) {
		cout << "[*] Performing validation for Cosine LSH" << endl;
		cout << "Cosine LSH Recommendation MAE: " << rec->validateCosineLSH() << endl;
		cout << "[*] Performing validation for Clustering" << endl;
		cout << "Clustering Recommendation MAE: " << rec->validateClustering() << endl;
	}

	cout << "[!] Exiting the program..." << endl;
	delete rec;
	return 0;
}


void usage(char *name) {
	cout << "Usage: " << name << " -d <input file> -o <output file> [-validate]" << endl;
}
