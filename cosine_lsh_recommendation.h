#ifndef COSINE_LSH_RECOMMENDATION_H
#define COSINE_LSH_RECOMMENDATION_H

#include <vector>
#include "recommendation.h"
#include "tweet.h"
#include "LSH/LSH.h"

class CosineLSHRecommendation : public Recommendation {
private:
	unsigned int numberOfNeighbors;

	LSH *lsh;
public:
	CosineLSHRecommendation(const std::vector<Tweet>&, unsigned int, int kLSH = 4, int L = 5);

	std::vector< std::vector<unsigned int> > recommendations() const;

	~CosineLSHRecommendation() {
		if (lsh != NULL) {
			delete lsh;
		}
	}
};

#endif // COSINE_LSH_RECOMMENDATION_H
