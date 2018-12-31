#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <vector>
#include <string>
#include <set>
#include "tweet.h"
#include "clustering.h"
#include "data_point.h"

class Recommendation {
protected:
	std::vector<DataPoint> userSentiments;
	std::vector<double> usersAverageSentiment;

	std::vector<DataPoint> clusterSentiments;
	std::vector<double> clustersAverageSentiment;
private:
	static const char *PROCESSED_TWEETS_FILENAME;
	static const unsigned int NUMBER_OF_CLUSTERS = 250;
	KMeansClustering *kMeans;

	void createUserSentiments(const std::vector<Tweet>&);
	void createClusterSentiments(const std::vector<Tweet>&);
	bool readProcessedTweets(const char *, std::vector<DataPoint>&, std::set<std::string>&) const;

	virtual std::vector< std::vector<unsigned int> > userBasedRecommendations() const = 0;
	virtual std::vector< std::vector<unsigned int> > clusterBasedRecommendations() const = 0;
public:
	Recommendation(const std::vector<Tweet>&);

	std::vector< std::vector<unsigned int> > recommendations() const;

	virtual ~Recommendation() {
		if (kMeans != NULL) {
			delete kMeans;
		}
	}
};

#endif // RECOMMENDATION_H
