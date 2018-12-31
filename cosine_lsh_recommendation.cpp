#include <vector>
#include <algorithm> // std::sort, std::min
#include <utility> // std::pair, std::make_pair
#include <cmath> // std::abs
#include "cosine_lsh_recommendation.h"
#include "tweet.h"
#include "LSH/LSH.h"
#include "data_point.h"
#include "metrics.h"
#include "util.h"

CosineLSHRecommendation::CosineLSHRecommendation(const std::vector<Tweet>& tweets, unsigned int neighborsArg, int kLSH, int L)
   		: Recommendation(tweets), numberOfNeighbors(neighborsArg), userLSH(NULL), clusterLSH(NULL) {
	// Insert every user vector in the Cosine LSH for user based and cluster based recommendations
	userLSH = new LSH(kLSH, userSentiments[0].getDimensions(), L, userSentiments.size());
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		userLSH->insert(userSentiments[i]);
	}

	clusterLSH = new LSH(kLSH, clusterSentiments[0].getDimensions(), L, clusterSentiments.size());
	for (unsigned int i = 0; i < clusterSentiments.size(); i++) {
		clusterLSH->insert(clusterSentiments[i]);
	}
}



/* Return the indices of the recommended coins for every user
 * based on total sentiment per user */
std::vector< std::vector<unsigned int> > CosineLSHRecommendation::userBasedRecommendations() const {
	std::vector< std::vector<unsigned int> > results;

	// Get the P neighbors from the LSH for every user
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		std::vector<DataPoint *> neighbors;
		std::vector<double> distances;
		userLSH->findAllNeighbors(userSentiments[i], 0.0, neighbors, distances);

		// Sort the neighbors by distance and keep the top P neighbors excluding the user himself
		std::vector< std::pair<double, unsigned int> > distancesAndIndices;
		// Create the vector of distances and indices used to find the indices of the closest neighbors
		for (unsigned int j = 0; j < distances.size(); j++) {
			if (distances[j] > 0) {
				distancesAndIndices.push_back(std::make_pair(distances[j], j));
			}
		}

		std::sort(distancesAndIndices.begin(), distancesAndIndices.end());

		// Get the P nearest neighbors
		std::vector<unsigned int> closestIndices;
		std::vector<DataPoint *> closest;
		for (unsigned int j = 0; j < min(numberOfNeighbors, distancesAndIndices.size()); j++) {
			closestIndices.push_back(distancesAndIndices[j].second);
			closest.push_back(neighbors[closestIndices[j]]);
		}


		// Guess the sentiment of coins without sentiment based on the neighbors
		// and keep the best 5
		std::vector< std::pair<double, unsigned int> > newCoins;
		for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
			if (userSentiments[i].at(j) == Tweet::SENTIMENT_NOT_SET) {
				double predictedSentiment = usersAverageSentiment[i];

				// Calculate normalizing factor z
				double z_sum = 0.0;
				double sum = 0.0;
				for (unsigned int k = 0; k < closest.size(); k++) {
					z_sum += std::abs(Metrics::cosineSimilarity(userSentiments[i], *closest[k]));
					sum += Metrics::cosineSimilarity(userSentiments[i], *closest[k]) * (closest[k]->at(j) - usersAverageSentiment[closestIndices[k]]);
				}
				double z = 1 / z_sum;
				predictedSentiment += z * sum;
				newCoins.push_back(std::make_pair(predictedSentiment, j));
			}
		}

		// Sort in descending order using lambda
		std::sort(newCoins.begin(), newCoins.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
			return left.first > right.second;
		});

		// Get the top 5 coins based on sentiment
		std::vector<unsigned int> recommendedCoins;
		for (unsigned int j = 0; j < 5; j++) {
			recommendedCoins.push_back(newCoins[j].second);
		}
		results.push_back(recommendedCoins);
	}

	return results;
}



/* Return the indices of the recommended coins for every user
 * based on total sentiment per cluster */
std::vector< std::vector<unsigned int> > CosineLSHRecommendation::clusterBasedRecommendations() const {
	std::vector< std::vector<unsigned int> > results;

	// Get the P neighbors from the LSH for every user
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		std::vector<DataPoint *> neighbors;
		std::vector<double> distances;
		clusterLSH->findAllNeighbors(userSentiments[i], 0.0, neighbors, distances);

		// Sort the neighbors by distance and keep the top P neighbors
		std::vector< std::pair<double, unsigned int> > distancesAndIndices;
		// Create the vector of distances and indices used to find the indices of the closest neighbors
		for (unsigned int j = 0; j < distances.size(); j++) {
			distancesAndIndices.push_back(std::make_pair(distances[j], j));
		}

		std::sort(distancesAndIndices.begin(), distancesAndIndices.end());

		// Get the P nearest neighbors
		std::vector<unsigned int> closestIndices;
		std::vector<DataPoint *> closest;
		for (unsigned int j = 0; j < min(numberOfNeighbors, distancesAndIndices.size()); j++) {
			closestIndices.push_back(distancesAndIndices[j].second);
			closest.push_back(neighbors[closestIndices[j]]);
		}


		// Guess the sentiment of coins without sentiment based on the neighbors
		// and keep the best 5
		std::vector< std::pair<double, unsigned int> > newCoins;
		for (unsigned int j = 0; j < userSentiments[i].getDimensions(); j++) {
			if (userSentiments[i].at(j) == Tweet::SENTIMENT_NOT_SET) {
				double predictedSentiment = usersAverageSentiment[i];

				// Calculate normalizing factor z
				double z_sum = 0.0;
				double sum = 0.0;
				for (unsigned int k = 0; k < closest.size(); k++) {
					z_sum += std::abs(Metrics::cosineSimilarity(userSentiments[i], *closest[k]));
					sum += Metrics::cosineSimilarity(userSentiments[i], *closest[k]) * (closest[k]->at(j) - clustersAverageSentiment[closestIndices[k]]);
				}
				double z = 1 / z_sum;
				predictedSentiment += z * sum;
				newCoins.push_back(std::make_pair(predictedSentiment, j));
			}
		}

		// Sort in descending order using lambda
		std::sort(newCoins.begin(), newCoins.end(), [](const std::pair<double, unsigned int>& left, const std::pair<double, unsigned int>& right) {
			return left.first > right.second;
		});

		// Get the top 5 coins based on sentiment
		std::vector<unsigned int> recommendedCoins;
		for (unsigned int j = 0; j < 2; j++) {
			recommendedCoins.push_back(newCoins[j].second);
		}
		results.push_back(recommendedCoins);
	}

	return results;
}
