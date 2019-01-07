#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm> // std::sort
#include <utility> // std::pair, std::make_pair
#include <cmath> // std::abs
#include "clustering_recommender.h"
#include "clustering.h"
#include "data_point.h"
#include "metrics.h"
#include "util.h"

const int ClusteringRecommender::DEFAULT_CLUSTERS = -1;

void ClusteringRecommender::train(std::vector<DataPoint>& userSentiments, const std::vector<double>& usersAvg, std::vector<DataPoint>& clusterSentimentsArg, const std::vector<double>& clustersAvg) {
	// Save the averages
	usersAverageSentiment.clear();
	userToSentiment.clear();
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		usersAverageSentiment.push_back(usersAvg[i]);
		userToSentiment[userSentiments[i].getID()] = i;
	}
	clustersAverageSentiment.clear();
	clusterToSentiment.clear();
	for (unsigned int i = 0; i < clusterSentimentsArg.size(); i++) {
		clusterSentiments.push_back(clusterSentimentsArg[i]);
		clustersAverageSentiment.push_back(clustersAvg[i]);
		clusterToSentiment[clusterSentimentsArg[i].getID()] = i;
	}


	// Create the clusters for user based and cluster based recommendations
	if (realUsersClusters != NULL) {
		delete realUsersClusters;
	}
	int newNumClusters = numberOfRealUserClusters;
	if (numberOfRealUserClusters == DEFAULT_CLUSTERS) {
		newNumClusters = userSentiments.size() / P;
	}
	realUsersClusters = new KMeansClustering(userSentiments, newNumClusters, Metrics::EUCLIDEAN);
	realUsersClusters->run();
}



/* Combine user based and cluster based recommendations */
std::vector<unsigned int> ClusteringRecommender::recommendations(const DataPoint& user, const std::set<unsigned int>& unknown) {
	if (userToSentiment.find(user.getID()) == userToSentiment.end() || unknown.size() == 0) {
		std::vector<unsigned int> empty;
		return empty;
	}

	std::vector<unsigned int> result1 = userBasedRecommendations(user, unknown);
	std::vector<unsigned int> result2 = clusterBasedRecommendations(user, unknown);
	for (unsigned int i = 0; i < result2.size(); i++) {
		result1.push_back(result2[i]);
	}
	return result1;
}



/* Return the indices of the recommended coins for a user (top 5)
 * based on total sentiment per user */
std::vector<unsigned int> ClusteringRecommender::userBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the predicted ratings of the unrated coins
	std::vector< std::pair<double, unsigned int> > predictions = userBasedPredictions(user, unknown);

	// Sort in descending order using lambda
	std::sort(predictions.begin(), predictions.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
		return left.first > right.first;
	});

	// Get the top 5 coins based on sentiment
	std::vector<unsigned int> recommendedCoins;
	for (unsigned int j = 0; j < min(5, predictions.size()); j++) {
		recommendedCoins.push_back(predictions[j].second);
	}

	return recommendedCoins;
}



/* Return the indices of the recommended coins for a user (top 2)
 * based on total sentiment per user */
std::vector<unsigned int> ClusteringRecommender::clusterBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) {
	// Get the predicted ratings of the unrated coins
	std::vector< std::pair<double, unsigned int> > predictions = clusterBasedPredictions(user, unknown);

	// Sort in descending order using lambda
	std::sort(predictions.begin(), predictions.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
		return left.first > right.first;
	});

	// Get the top 2 coins based on sentiment
	std::vector<unsigned int> recommendedCoins;
	for (unsigned int j = 0; j < min(2, predictions.size()); j++) {
		recommendedCoins.push_back(predictions[j].second);
	}

	return recommendedCoins;
}



/* Return the predicted score for the given unknown coin ratings */
std::vector< std::pair<double, unsigned int> > ClusteringRecommender::userBasedPredictions(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the users in the same cluster as this user
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors = realUsersClusters->getPointsInSameCluster(user);

	std::vector< std::pair<double, unsigned int> > predictions;
	if (neighbors.size() < 2) { // Nothing in cluster or only user
		return predictions;
	}

	// Guess the sentiment of coins without sentiment based on the neighbors
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z and ratings
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < neighbors.size(); k++) {
				// Exclude same user
				if (neighbors[k]->getID() != user.getID()) {
					z_sum += std::abs(Metrics::euclideanSimilarity(user, *neighbors[k]));
					unsigned int neighborIndex = userToSentiment.at(neighbors[k]->getID());
					sum += Metrics::euclideanSimilarity(user, *neighbors[k]) * (neighbors[k]->at(j) - usersAverageSentiment[neighborIndex]);
				}
			}

			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			predictions.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	return predictions;
}



/* Return the predicted score for the given unknown coin ratings */
std::vector< std::pair<double, unsigned int> > ClusteringRecommender::clusterBasedPredictions(const DataPoint& user, const std::set<unsigned int>& unknown) {
	// Perform the clustering of this user with all the virtual users
	int newNumClusters = numberOfVirtualUserClusters;
	if (numberOfVirtualUserClusters == DEFAULT_CLUSTERS) {
		newNumClusters = clusterSentiments.size() / P;
	}
	// Add the user to the virtual users
	clusterSentiments.push_back(user);
	virtualUsersClusters = new KMeansClustering(clusterSentiments, newNumClusters, Metrics::EUCLIDEAN);
	virtualUsersClusters->run();


	// Get the virtual users in the same cluster as this user
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors = virtualUsersClusters->getPointsInSameCluster(user);

	std::vector< std::pair<double, unsigned int> > predictions;
	if (neighbors.size() < 2) { // Nothing in cluster or only user
		// Remove user from virtual users again
		clusterSentiments.pop_back();
		delete virtualUsersClusters;
		virtualUsersClusters = NULL;
		return predictions;
	}

	// Guess the sentiment of coins without sentiment based on the neighbors
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z and ratings
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < neighbors.size(); k++) {
				// Exclude the user
				if (neighbors[k]->getID() != user.getID()) {
					z_sum += std::abs(Metrics::euclideanSimilarity(user, *neighbors[k]));
					unsigned int neighborIndex = clusterToSentiment.at(neighbors[k]->getID());
					sum += Metrics::euclideanSimilarity(user, *neighbors[k]) * (neighbors[k]->at(j) - clustersAverageSentiment[neighborIndex]);
				}
			}

			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			predictions.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	// Remove user from virtual users again
	clusterSentiments.pop_back();

	delete virtualUsersClusters;
	virtualUsersClusters = NULL;

	return predictions;
}



/* Find the number of clusters from the options with the best silhouette evaluation */
std::vector<int> ClusteringRecommender::findBestClusters(const std::vector<int>& options, std::vector<DataPoint>& userSentiments, std::vector<DataPoint>& clusterSentiments) const {
	// Find best for real users
	double max = 0.0;
	int maxClusters = 0;
	for (auto num : options) {
		if (num == DEFAULT_CLUSTERS) {
			num = usersAverageSentiment.size() / P;
		}

		if ((unsigned int) num <= usersAverageSentiment.size() && num > 1) {
			KMeansClustering *clustering = new KMeansClustering(userSentiments, num, Metrics::EUCLIDEAN);
			clustering->run();
			
			std::vector<double> temp;
			double silhouette = clustering->silhouette(temp);
			std::cout << "Silhouette for " << num << " is: " << silhouette << std::endl;
			if (silhouette > max) {
				max = silhouette;
				maxClusters = num;
			}

			delete clustering;
		}
	}
	std::vector<int> result;
	result.push_back(maxClusters);

	// Find best for virtual users
	max = 0.0;
	maxClusters = 0;
	for (auto num : options) {
		if ((unsigned int) num <= clustersAverageSentiment.size() && num > 1) {
			if (num == DEFAULT_CLUSTERS) {
				num = clustersAverageSentiment.size() / P;
			}

			KMeansClustering *clustering = new KMeansClustering(clusterSentiments, num, Metrics::EUCLIDEAN);
			clustering->run();
			
			std::vector<double> temp;
			double silhouette = clustering->silhouette(temp);
			std::cout << "Silhouette for " << num << " is: " << silhouette << std::endl;;
			if (silhouette > max) {
				max = silhouette;
				maxClusters = num;
			}

			delete clustering;
		}
	}
	result.push_back(maxClusters);
	return result;
}
