#ifndef COSINE_LSH_RECOMMENDATION_H
#define COSINE_LSH_RECOMMENDATION_H

#include <vector>
#include "recommendation.h"
#include "tweet.h"
#include "LSH/LSH.h"

class CosineLSHRecommendation : public Recommendation {
private:
	unsigned int numberOfNeighbors;
	LSH *userLSH;
	LSH *clusterLSH;

	std::vector< std::vector<unsigned int> > userBasedRecommendations() const;
	std::vector< std::vector<unsigned int> > clusterBasedRecommendations() const;
public:
	CosineLSHRecommendation(const std::vector<Tweet>&, unsigned int, int kLSH = 4, int L = 5);

	~CosineLSHRecommendation() {
		if (userLSH != NULL) {
			delete userLSH;
		}
		if (clusterLSH != NULL) {
			delete clusterLSH;
		}
	}
};

#endif // COSINE_LSH_RECOMMENDATION_H
