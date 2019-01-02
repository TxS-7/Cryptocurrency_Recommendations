#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm> // std::fill
#include "recommendation.h"
#include "tweet.h"
#include "clustering.h"
#include "data_point.h"
#include "metrics.h"

const char *Recommendation::PROCESSED_TWEETS_FILENAME = "datasets/twitter_dataset_small_v2.csv";

Recommendation::Recommendation(const std::vector<Tweet>& tweets) {
	std::cout << "[*] Creating sentiment scores based on users" << std::endl;
	createUserSentiments(tweets);
	std::cout << "[*] Creating sentiment scores based on clusters" << std::endl;
	createClusterSentiments(tweets);
}



/* Create a total sentiment for every user based on his tweets */
void Recommendation::createUserSentiments(const std::vector<Tweet>& tweets) {
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
void Recommendation::createClusterSentiments(const std::vector<Tweet>& tweets) {
	// Map tweet IDs to index to find them faster when we know their ID
	std::unordered_map<std::string, unsigned int> IDToIndex;
	// Set of tweet IDs used to check if the processed tweet IDs match the given tweets
	std::set<std::string> existingIDs;
	for (unsigned int i = 0; i < tweets.size(); i++) {
		std::string strID = std::to_string(tweets[i].getID());
		IDToIndex[strID] = i;
		existingIDs.insert(strID);
	}


	// Get the processed tweets
	if (readProcessedTweets(PROCESSED_TWEETS_FILENAME, processedTweets, existingIDs) == false) {
		std::cerr << "[-] Error while reading processed tweets file: " << PROCESSED_TWEETS_FILENAME << std::endl;
		exit(-1);
	}

	// Perform clustering of the tweets using K-means
	kMeans = new KMeansClustering(processedTweets, NUMBER_OF_CLUSTERS, Metrics::COSINE);
	kMeans->run();

	// Get the point IDs of each cluster
	std::vector< std::vector<std::string> > clusters;
	kMeans->getPointsPerCluster(clusters);


	// Create a vector for every cluster as the sum of the sentiments of its tweets
	std::vector<double> clusterSentiment(tweets[0].getSentiment().size());
	// Create total sentiment for every cluster
	for (unsigned int i = 0; i < clusters.size(); i++) { // Each cluster
		std::fill(clusterSentiment.begin(), clusterSentiment.end(), Tweet::SENTIMENT_NOT_SET);
		for (unsigned int j = 0; j < clusters[i].size(); j++) { // Each point in cluster
			unsigned int tweetIndex = IDToIndex[clusters[i][j]];
			std::vector<double> sentiment = tweets[tweetIndex].getSentiment();

			for (unsigned int k = 0; k < sentiment.size(); k++) { // Each coin sentiment
				if (sentiment[k] != Tweet::SENTIMENT_NOT_SET) {
					if (clusterSentiment[k] != Tweet::SENTIMENT_NOT_SET) {
						clusterSentiment[k] += sentiment[k];
					} else {
						clusterSentiment[k] = sentiment[k];
					}
				}
			}
		}

		// Calculate average sentiment of the cluster
		double sum = 0.0;
		for (unsigned int k = 0; k < clusterSentiment.size(); k++) {
			if (clusterSentiment[k] != Tweet::SENTIMENT_NOT_SET) {
				sum += clusterSentiment[k];
			}
		}
		clustersAverageSentiment.push_back(sum / clusterSentiment.size());

		// Convert total cluster sentiment vector to DataPoint and store it
		DataPoint clusterPoint(clusterSentiment, std::to_string(i+1));
		clusterSentiments.push_back(clusterPoint);
	}
}



/* Read the tweets after they have passed through TF-IDF vectorization and SVD */
bool Recommendation::readProcessedTweets(const char *filename, std::vector<DataPoint>& points, std::set<std::string>& existingIDs) const {
	// Open file for reading
	std::ifstream inputFile;
	inputFile.open(filename);
	if (!inputFile) {
		return false;
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
			return false;
		}

		// Check if ID already exists
		if (ids.insert(point.getID()).second == false) {
			return false;
		}

		// Check if the ID already exists in the non-processed tweets
		if (existingIDs.insert(point.getID()).second != false) {
			return false;
		}

		points.push_back(point);
		if (i == 0) {
			dimensions = point.getDimensions();
		} else { // Dimensions of different points don't match
			if (point.getDimensions() != dimensions) {
				return false;
			}
		}
		i++;
	}

	inputFile.close();
	return true;
}



/* Combine user based and cluster based recommendations */
std::vector< std::vector<unsigned int> > Recommendation::recommendations() const {
	std::cout << "\nCreating recommendations based on users" << std::endl;
	std::vector< std::vector<unsigned int> > result = userBasedRecommendations();
	std::cout << "Creating recommendations based on clusters" << std::endl;
	std::vector< std::vector<unsigned int> > result2 = clusterBasedRecommendations();

	for (unsigned int i = 0; i < result.size(); i++) {
		// Used to avoid duplicate coins from user/cluster based recommendations
		std::set<unsigned int> foundCoins;
		for (unsigned int j = 0; j < result[i].size(); j++) {
			foundCoins.insert(result[i][j]);
		}
		for (unsigned int j = 0; j < result2[i].size(); j++) {
			if (foundCoins.insert(result2[i][j]).second != false) {
				result[i].push_back(result2[i][j]);
			}
		}
	}
	return result;
}
