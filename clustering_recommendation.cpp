#include <vector>
#include <algorithm> // std::sort
#include <cmath> // abs
#include "clustering_recommendation.h"
#include "tweet.h"
#include "clustering.h"
#include "data_point.h"
#include "metrics.h"
#include "util.h"

ClusteringRecommendation::ClusteringRecommendation(const std::vector<Tweet>& tweets, int numberOfClusters)
   		: Recommendation(tweets), realUsersClusters(NULL), virtualUsersClusters(NULL) {
	// Create the clusters for user based and cluster based recommendations
	realUsersClusters = new KMeansClustering(userSentiments, numberOfClusters, Metrics::EUCLIDEAN);
	realUsersClusters->run();
	virtualUsersClusters = new KMeansClustering(clusterSentiments, numberOfClusters, Metrics::EUCLIDEAN);
	virtualUsersClusters->run();
}



/* Return the indices of the recommended coins for every user
 * based on total sentiment per user */
std::vector< std::vector<unsigned int> > ClusteringRecommendation::userBasedRecommendations() const {
	std::vector< std::vector<unsigned int> > results;

	// Get the users in the same cluster as this user
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		std::vector<DataPoint *> neighbors = realUsersClusters->getPointsInSameCluster(userSentiments[i]);

		// Guess the sentiment of coins without sentiment based on the neighbors
		// and keep the best 5
		std::vector< std::pair<double, unsigned int> > newCoins;
		for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
			if (userSentiments[i].at(j) == usersAverageSentiment[i]) {
				double predictedSentiment = usersAverageSentiment[i];

				// Calculate normalizing factor z
				double z_sum = 0.0;
				double sum = 0.0;
				for (unsigned int k = 0; k < neighbors.size(); k++) {
					z_sum += std::abs(Metrics::cosineSimilarity(userSentiments[i], *neighbors[k]));
					sum += Metrics::cosineSimilarity(userSentiments[i], *neighbors[k]) * (neighbors[k]->at(j) - usersAverageSentiment[k]);
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
		results.push_back(recommendedCoins);
	}

	return results;
}

/* Return the indices of the recommended coins for every user
 * based on total sentiment per cluster */
std::vector< std::vector<unsigned int> > ClusteringRecommendation::clusterBasedRecommendations() const {
	std::vector< std::vector<unsigned int> > results;

	//

	// Get the users in the same cluster as this user
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		std::vector<DataPoint *> neighbors = realUsersClusters->getPointsInSameCluster(userSentiments[i]);

		// Guess the sentiment of coins without sentiment based on the neighbors
		// and keep the best 5
		std::vector< std::pair<double, unsigned int> > newCoins;
		for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
			if (userSentiments[i].at(j) == usersAverageSentiment[i]) {
				double predictedSentiment = usersAverageSentiment[i];

				// Calculate normalizing factor z
				double z_sum = 0.0;
				double sum = 0.0;
				for (unsigned int k = 0; k < neighbors.size(); k++) {
					z_sum += std::abs(Metrics::cosineSimilarity(userSentiments[i], *neighbors[k]));
					sum += Metrics::cosineSimilarity(userSentiments[i], *neighbors[k]) * (neighbors[k]->at(j) - usersAverageSentiment[k]);
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
		results.push_back(recommendedCoins);
	}

	return results;
}
