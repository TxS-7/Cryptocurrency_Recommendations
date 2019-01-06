#include <vector>
#include <string>
#include <set>
#include <algorithm> // std::sort
#include <utility> // std::pair, std::make_pair
#include <cmath> // std::abs
#include "cosine_lsh_recommender.h"
#include "LSH/LSH.h"
#include "data_point.h"
#include "metrics.h"
#include "util.h"

void CosineLSHRecommender::train(std::vector<DataPoint>& userSentiments, const std::vector<double>& usersAvg, std::vector<DataPoint>& clusterSentiments, const std::vector<double>& clustersAvg) {
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


	// Insert every user vector in the Cosine LSH for user based and cluster based recommendations
	if (userLSH != NULL) {
		delete userLSH;
	}
	userLSH = new LSH(kLSH, userSentiments[0].getDimensions(), L, userSentiments.size());
	for (unsigned int i = 0; i < userSentiments.size(); i++) {
		userLSH->insert(userSentiments[i]);
	}

	if (clusterLSH != NULL) {
		delete clusterLSH;
	}
	clusterLSH = new LSH(kLSH, clusterSentiments[0].getDimensions(), L, clusterSentiments.size());
	for (unsigned int i = 0; i < clusterSentiments.size(); i++) {
		clusterLSH->insert(clusterSentiments[i]);
	}
}



/* Combine user based and cluster based recommendations */
std::vector<unsigned int> CosineLSHRecommender::recommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
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
std::vector<unsigned int> CosineLSHRecommender::userBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
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



/* Return the indices of the recommended coins for a user
 * based on total sentiment per cluster */
std::vector<unsigned int> CosineLSHRecommender::clusterBasedRecommendations(const DataPoint& user, const std::set<unsigned int>& unknown) const {
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
std::vector< std::pair<double, unsigned int> > CosineLSHRecommender::userBasedPredictions(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the P neighbors from the LSH
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors;
	std::vector<double> distances;
	userLSH->findAllNeighbors(user, neighbors, distances);

	std::vector< std::pair<double, unsigned int> > predictions;
	if (neighbors.size() == 0) {
		return predictions;
	}


	// Sort the neighbors by distance and keep the top P neighbors excluding the user himself
	std::vector< std::pair<double, unsigned int> > distancesAndIndices;
	// Create the vector of distances and indices used to find the indices of the closest neighbors
	for (unsigned int j = 0; j < distances.size(); j++) {
		// Exclude the same user
		if (neighbors[j]->getID() != user.getID()) {
			distancesAndIndices.push_back(std::make_pair(distances[j], j));
		}
	}

	std::sort(distancesAndIndices.begin(), distancesAndIndices.end());

	// Get the P nearest neighbors
	std::vector<DataPoint *> closest;
	for (unsigned int j = 0; j < min(numberOfNeighbors, distancesAndIndices.size()); j++) {
		closest.push_back(neighbors[distancesAndIndices[j].second]);
	}


	// Guess the sentiment of coins without sentiment based on the neighbors
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z and ratings
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < closest.size(); k++) {
				z_sum += std::abs(Metrics::cosineSimilarity(user, *closest[k]));
				unsigned int neighborIndex = userToSentiment.at(closest[k]->getID());
				sum += Metrics::cosineSimilarity(user, *closest[k]) * (closest[k]->at(j) - usersAverageSentiment[neighborIndex]);
			}
			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			predictions.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	return predictions;
}



/* Return the predicted score for the given unknown coin ratings */
std::vector< std::pair<double, unsigned int> > CosineLSHRecommender::clusterBasedPredictions(const DataPoint& user, const std::set<unsigned int>& unknown) const {
	// Get the P neighbors from the LSH
	unsigned int userIndex = userToSentiment.at(user.getID());
	std::vector<DataPoint *> neighbors;
	std::vector<double> distances;
	clusterLSH->findAllNeighbors(user, neighbors, distances);

	std::vector< std::pair<double, unsigned int> > predictions;
	if (neighbors.size() == 0) {
		return predictions;
	}

	// Sort the neighbors by distance and keep the top P neighbors
	std::vector< std::pair<double, unsigned int> > distancesAndIndices;
	// Create the vector of distances and indices used to find the indices of the closest neighbors
	for (unsigned int j = 0; j < distances.size(); j++) {
		distancesAndIndices.push_back(std::make_pair(distances[j], j));
	}

	std::sort(distancesAndIndices.begin(), distancesAndIndices.end());

	// Get the P nearest neighbors
	std::vector<DataPoint *> closest;
	for (unsigned int j = 0; j < min(numberOfNeighbors, distancesAndIndices.size()); j++) {
		closest.push_back(neighbors[distancesAndIndices[j].second]);
	}


	// Guess the sentiment of coins without sentiment based on the neighbors
	for (unsigned int j = 0; j < user.getDimensions(); j++) {
		if (unknown.find(j) != unknown.end()) {
			double predictedSentiment = usersAverageSentiment[userIndex];

			// Calculate normalizing factor z and ratings
			double z_sum = 0.0;
			double sum = 0.0;
			for (unsigned int k = 0; k < closest.size(); k++) {
				z_sum += std::abs(Metrics::cosineSimilarity(user, *closest[k]));
				unsigned int neighborIndex = clusterToSentiment.at(closest[k]->getID());
				sum += Metrics::cosineSimilarity(user, *closest[k]) * (closest[k]->at(j) - clustersAverageSentiment[neighborIndex]);
			}
			double z = 1 / z_sum;
			predictedSentiment += z * sum;
			predictions.push_back(std::make_pair(predictedSentiment, j));
		}
	}

	return predictions;
}
