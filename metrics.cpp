#include "metrics.h"
#include "data_point.h"

double Metrics::euclideanDistance(const DataPoint& p1, const DataPoint& p2) {
	return p1.distance(p2);
}


double Metrics::cosineSimilarity(const DataPoint& p1, const DataPoint& p2) {
	double sim = p1.dotProduct(p2) / (p1.getNorm() * p2.getNorm());
	return 1 - sim;
}