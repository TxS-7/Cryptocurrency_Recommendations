#ifndef TWEET_H
#define TWEET_H

#include <vector>
#include <string>

class Tweet {
private:
	unsigned int tweetID;
	unsigned int userID;
	std::vector<std::string> tokens;
public:
	Tweet() : tweetID(0), userID(0) {}

	bool readTweet(const std::string&);

	unsigned int getID() const { return tweetID; }
	unsigned int getUser() const { return userID; }
	unsigned int getSize() const { return tokens.size(); }
	std::vector<std::string> getTokens() const { return tokens; }

	//calculateSentiment() const;
};

#endif // TWEET_H
