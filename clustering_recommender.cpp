#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <algorithm> // std::sort
#include <utility> // std::pair, std::make_pair
#include <cmath> // abs
#include "clustering_recommender.h"
#include "clustering.h"
#include "data_point.h"
#include "metrics.h"
#include "util.h"

const int ClusteringRecommender::DEFAULT_CLUSTERS = -1;

void ClusteringRecommender::train(std::vector<DataPoint>& userSentiments, const std::vector<double>& usersAvg, std::vector<DataPoint>& clusterSentiments, const std::vector<double>& clustersAvg) {
	// Save the averages
	usersAverageSentiment.clear();
	userToSentiment.clear();
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		usersAverageSentiment.push_back(usersAvg[i]);
		userToSentiment[userSentiments[i].getID()] = i;
	}
	clustersAverageSentiment.clear();
	clusterToSentiment.clear();
	for (unsigned int i = 0; i < clusterSentiments.size(); i++) {
		clustersAverageSentiment.push_back(clustersAvg[i]);
		clusterToSentiment[clusterSentiments[i].getID()] = i;
	}


	// Create the clusters for user based and cluster based recommendations
	if (realUsersClusters != NULL) {
		delete realUsersClusters;
	}
	int newNumClusters = numberOfClusters;
	if (numberOfClusters == DEFAULT_CLUSTERS) {
		newNumClusters = userSentiments.size() / P;
	}
	realUsersClusters = new KMeansClustering(userSentiments, newNumClusters, Metrics::EUCLIDEAN);
	realUsersClusters->run();

	if (virtualUsersClusters != NULL) {
		delete virtualUsersClusters;
	}
	newNumClusters = numberOfClusters;
	if (numberOfClusters == DEFAULT_CLUSTERS) {
		newNumClusters = clusterSentiments.size() / P;
	}
	virtualUsersClusters = new KMeansClustering(clusterSentiments, newNumClusters, Metrics::EUCLIDEAN);
	virtualUsersClusters->run();
}



/* Combine user based and cluster based recommendations */
std::vector<unsigned int> ClusteringRecommender::recommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
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



/* Return the indices of the recommended coins for a user
 * based on total sentiment per user */
std::vector<unsigned int> ClusteringRecommender::userBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the users in the same cluster as this user
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors = realUsersClusters->getPointsInSameCluster(user);

	// Guess the sentiment of coins without sentiment based on the neighbors
	// and keep the best 5
	std::vector< std::pair<double, unsigned int> > newCoins;
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < neighbors.size(); k++) {
				z_sum += std::abs(Metrics::cosineSimilarity(user, *neighbors[k]));
				unsigned int neighborIndex = userToSentiment.at(neighbors[k]->getID());
				sum += Metrics::cosineSimilarity(user, *neighbors[k]) * (neighbors[k]->at(j) - usersAverageSentiment[neighborIndex]);
			}
			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			newCoins.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	// Sort in descending order using lambda
	std::sort(newCoins.begin(), newCoins.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
		return left.first > right.first;
	});

	// Get the top 5 coins based on sentiment
	std::vector<unsigned int> recommendedCoins;
	for (unsigned int j = 0; j < min(5, newCoins.size()); j++) {
		recommendedCoins.push_back(newCoins[j].second);
	}

	return recommendedCoins;
}



/* Return the indices of the recommended coins for a user
 * based on total sentiment per user */
std::vector<unsigned int> ClusteringRecommender::clusterBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the users in the same cluster as this user
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors = realUsersClusters->getPointsInSameCluster(user);

	// Guess the sentiment of coins without sentiment based on the neighbors
	// and keep the best 5
	std::vector< std::pair<double, unsigned int> > newCoins;
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < neighbors.size(); k++) {
				z_sum += std::abs(Metrics::cosineSimilarity(user, *neighbors[k]));
				unsigned int neighborIndex = userToSentiment.at(neighbors[k]->getID());
				sum += Metrics::cosineSimilarity(user, *neighbors[k]) * (neighbors[k]->at(j) - usersAverageSentiment[neighborIndex]);
			}
			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			newCoins.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	// Sort in descending order using lambda
	std::sort(newCoins.begin(), newCoins.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
		return left.first > right.first;
	});

	// Get the top 5 coins based on sentiment
	std::vector<unsigned int> recommendedCoins;
	for (unsigned int j = 0; j < min(5, newCoins.size()); j++) {
		recommendedCoins.push_back(newCoins[j].second);
	}

	return recommendedCoins;
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

		if ((unsigned int) num <= usersAverageSentiment.size() && num > 0) {
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
		if ((unsigned int) num <= clustersAverageSentiment.size() && num > 0) {
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
