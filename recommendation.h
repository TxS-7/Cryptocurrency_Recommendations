#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <vector>
#include <string>
#include <unordered_map>
#include <set>
#include "tweet.h"
#include "clustering.h"
#include "cosine_lsh_recommender.h"
#include "clustering_recommender.h"
#include "data_point.h"

class Recommendation {
private:
	static const char *PROCESSED_TWEETS_FILENAME;
	static const unsigned int NUMBER_OF_CLUSTERS = 10;

	// Total sentiments for each user
	std::vector<DataPoint> userSentiments;
	std::vector<double> usersAverageSentiment;

	// Total sentiments for each cluster
	std::vector<DataPoint> processedTweets;
	std::vector<DataPoint> clusterSentiments;
	std::vector<double> clustersAverageSentiment;
	KMeansClustering *kMeans;

	std::unordered_map<unsigned int, unsigned int> userToSentiment;
	std::unordered_map<unsigned int, std::set<unsigned int> > unknownCoins;

	CosineLSHRecommender *rec1;
	ClusteringRecommender *rec2;

	void createUserSentiments(const std::vector<Tweet>&);
	void createClusterSentiments(const std::vector<Tweet>&);
	bool readProcessedTweets(const char *, std::vector<DataPoint>&, std::set<std::string>&) const;
public:
	Recommendation(const std::vector<Tweet>&, unsigned int, int, int);

	std::vector<std::string> cosineLSHRecommendations(unsigned int, const std::vector< std::vector<std::string> >&) const;
	std::vector<std::string> clusteringRecommendations(unsigned int, const std::vector< std::vector<std::string> >&) const;

	std::vector<int> findBestClusters(const std::vector<int>& options) { return rec2->findBestClusters(options, userSentiments, clusterSentiments); }

	std::vector<double> validate();

	virtual ~Recommendation() {
		if (kMeans != NULL) {
			delete kMeans;
		}
		if (rec1 != NULL) {
			delete rec1;
		}
		if (rec2 != NULL) {
			delete rec2;
		}
	}
};

#endif // RECOMMENDATION_H
