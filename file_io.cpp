#include <fstream>
#include <iomanip> // setprecision
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib> // atoi
#include <set>
#include <algorithm> // all_of
#include <cctype> // isspace
#include "tweet.h"
#include "file_io.h"

/* Read the tweets and return the number of tweets read */
int readInputFile(const char *filename, std::vector<Tweet>& tweets, unsigned int& neighbors) {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return 0;
	}


	// Set of tweet IDs to check if they are unique
	std::set<unsigned int> tweetIDs;
	// Used to check if IDs are in increasing order
	int prevUserID = -1;
	int prevTweetID = -1;


	unsigned int dimensions = 0;
	std::string line;

	// Skip lines that are empty or contain only whitespaces
	do {
		if (!getline(inputFile, line)) {
			return 0;
		}
	} while (line == "" || std::all_of(line.begin(), line.end(), isspace));

	// Get the first line as a stringstream
	std::istringstream lineStream(line);
	std::string temp;
	// Get number of neighbors if it is given in the first line
	if (!getline(lineStream, temp, ' ')) {
		return 0;
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
			return 0;
		}

		// First tweet ID
		unsigned int tweetID = tweet.getID();
		unsigned int userID = tweet.getUser();
		tweetIDs.insert(tweetID);

		prevTweetID = tweetID;
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
			return 0;
		}

		// Check if tweet ID already exists
		unsigned int tweetID = tweet.getID();
		if (tweetIdDs.insert(tweetID).second == false) {
			return -1;
		}

		// Check if tweet ID and user ID are in increasing order
		if (tweetID <= prevTweetID) {
			return -2;
		}
		prevTweetID = tweetID;
		unsigned int userID = tweet.getUser();
		if (userID < prevUserID) {
			return -2;
		}

		tweets.push_back(tweet);
	}

	inputFile.close();
	return tweets.size();
}
