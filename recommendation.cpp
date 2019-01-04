#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <utility> // std::pair, std::make_pair
#include <algorithm> // std::fill, std::random_shuffle
#include <random>
#include <chrono>
#include "recommendation.h"
#include "tweet.h"
#include "clustering.h"
#include "cosine_lsh_recommender.h"
#include "clustering_recommender.h"
#include "data_point.h"
#include "metrics.h"

const char *Recommendation::PROCESSED_TWEETS_FILENAME = "datasets/twitter_dataset_small_v2.csv";

Recommendation::Recommendation(const std::vector<Tweet>& tweets, unsigned int neighbors, int usersNumClusters, int virtualNumClusters) : kMeans(NULL) {
	std::cout << "[*] Creating sentiment scores based on users" << std::endl;
	createUserSentiments(tweets);
	std::cout << "[*] Creating sentiment scores based on clusters" << std::endl;
	createClusterSentiments(tweets);

	rec1 = new CosineLSHRecommender(neighbors);
	rec1->train(userSentiments, usersAverageSentiment, clusterSentiments, clustersAverageSentiment);
	rec2 = new ClusteringRecommender(usersNumClusters, virtualNumClusters, neighbors);
	rec2->train(userSentiments, usersAverageSentiment, clusterSentiments, clustersAverageSentiment);
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
			unsigned int knownCount = 0;
			bool nonZero = false;
			for (unsigned int j = 0; j < userSentiment.size(); j++) {
				if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
					if (userSentiment[j] != 0) {
						nonZero = true;
					}
					knownCount++;
					sum += userSentiment[j];
				}
			}

			if (knownCount > 0 && nonZero) { // Keep only users with at least one sentiments for a coin
				usersAverageSentiment.push_back(sum / knownCount);

				// Set the coins with unknown rating to the user's average
				for (unsigned int j = 0; j < userSentiment.size(); j++) {
					if (userSentiment[j] == Tweet::SENTIMENT_NOT_SET) {
						unknownCoins[currUser].insert(j);
						userSentiment[j] = sum / knownCount;
					}
				}

				DataPoint userPoint(userSentiment, std::to_string(currUser));
				userSentiments.push_back(userPoint);
				userToSentiment[currUser] = userSentiments.size() - 1;
			}

			currUser = tweets[i].getUser();
			for (unsigned int j = 0; j < sentiment.size(); j++) {
				userSentiment[j] = sentiment[j];
			}
		}
	}

	// Save last user's total sentiment
	// Calculate average sentiment of the user
	double sum = 0.0;
	unsigned int knownCount = 0;
	bool nonZero = false;
	for (unsigned int j = 0; j < userSentiment.size(); j++) {
		if (userSentiment[j] != Tweet::SENTIMENT_NOT_SET) {
			if (userSentiment[j] != 0) {
				nonZero = true;
			}
			knownCount++;
			sum += userSentiment[j];
		}
	}

	if (knownCount > 0 && nonZero) {
		usersAverageSentiment.push_back(sum / knownCount);

		// Set the coins with unknown rating to the user's average
		for (unsigned int j = 0; j < userSentiment.size(); j++) {
			if (userSentiment[j] == Tweet::SENTIMENT_NOT_SET) {
				unknownCoins[tweets[tweets.size()-1].getUser()].insert(j);
				userSentiment[j] = sum / knownCount;
			}
		}

		DataPoint userPoint(userSentiment, std::to_string(tweets[tweets.size()-1].getUser()));
		userSentiments.push_back(userPoint);
		userToSentiment[tweets[tweets.size()-1].getUser()] = userSentiments.size() - 1;
	}
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
		unsigned int knownCount = 0;
		bool nonZero = false;
		for (unsigned int k = 0; k < clusterSentiment.size(); k++) {
			if (clusterSentiment[k] != Tweet::SENTIMENT_NOT_SET) {
				if (clusterSentiment[k] != 0) {
					nonZero = true;
				}
				knownCount++;
				sum += clusterSentiment[k];
			}
		}

		if (knownCount > 0 && nonZero) {
			clustersAverageSentiment.push_back(sum / knownCount);

			// Set the coins with unknown rating to the user's average
			for (unsigned int j = 0; j < clusterSentiment.size(); j++) {
				if (clusterSentiment[j] == Tweet::SENTIMENT_NOT_SET) {
					clusterSentiment[j] = sum / knownCount;
				}
			}

			DataPoint clusterPoint(clusterSentiment, "C-" + std::to_string(i+1));
			clusterSentiments.push_back(clusterPoint);
		}
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
			continue;
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
std::vector<std::string> Recommendation::cosineLSHRecommendations(unsigned int userID, const std::vector< std::vector<std::string> >& coins) const {
	if (userToSentiment.find(userID) == userToSentiment.end()) {
		std::vector<std::string> empty;
		return empty;
	}
	unsigned int userIndex = userToSentiment.at(userID);
	std::vector<unsigned int> result = rec1->recommendations(userSentiments[userIndex], unknownCoins.at(userID));

	std::vector<std::string> recommendedCoins;
	for (unsigned int i = 0; i < result.size(); i++) {
		recommendedCoins.push_back(coins[result[i]][0]);
	}
	return recommendedCoins;
}



/* Combine user based and cluster based recommendations */
std::vector<std::string> Recommendation::clusteringRecommendations(unsigned int userID, const std::vector< std::vector<std::string> >& coins) const {
	if (userToSentiment.find(userID) == userToSentiment.end()) {
		std::vector<std::string> empty;
		return empty;
	}
	unsigned int userIndex = userToSentiment.at(userID);
	std::vector<unsigned int> result = rec2->recommendations(userSentiments[userIndex], unknownCoins.at(userID));

	std::vector<std::string> recommendedCoins;
	for (unsigned int i = 0; i < result.size(); i++) {
		recommendedCoins.push_back(coins[result[i]][0]);
	}
	return recommendedCoins;
}



std::vector<double> Recommendation::validate() {
	// Total error for each recommendation method
	std::vector<double> totalError(2);
	std::fill(totalError.begin(), totalError.end(), 0.0);

	// Create a list of all rated coins of each user and their old rating
	std::vector< std::pair<unsigned int, unsigned int> > ratedCoins;
	std::vector< std::unordered_map<unsigned int, double> > oldRatings;
	std::vector<double> oldAverages;
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		std::unordered_map<unsigned int, double> userRatings;
		for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
			if (unknownCoins[i].find(j) == unknownCoins[i].end()) { // Coin is rated (not in unknown coins)
				ratedCoins.push_back(std::make_pair(i, j));
				userRatings[j] = userSentiments[i].at(j);
			}
		}
		oldRatings.push_back(userRatings);
		oldAverages.push_back(usersAverageSentiment[i]);
	}

	// Create a vector of all the rated coins indices
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < ratedCoins.size(); i++) {
		indices.push_back(i);
	}

	// Create a new map of unrated coins (chosen for validation)
	std::unordered_map<unsigned int, std::set<unsigned int> > validationCoins;

	// Shuffle the vector
	srand(time(NULL));
	std::random_shuffle(indices.begin(), indices.end());

	// Split the set of rated coins to 10 equal size subsets.
	// Each time the 1/10 is the validation set and the 9/10 is the training set
	double LSHUserBasedError = 0.0;
	double clusterUserBasedError = 0.0;
	unsigned int foldSize = ratedCoins.size() / 10;
	for (unsigned int fold = 0; fold < 10; fold++) {
		double LSHDiffSum = 0.0;
		double clusterDiffSum = 0.0;

		unsigned int start = fold * foldSize;
		unsigned int end = ((fold != 9) ? start + foldSize : ratedCoins.size());

		// Create the validation set
		for (unsigned int i = start; i < end; i++) {
			unsigned int pair = indices[i];
			unsigned int user = ratedCoins[pair].first;
			unsigned int coin = ratedCoins[pair].second;

			// Insert the coin in the unknown coins set of the user
			validationCoins[user].insert(coin);
		}

		// Calculate the new averages of the users
		for (unsigned int i = 0; i < userSentiments.size(); i++) {
			double sum = 0.0;
			unsigned int knownCount = 0;
			for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
				if (validationCoins[i].find(j) == validationCoins[i].end() && unknownCoins[i].find(j) == unknownCoins[i].end()) {
					sum += userSentiments[i].at(j);
					knownCount++;
				}
			}
			usersAverageSentiment[i] = sum / knownCount;
		}

		// Replace validation coins and unknown coins ratings with new average
		for (unsigned int i = start; i < end; i++) {
			unsigned int pair = indices[i];
			unsigned int user = ratedCoins[pair].first;
			unsigned int coin = ratedCoins[pair].second;

			userSentiments[user][coin] = usersAverageSentiment[user];
		}
		for (unsigned int i = 0; i < userSentiments.size(); i++) {
			if (unknownCoins.find(i) != unknownCoins.end()) {
				for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
					if (unknownCoins[i].find(j) != unknownCoins[i].end()) {
						userSentiments[i][j] = usersAverageSentiment[i];
					}
				}
			}
		}

		// Retrain the recommendation systems with the new data
		rec1->train(userSentiments, usersAverageSentiment, clusterSentiments, clustersAverageSentiment);
		rec2->train(userSentiments, usersAverageSentiment, clusterSentiments, clustersAverageSentiment);

		// Get the ratings for the unknown coins
		for (unsigned int i = 0; i < userSentiments.size(); i++) {
			if (validationCoins.find(i) != validationCoins.end() && validationCoins.at(i).size() > 0) { // User has at least one unrated coin
				std::unordered_map<unsigned int, double> LSHResults = rec1->userBasedPredictions(userSentiments[i], validationCoins[i]);
				std::unordered_map<unsigned int, double> clusterResults = rec2->userBasedPredictions(userSentiments[i], validationCoins[i]);
				for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
					if (validationCoins[i].find(j) != validationCoins[i].end()) {
						LSHDiffSum += abs(LSHResults[j] - oldRatings[i].at(j));
						clusterDiffSum += abs(clusterResults[j] - oldRatings[i].at(j));
						std::cout << LSHResults[j] << "    " << oldRatings[i].at(j) << std::endl;
						std::cout << LSHDiffSum << "\n\n";
					}
				}
			}
		}

		LSHUserBasedError += (1.0 / (end - start)) * LSHDiffSum;
		clusterUserBasedError += (1.0 / (end - start)) * clusterDiffSum;

		// Reset the changed ratings
		for (unsigned int i = start; i < end; i++) {
			unsigned int pair = indices[i];
			unsigned int user = ratedCoins[pair].first;
			unsigned int coin = ratedCoins[pair].second;

			// Set the rating of the coin to its old value
			userSentiments[user][coin] = oldRatings[user].at(coin);
		}
		// Reset the averages
		for (unsigned int i = 0; i < usersAverageSentiment.size(); i++) {
			usersAverageSentiment[i] = oldAverages[i];
			// Set real unknown ratings to the old average
			if (unknownCoins.find(i) != unknownCoins.end()) {
				for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
					if (unknownCoins[i].find(j) != unknownCoins[i].end()) {
						userSentiments[i][j] = oldAverages[i];
					}
				}
			}
		}
		validationCoins.clear();
	}
	// Average error from all the folds
	totalError[0] += LSHUserBasedError / 10; // Average error from all the folds
	totalError[1] += clusterUserBasedError / 10; // Average error from all the folds

	return totalError;
}
