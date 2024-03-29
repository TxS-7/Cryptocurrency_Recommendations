#ifndef TWEET_H
#define TWEET_H

#include <vector>
#include <string>
#include <unordered_map>

class Tweet {
private:
	unsigned int tweetID;
	unsigned int userID;
	std::vector<std::string> tokens;
	std::vector<double> sentimentVector;
public:
	static const double SENTIMENT_NOT_SET;

	Tweet() : tweetID(0), userID(0) {}

	bool readTweet(const std::string&);
	void calculateSentiments(const std::unordered_map<std::string, double>&, double alpha, const std::vector< std::vector<std::string> >&);

	unsigned int getID() const { return tweetID; }
	unsigned int getUser() const { return userID; }
	unsigned int getSize() const { return tokens.size(); }
	std::vector<std::string> getTokens() const { return tokens; }
	std::vector<double> getSentiment() const { return sentimentVector; }
};

#endif // TWEET_H
