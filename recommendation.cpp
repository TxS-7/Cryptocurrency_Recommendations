#include <fstream>
#include <vector>
#include <set>
#include <algorithm> // std::fill
#include "recommendation.h"
#include "tweet.h"
#include "clustering.h"
#include "data_point.h"

const char *Recommendation::PROCESSES_TWEETS_FILENAME = "datasets/twitter_dataset_small_v2.csv";

Recommendation::Recommendation(const std::vector<Tweet>& tweets) {
	createUserSentiments(tweets);
	createClusterSentiments(tweets);
}



/* Create a total sentiment for every user based on his tweets */
void createUserSentiments(const std::vector<Tweet>& tweets) {
	// Create a vector for every user as the sum of the sentiments of each one of his/her tweets
	std::vector<double> userSentiment(tweets[0].getSentiment().size());
	std::fill(userSentiment.begin(), userSentiment.end(), Tweet::SENTIMENT_NOT_SET);

	// NOTE: User IDs must be grouped
	unsigned int currUser = tweets[0].getUser();
	for (unsigned int i = 0; i < tweets.size(); i++) {
		std::vector<double> sentiment = tweets[i].getSentiment();
		if (tweets[i].getUser() == currUser) {
			for (unsigned int j = 0; j < sentiment.size(); j++) {
				if (sentiment[j] != Tweet::SENTIMENT_NOT_SET) {
					if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
						userSentiment[j] += sentiment[j];
					} else {
						userSentiment[j] = sentiment[j];
					}
				}
			}
		} else { // New user: save current user sentiment and reset it
			// Calculate average sentiment of the user
			double sum = 0.0;
			for (unsigned int j = 0; j < userSentiment.size(); j++) {
				if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
					sum += userSentiment[j];
				}
			}
			usersAverageSentiment.push_back(sum / userSentiment.size());

			DataPoint userPoint(userSentiment, std::to_string(currUser));
			userSentiments.push_back(userPoint);
			currUser = tweets[i].getUser();

			for (unsigned int j = 0; j < sentiment.size(); j++) {
				userSentiment[j] = sentiment[j];
			}
		}
	}

	// Save last user's total sentiment
	// Calculate average sentiment of the user
	double sum = 0.0;
	for (unsigned int j = 0; j < userSentiment.size(); j++) {
		if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
			sum += userSentiment[j];
		}
	}
	usersAverageSentiment.push_back(sum / userSentiment.size());

	DataPoint userPoint(userSentiment, std::to_string(tweets[tweets.size()-1].getUser()));
	userSentiments.push_back(userPoint);
}



/* Place all processed tweets into clusters using K-means and create a total
 * sentiment for each cluster */
void createClusterSentiments(const std::vector<Tweet>& tweets) {
	// Create a vector for every cluster as the sum of the sentiments of its tweets
	std::vector<double> clusterSentiment(tweets[0].getSentiment().size());
	std::fill(clusterSentiment.begin(), clusterSentiment.end(), Tweet::SENTIMENT_NOT_SET);

	// NOTE: User IDs must be grouped
	unsigned int currUser = tweets[0].getUser();
	for (unsigned int i = 0; i < tweets.size(); i++) {
		std::vector<double> sentiment = tweets[i].getSentiment();
		if (tweets[i].getUser() == currUser) {
			for (unsigned int j = 0; j < sentiment.size(); j++) {
				if (sentiment[j] != Tweet::SENTIMENT_NOT_SET) {
					if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
						userSentiment[j] += sentiment[j];
					} else {
						userSentiment[j] = sentiment[j];
					}
				}
			}
		} else { // New user: save current user sentiment and reset it
			// Calculate average sentiment of the user
			double sum = 0.0;
			for (unsigned int j = 0; j < userSentiment.size(); j++) {
				if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
					sum += userSentiment[j];
				}
			}
			usersAverageSentiment.push_back(sum / userSentiment.size());

			DataPoint userPoint(userSentiment, std::to_string(currUser));
			userSentiments.push_back(userPoint);
			currUser = tweets[i].getUser();

			for (unsigned int j = 0; j < sentiment.size(); j++) {
				userSentiment[j] = sentiment[j];
			}
		}
	}

	// Save last user's total sentiment
	// Calculate average sentiment of the user
	double sum = 0.0;
	for (unsigned int j = 0; j < userSentiment.size(); j++) {
		if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
			sum += userSentiment[j];
		}
	}
	usersAverageSentiment.push_back(sum / userSentiment.size());

	DataPoint userPoint(userSentiment, std::to_string(tweets[tweets.size()-1].getUser()));
	userSentiments.push_back(userPoint);
}



/* Read the tweets after they have passed through TF-IDF vectorization and SVD */
bool Recommendation::readProcessedTweets(const char *filename, std::vector<DataPoint>& points) const {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return 0;
	}

	// Set of IDs to check if they are unique
	std::set<std::string> ids;

	unsigned int dimensions = 0;
	int i = 0;
	std::string line;

	while (getline(inputFile, line)) {
		// Skip lines that are empty or contain only whitespaces
		if (line == "" || std::all_of(line.begin(), line.end(), isspace)) {
			continue;
		}

		DataPoint point;
		if (point.readDataPoint(line) == false) {
			return 0;
		}

		// Check if ID already exists
		if (ids.insert(point.getID()).second == false) {
			return -1;
		}

		points.push_back(point);
		if (i == 0) {
			dimensions = point.getDimensions();
		} else { // Dimensions of different points don't match
			if (point.getDimensions() != dimensions) {
				return 0;
			}
		}
		i++;
	}

	inputFile.close();
	return i;
}
