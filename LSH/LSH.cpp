#include <vector>
#include <set>
#include "LSH.h"
#include "../metrics.h"
#include "../data_point.h"
#include "cosine_hash_table.h"

LSH::LSH(int k, int dimensions, int L2, int n)
	: L(L2) {
	// Create L hash tables
	int i;
	for (i = 0; i < L; i++) {
		tables.push_back(new CosineHashTable(k, dimensions));
	}
}


void LSH::insert(DataPoint& p) {
	// Insert the data point in every table
	int i;
	for (i = 0; i < L; i++) {
		tables[i]->insert(p);
	}
}


/* Find all neighbors from the L hash tables and return the index of the closest */
int LSH::findAllNeighbors(const DataPoint& q, std::vector<DataPoint *>& results, std::vector<double>& distances) const {
	// Create a set of points that have been already found
	std::set<std::string> alreadyFound;

	int i;
	double minDist = -1.0;
	int minIndex = -1;
	unsigned int total = 0;

	// Find the neighbors of q in every hash table
	for (i = 0; i < L; i++) {
		std::vector<DataPoint *> neighbors;
		std::vector<double> tempDistances;
		int tempMinIndex = tables[i]->findNeighbors(q, neighbors, tempDistances);
		total += neighbors.size();

		// No neighbors found
		if (tempMinIndex < 0) {
			continue;
		}

		DataPoint *tempMin = neighbors[tempMinIndex];
		double tempDist = tempDistances[tempMinIndex];
		// Check if local minimum is overall minimum
		if (tempDist < minDist || minIndex < 0) {
			minDist = tempDist;

			// Since we got a new minimum, the data point is new
			alreadyFound.insert(tempMin->getID());

			results.push_back(tempMin);
			distances.push_back(tempDist);
			minIndex = results.size() - 1;
		}

		// Store the points that haven't already been found
		unsigned int j;
		for (j = 0; j < neighbors.size(); j++) {
			if (alreadyFound.insert(neighbors[j]->getID()).second != false) { // new
				results.push_back(neighbors[j]);
				distances.push_back(tempDistances[j]);
			}
		}
	}

	return minIndex;
}


/* Find the nearest neighbor from the L hash tables and return the index of the closest */
double LSH::findNearestNeighbor(const DataPoint& q, DataPoint& min) const {
	int i;
	double minDist = -1.0;

	// Find the nearest neighbor of q in every hash table and return the closest
	for (i = 0; i < L; i++) {
		DataPoint curr;
		double tempDist = tables[i]->findNearest(q, curr);

		// No neighbors found
		if (tempDist < 0.0) {
			continue;
		} else if (tempDist == 0.0) { // Minimum found
			min = curr;
			return tempDist;
		}

		// Check if local minimum is overall minimum
		if (tempDist < minDist || minDist < 0) {
			min = curr;
			minDist = tempDist;
		}
	}

	return minDist;
}


unsigned long long LSH::getSize() const {
	unsigned long long total = 0;
	total += sizeof(L);
	total += sizeof(tables);
	total += L * sizeof(HashTable *);
	int i;
	for (i = 0; i < L; i++) {
		total += tables[i]->getSize();
	}
	return total;
}


LSH::~LSH() {
	int i;
	for (i = 0; i < L; i++) {
		delete tables[i];
	}
}
