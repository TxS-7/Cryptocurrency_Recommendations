#ifndef CLUSTERING_RECOMMENDATION_H
#define CLUSTERING_RECOMMENDATION_H

#include <vector>
#include "recommendation.h"
#include "tweet.h"
#include "clustering.h"

class ClusteringRecommendation : public Recommendation {
private:
	KMeansClustering *realUsersClusters;
	KMeansClustering *virtualUsersClusters;

	std::vector< std::vector<unsigned int> > userBasedRecommendations() const;
	std::vector< std::vector<unsigned int> > clusterBasedRecommendations() const;
public:
	ClusteringRecommendation(const std::vector<Tweet>&, int);

	~ClusteringRecommendation() {
		if (realUsersClusters != NULL) {
			delete realUsersClusters;
		}
		if (virtualUsersClusters != NULL) {
			delete virtualUsersClusters;
		}
	}
};

#endif // CLUSTERING_RECOMMENDATION_H
