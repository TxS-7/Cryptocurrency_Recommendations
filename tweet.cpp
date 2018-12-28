#include <iostream>
#include <string>
#include <sstream>
#include <cctype> // isalnum
#include <cstdlib> // atoi
#include "tweet.h"

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
