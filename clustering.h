#ifndef CLUSTERING_H
#define CLUSTERING_H

#include <vector>
#include "data_point.h"
#include "metrics.h"


class KMeansClustering {
private:
	static const unsigned int LOOP_LIMIT = 50;

	DIST_PTR distFun;

	std::vector<DataPoint *> points;
	std::vector<DataPoint *> centroids;
	int numberOfClusters;
	std::vector< std::vector<DataPoint *> > clusters;


	void initialize();
	void assign();
	unsigned int update();

	bool isCentroid(const DataPoint *) const;
	void resetClusters();

	double silhouetteOfPoint(const DataPoint *) const;
public:
	KMeansClustering(std::vector<DataPoint>&, int, int);

	int run();
	double silhouette(std::vector<double>&) const;

	void getNumberOfPointsPerCluster(std::vector<unsigned int>&) const;
	void getPointsPerCluster(std::vector< std::vector< std::string> >&) const;
	std::vector<DataPoint *> getCentroids() const { return centroids; }

	~KMeansClustering() {
		if (!constructorError) {
			for (unsigned int i = 0; i < centroids.size(); i++) {
				// Delete centroids that don't match any of the dataset points
				// (Noted using ID "dummy")
				if (centroids[i]->getID() == "dummy") {
					delete centroids[i];
				}
			}
		}
	}
};

#endif // CLUSTERING_H
