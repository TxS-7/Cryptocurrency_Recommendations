#ifndef METRICS_H
#define METRICS_H

#include "data_point.h"

typedef double (*DIST_PTR)(const DataPoint&, const DataPoint&);

class Metrics {
public:
	static const int EUCLIDEAN = 1;
	static const int COSINE = 2;

	static double euclideanDistance(const DataPoint&, const DataPoint&);
	static double cosineDistance(const DataPoint&, const DataPoint&);
	static double cosineSimilarity(const DataPoint&, const DataPoint&);
};

#endif // METRICS_H
