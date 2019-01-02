#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <set>
#include <limits> // std::numeric_limits
#include <algorithm> // std::fill
#include <cctype> // isalnum
#include <cstdlib> // atoi
#include "tweet.h"
#include "util.h"

const double Tweet::SENTIMENT_NOT_SET = std::numeric_limits<double>::infinity();

/* Read the tab-seperated tokens */
bool Tweet::readTweet(const std::string& line) {
	// Check if '\r' is found which is used in Windows OS
	if (line.find('\r') != std::string::npos) {
		std::cerr << "[-] Windows format found" << std::endl;
		return false;
	}

	char delim = '\t';

	// Convert to string stream
	std::istringstream lineStream(line);


	std::string temp;
	// Read the user ID first as a string
	if (!getline(lineStream, temp, delim)) {
		return false;
	}
	// Convert it to an unsigned integer
	int tempID = atoi(temp.c_str());
	if (tempID < 0) {
		return false;
	}
	userID = (unsigned int) tempID;


	// Read the tweet ID as a string
	if (!getline(lineStream, temp, delim)) {
		return false;
	}
	// Convert it to an unsigned integer
	tempID = atoi(temp.c_str());
	if (tempID < 0) {
		return false;
	}
	tweetID = (unsigned int) tempID;


	// Read the tokens
	while (getline(lineStream, temp, delim)) {
		tokens.push_back(temp);
	}

	return true;
}



/* Calculate the sentiment for every coin mentioned from the list using the sentiment lexicon given */
void Tweet::calculateSentiments(const std::unordered_map<std::string, double>& sentimentMap, double alpha, const std::vector< std::vector<std::string> >& coins) {
	sentimentVector.clear();
	sentimentVector.resize(coins.size());
	std::fill(sentimentVector.begin(), sentimentVector.end(), SENTIMENT_NOT_SET);

	double totalScore = 0.0;
	bool foundWord = false; // At least one word found in sentiment lexicon
	std::set<unsigned int> foundCoinsIndices; // Indices of the coins that are mentioned in the tweet

	// Get tweet total score
	for (unsigned int i = 0; i < tokens.size(); i++) {
		// Convert token to lower case
		std::string tokenLower = toLower(tokens[i]);

		// Check if the token is a coin mention
		bool coinFound = false;
		for (unsigned int coinIndex = 0; coinIndex < coins.size() && !coinFound; coinIndex++) {
			for (unsigned int j = 0; j < coins[coinIndex].size(); j++) {
				if (tokenLower.compare(coins[coinIndex][j]) == 0) {
					foundCoinsIndices.insert(coinIndex);
					coinFound = true;
					break;
				}
			}
		}

		// Check if the token exists in the sentiment lexicon
		if (sentimentMap.find(tokenLower) != sentimentMap.end()) {
			foundWord = true;
			totalScore += sentimentMap.at(tokenLower);
		}
	}

	if (foundWord) {
		// Normalize total score
		totalScore = totalScore / sqrt(totalScore * totalScore + alpha);

		// Set the sentiment for the coins mentioned in the tweet
		for (auto coinIndex : foundCoinsIndices) {
			sentimentVector[coinIndex] = totalScore;
		}
	}
}
