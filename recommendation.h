#ifndef RECOMMENDATION_H
#define RECOMMENDATION_H

#include <vector>
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
	KMeansClustering *kMeans;

	void createUserSentiments(const std::vector<Tweet>&);
	void createClusterSentiments(const std::vector<Tweet>&);
	bool readProcessedTweets(const char *, std::vector<DataPoint>&) const;
public:
	Recommendation(const std::vector<Tweet>&);

	virtual std::vector< std::vector<unsigned int> > recommendations() const = 0;
};

#endif // RECOMMENDATION_H
