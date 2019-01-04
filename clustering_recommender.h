#ifndef CLUSTERING_RECOMMENDER_H
#define CLUSTERING_RECOMMENDER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "clustering.h"
#include "data_point.h"

class ClusteringRecommender {
private:
	int numberOfClusters;
	unsigned int P;

	KMeansClustering *realUsersClusters;
	KMeansClustering *virtualUsersClusters;

	std::vector<double> usersAverageSentiment;
	std::vector<double> clustersAverageSentiment;

	std::unordered_map<std::string, unsigned int> userToSentiment;
	std::unordered_map<std::string, unsigned int> clusterToSentiment;

	std::vector<unsigned int> userBasedRecommendations(const DataPoint&, const std::set<unsigned int>&) const;
	std::vector<unsigned int> clusterBasedRecommendations(const DataPoint&, const std::set<unsigned int>&) const;
public:
	static const int DEFAULT_CLUSTERS;

	ClusteringRecommender(int numClusters, unsigned int PArg)
		: numberOfClusters(numClusters), P(PArg), realUsersClusters(NULL), virtualUsersClusters(NULL) {}

	std::vector<unsigned int> recommendations(const DataPoint&, const std::set<unsigned int>&) const;
	void train(std::vector<DataPoint>&, const std::vector<double>&, std::vector<DataPoint>&, const std::vector<double>&);
	std::vector<int> findBestClusters(const std::vector<int>&, std::vector<DataPoint>&, std::vector<DataPoint>&) const;

	~ClusteringRecommender() {
		if (realUsersClusters != NULL) {
			delete realUsersClusters;
		}
		if (virtualUsersClusters != NULL) {
			delete virtualUsersClusters;
		}
	}
};

#endif // CLUSTERING_RECOMMENDER_H