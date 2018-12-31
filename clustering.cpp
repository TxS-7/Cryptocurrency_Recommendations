#include <iostream>
#include <vector>
#include <algorithm> // random_shuffle
#include <random>
#include <chrono>
#include "clustering.h"
#include "data_point.h"
#include "metrics.h"


KMeansClustering::KMeansClustering(std::vector<DataPoint>& inputPoints, int numClusters, int metric)
		: numberOfClusters(numClusters), clusters(numClusters) {
	// Invalid arguments
	if (numClusters <= 1 || (unsigned int) numClusters >= inputPoints.size()) {
		std::cerr << "Invalid constructor arguments" << std::endl;
		exit(-1);
	}

	if (metric == Metrics::EUCLIDEAN) {
		distFun = &Metrics::euclideanDistance;
	} else {
		distFun = &Metrics::cosineSimilarity;
	}

	for (unsigned int i = 0; i < inputPoints.size(); i++) {
		if (inputPoints[i].getID() == "dummy") {
			std::cerr << "Point ID: dummy is not allowed" << std::endl;
			exit(-1);
		}
		points.push_back(&inputPoints[i]);
	}
}



/* Pick the centroids at random */
void KMeansClustering::initialize() {
	// Shuffle the points and choose the k first points as the centroids
	std::vector<int> indices;
	for (unsigned int i = 0; i < points.size(); i++) {
		indices.push_back(i);
	}

	srand(time(NULL));
	std::random_shuffle(indices.begin(), indices.end());
	for (int i = 0; i < numberOfClusters; i++) {
		centroids.push_back(points[indices[i]]);
		points[indices[i]]->setCluster(i);
	}
}



/* Assign each point to the closest centroid */
void KMeansClustering::assign() {
	// For every point, find closest centroid
	for (unsigned int i = 0; i < points.size(); i++) {
		if (points[i]->getCluster() < 0) { // Unassigned point
			// Find nearest centroid to point
			double minDist = -1.0;
			int minIndex = points[i]->findNearest(centroids, minDist, distFun);
			points[i]->setCluster(minIndex);
			clusters[minIndex].push_back(points[i]);
		}
	}
}



/* Set the new centroid of each cluster as the mean of its points */
unsigned int KMeansClustering::update() {
	// vector of zeros used as initializer
	std::vector<double> zero(points[0]->getDimensions());
	std::fill(zero.begin(), zero.end(), 0.0);

	// Initialize the vector for every cluster to [0, 0, ..., 0]
	std::vector<DataPoint *> newCentroids(numberOfClusters);
	for (unsigned int i = 0; i < newCentroids.size(); i++) {
		newCentroids[i] = new DataPoint(zero, "dummy");
	}


	// Create the sum for every cluster
	for (unsigned int i = 0; i < clusters.size(); i++) {
		for (unsigned int j = 0; j < clusters[i].size(); j++) {
			newCentroids[i]->add(*(clusters[i][j]));
		}
	}

	// Create the mean point by dividing by number of points for each cluster
	for (unsigned int i = 0; i < clusters.size(); i++) {
		if (clusters[i].size() > 0) {
			newCentroids[i]->divide(clusters[i].size());
		}
	}


	// Create the new centroids if they are different
	unsigned int changesMade = 0;
	for (unsigned int i = 0; i < clusters.size(); i++) {
		if (clusters[i].size() == 0) { // Empty cluster
			delete newCentroids[i];
			continue;
		}

		if (!newCentroids[i]->equal(*centroids[i])) { // Centroid changed
			if (centroids[i]->getID() == "dummy") { // The previous centroid was a new point
				delete centroids[i];
			} else { // The previous centroid is part of the dataset
				// Place the old centroid back in the cluster
				clusters[i].push_back(centroids[i]);
			}
			centroids[i] = newCentroids[i];
			centroids[i]->setCluster(i);
			changesMade++;
		} else {
			delete newCentroids[i]; // Not needed any more
		}
	}

	return changesMade;
}



int KMeansClustering::run() {
	int numberOfLoops = 0;
	initialize();
	unsigned int changes = 0;
	do {
		resetClusters();

		assign();
		changes = update();
		numberOfLoops++;
	} while (changes > 0 && (unsigned int) numberOfLoops < LOOP_LIMIT);

	return numberOfLoops;
}


bool KMeansClustering::isCentroid(const DataPoint *x) const {
	for (unsigned int i = 0; i < centroids.size(); i++) {
		if (x->getID() == centroids[i]->getID()) {
			return true;
		}
	}

	return false;
}


/* Set current cluster for every non-centroid point to -1
 * (Called before assignment step to reassign the points to new clusters) */
void KMeansClustering::resetClusters() {
	for (unsigned int i = 0; i < points.size(); i++) {
		if (!isCentroid(points[i])) {
			points[i]->setCluster(-1);
		}
	}

	for (unsigned int i = 0; i < clusters.size(); i++) {
		clusters[i].clear();
	}
}



double KMeansClustering::silhouette(std::vector<double>& clusterSilhouette) const {
	clusterSilhouette.clear();
	for (unsigned int i = 0; i < clusters.size(); i++) {
		clusterSilhouette.push_back(0.0);
	}
	double totalSilhouette = 0.0;


	for (unsigned int i = 0; i < clusters.size(); i++) {
		for (unsigned int j = 0; j < clusters[i].size(); j++) {
			double result = silhouetteOfPoint(clusters[i][j]);
			clusterSilhouette[i] += result;
			totalSilhouette += result;
		}
	}


	// Calculate average silhouette for every cluster
	for (unsigned int i = 0; i < clusters.size(); i++) {
		if (clusters[i].size() > 0) {
			clusterSilhouette[i] /= (clusters[i].size());
		}
	}

	// Return average silhouette of all points
	if (points.size() > 0) {
		return totalSilhouette / points.size();
	} else {
		return 0;
	}
}


double KMeansClustering::silhouetteOfPoint(const DataPoint *p) const {
	// Calculate average distance of p to other points in same cluster
	int clusterIndex = p->getCluster();
	double sum = 0.0;
	unsigned int pointsInCluster = clusters[clusterIndex].size();
	for (unsigned int i = 0; i < clusters[clusterIndex].size(); i++) {
		sum += distFun(*p, *(clusters[clusterIndex][i]));
	}
	double a = 0.0;
	if (pointsInCluster > 0) {
		a = sum / pointsInCluster;
	}


	// Find second closest cluster
	std::vector<DataPoint *> otherCentroids;
	std::vector<int> indices;
	for (unsigned int i = 0; i < centroids.size(); i++) {
		if (i == (unsigned int) clusterIndex) {
			continue;
		}

		otherCentroids.push_back(centroids[i]);
		indices.push_back(i);
	}

	double minDist = 0.0;
	int minIndex = p->findNearest(otherCentroids, minDist, distFun);
	int minCluster = indices[minIndex];

	// Calculate average distance of p to other points in second closest cluster
	sum = 0.0;
	pointsInCluster = clusters[minCluster].size();
	for (unsigned int i = 0; i < clusters[minCluster].size(); i++) {
		sum += distFun(*p, *(clusters[minCluster][i]));
	}
	double b = 0.0;
	if (pointsInCluster > 0) {
		b = sum / pointsInCluster;
	}

	double max = ((a > b) ? a : b);
	if (max == 0) {
		return 0.0;
	}
	return (b - a) / max;
}


void KMeansClustering::getNumberOfPointsPerCluster(std::vector<unsigned int>& result) const {
	result.clear();
	for (unsigned int i = 0; i < clusters.size(); i++) {
		result.push_back(clusters[i].size());
	}
}


/* Get the IDs of the points of each cluster */
void KMeansClustering::getPointsPerCluster(std::vector< std::vector< std::string> >& results) const {
	results.clear();

	for (unsigned int i = 0; i < clusters.size(); i++) {
		results.push_back(std::vector<std::string>());
		for (unsigned int j = 0; j < clusters[i].size(); j++) {
			results[i].push_back(clusters[i][j]->getID());
		}
	}
}
