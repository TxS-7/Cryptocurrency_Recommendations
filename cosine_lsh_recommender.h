#ifndef COSINE_LSH_RECOMMENDER_H
#define COSINE_LSH_RECOMMENDER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "LSH/LSH.h"
#include "data_point.h"

class CosineLSHRecommender {
private:
	unsigned int numberOfNeighbors;
	LSH *userLSH;
	LSH *clusterLSH;
	int kLSH;
	int L;

	std::vector<double> usersAverageSentiment;
	std::vector<double> clustersAverageSentiment;

	std::unordered_map<std::string, unsigned int> userToSentiment;
	std::unordered_map<std::string, unsigned int> clusterToSentiment;

	std::vector<unsigned int> userBasedRecommendations(const DataPoint&, const std::set<unsigned int>&) const;
	std::vector<unsigned int> clusterBasedRecommendations(const DataPoint&, const std::set<unsigned int>&) const;
public:
	CosineLSHRecommender(unsigned int neighborsArg, int kLSHArg = 4, int LArg = 5)
		: numberOfNeighbors(neighborsArg), userLSH(NULL), clusterLSH(NULL), kLSH(kLSHArg), L(LArg) {}

	void train(std::vector<DataPoint>&, const std::vector<double>&, std::vector<DataPoint>&, const std::vector<double>&);
	std::vector<unsigned int> recommendations(const DataPoint&, const std::set<unsigned int>&) const;
	std::unordered_map<unsigned int, double> userBasedPredictions(const DataPoint&, const std::set<unsigned int>&) const;
	//std::unordered_map<unsigned int, double> clusterBasedPrediction(const DataPoint&, const std::set<unsigned int>&) const;

	~CosineLSHRecommender() {
		if (userLSH != NULL) {
			delete userLSH;
		}
		if (clusterLSH != NULL) {
			delete clusterLSH;
		}
	}
};

#endif // COSINE_LSH_RECOMMENDER_H
