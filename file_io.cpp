#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility> // std::pair
#include <cstdlib> // atoi, atof
#include <set>
#include <algorithm> // all_of
#include <cctype> // isspace
#include "tweet.h"
#include "file_io.h"
#include "util.h"

/* Read the tweets and return the number of tweets read */
int readInputFile(const char *filename, std::vector<Tweet>& tweets, unsigned int& neighbors) {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return IO_GENERAL_ERROR;
	}


	// Set of tweet IDs to check if they are unique
	std::set<unsigned int> tweetIDs;
	// Used to check if IDs are in increasing order
	unsigned int prevUserID = 0;
	unsigned int prevTweetID = 0;


	std::string line;
	// Skip lines that are empty or contain only whitespaces
	do {
		if (!getline(inputFile, line)) {
			return IO_GENERAL_ERROR;
		}
	} while (line == "" || std::all_of(line.begin(), line.end(), isspace));

	// Get the first line as a stringstream
	std::istringstream lineStream(line);
	std::string temp;
	// Get number of neighbors if it is given in the first line
	if (!getline(lineStream, temp, ' ')) {
		return IO_GENERAL_ERROR;
	}

	if (temp.compare("P:") == 0) {
		// Read the value if it is valid
		int tempNeighbors;
		if (getline(lineStream, temp, ' ') && (tempNeighbors = atoi(temp.c_str())) > 0) {
			neighbors = (unsigned int) tempNeighbors;
		}
	} else {
		// The first line is a tweet
		Tweet tweet;
		if (tweet.readTweet(line) == false) {
			return IO_GENERAL_ERROR;
		}

		// First tweet ID
		unsigned int tweetID = tweet.getID();
		unsigned int userID = tweet.getUser();
		tweetIDs.insert(tweetID);

		prevTweetID = tweetID;
		prevUserID = userID;
		tweets.push_back(tweet);
	}


	// Read the rest of the tweets
	while (getline(inputFile, line)) {
		// Skip lines that are empty or contain only whitespaces
		if (line == "" || std::all_of(line.begin(), line.end(), isspace)) {
			continue;
		}

		Tweet tweet;
		if (tweet.readTweet(line) == false) {
			return IO_GENERAL_ERROR;
		}

		// Check if tweet ID already exists
		unsigned int userID = tweet.getUser();
		unsigned int tweetID = tweet.getID();
		if (tweetIDs.insert(tweetID).second == false) {
			return IO_NOT_UNIQUE;
		}

		// Check if user ID and tweet ID are in increasing order
		if (tweets.size() > 0) { // Exclude first tweet
			if (userID < prevUserID) { // Invalid user ID
				return IO_NOT_INCREASING;
			} else if (userID == prevUserID) { // Same user: check tweet ID
				if (tweetID <= prevTweetID) { // Invalid tweet ID
					return IO_NOT_INCREASING;
				}
			}
		}

		prevUserID = userID;
		prevTweetID = tweetID;

		tweets.push_back(tweet);
	}

	inputFile.close();
	return tweets.size();
}



bool readSentimentLexicon(const char *filename, std::unordered_map<std::string, double>&sentimentMap) {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return false;
	}

	// Read every word mapping
	std::string line;
	while (getline(inputFile, line)) {
		// Skip lines that are empty or contain only whitespaces
		if (line == "" || std::all_of(line.begin(), line.end(), isspace)) {
			continue;
		}

		if (line.find('\r') != std::string::npos) {
			std::cerr << "[-] Windows format found" << std::endl;
			return false;
		}

		// Convert line to stringstream
		std::istringstream lineStream(line);
		std::string word;
		std::string sentiment;
		// Get the word
		if (!getline(lineStream, word, '\t')) {
			return false;
		}
		// Get its sentiment
		if (!getline(lineStream, sentiment)) {
			return false;
		}
		if (!isNumber(sentiment)) {
			return false;
		}

		// Save the word - sentiment mapping
		sentimentMap[word] = atof(sentiment.c_str());
	}

	inputFile.close();
	return true;
}


bool readCoins(const char *filename, std::vector< std::vector<std::string> >& coins) {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return false;
	}

	// Read every word for each cryptocoin
	std::string line;
	while (getline(inputFile, line)) {
		// Skip lines that are empty or contain only whitespaces
		if (line == "" || std::all_of(line.begin(), line.end(), isspace)) {
			continue;
		}

		if (line.find('\r') != std::string::npos) {
			std::cerr << "[-] Windows format found" << std::endl;
			return false;
		}

		std::vector<std::string> coinWords;

		// Convert line to stringstream
		std::istringstream lineStream(line);
		std::string word;
		while (getline(lineStream, word, '\t')) {
			coinWords.push_back(word);
		}
		if (coinWords.size() == 0) {
			return false;
		}

		coins.push_back(coinWords);
	}

	inputFile.close();
	return true;
}


bool writeOutputFile(const char *filename, const std::vector< std::pair<unsigned int, std::vector<std::string> > >& cosineLSHResults, double cosineLSHTime, const std::vector< std::pair<unsigned int, std::vector<std::string> > >& clusteringResults, double clusteringTime) {
	// Open file for writing
	std::fstream outputFile;
	outputFile.open(filename, std::fstream::out);
	if (!outputFile) {
		return false;
	}

	// Cosine LSH recommendations
	outputFile << "Cosine LSH" << std::endl;
	for (auto& user : cosineLSHResults) {
		outputFile << user.first << ": ";
		if (user.second.size() == 0) {
			outputFile << "No results";
		} else {
			for (unsigned int j = 0; j < user.second.size(); j++) {
				outputFile << user.second[j];
				if (j != user.second.size() - 1) {
					outputFile << "\t";
				}
			}
		}
		outputFile << std::endl;
	}
	outputFile << "Execution Time: " << cosineLSHTime << "\n" << std::endl;


	// Clustering recommendations
	outputFile << "Clustering" << std::endl;
	for (auto& user : clusteringResults) {
		outputFile << user.first << ": ";
		if (user.second.size() == 0) {
			outputFile << "No results";
		} else {
			for (unsigned int j = 0; j < user.second.size(); j++) {
				outputFile << user.second[j];
				if (j != user.second.size() - 1) {
					outputFile << "\t";
				}
			}
		}
		outputFile << std::endl;
	}
	outputFile << "Execution Time: " << clusteringTime << std::endl;

	outputFile.close();
	return true;
}
