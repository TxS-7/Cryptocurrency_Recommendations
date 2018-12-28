#include <vector>
#include "hash_table.h"
#include "../data_point.h"

/* Insert a data point to the hash table */
void HashTable::insert(DataPoint& p) {
	if (p.getDimensions() != (unsigned int) dimensions) {
		return;
	}

	// Create g function consisting of every h_i
	std::vector<int> g;
	int i;
	for (i = 0; i < k; i++) {
		g.push_back(h(p, i));
	}

	int index = gToBucket(g);

	// Insert the point in the hash table
	buckets[index].push_back(&p);

	// Save the g function for this point
	saved_g[p.getID()] = g;
}


/* Find the neighbors of the given point and return them along with the closest neigbor index */
int HashTable::findNeighbors(const DataPoint& q, double radius, std::vector<DataPoint *>& result, std::vector<double>& distances) const {
	// Find the bucket of the query point
	std::vector<int> g;
	int j;
	for (j = 0; j < k; j++) {
		g.push_back(h(q, j));
	}

	int index = gToBucket(g);

	// Key doesn't exist (no neighbors)
	if (buckets.find(index) == buckets.end()) {
		return -1;
	}

	std::vector<DataPoint *> neighbors = buckets.at(index);
	double minDist = -1.0;
	int minIndex = -1;

	// Find every other point in the same bucket and keep the points within the given radius
	unsigned int i;
	for (i = 0; i < neighbors.size(); i++) {
		// Check if the points have the same g function
		if (g != saved_g.at(neighbors[i]->getID())) {
			continue;
		}

		double tempDist = dist(q, *(neighbors.at(i)));
		if (tempDist < radius) {
			result.push_back(neighbors.at(i));
			distances.push_back(tempDist);

			if (tempDist < minDist || minIndex < 0) {
				minDist = tempDist;
				minIndex = result.size() - 1;
			}
		}
	}

	return minIndex;
}


/* Find the nearest neighbor of the given point and return the distance */
double HashTable::findNearest(const DataPoint& q, DataPoint& min) const {
	// Find the bucket of the query point
	std::vector<int> g;
	int j;
	for (j = 0; j < k; j++) {
		g.push_back(h(q, j));
	}

	int index = gToBucket(g);

	// Key doesn't exist (no neighbors)
	if (buckets.find(index) == buckets.end()) {
		return -1;
	}

	std::vector<DataPoint *> neighbors = buckets.at(index);
	double minDist = -1.0;

	// Find every other point in the same bucket and find the nearest
	unsigned int i;
	for (i = 0; i < neighbors.size(); i++) {
		// Check if the points have the same g function
		if (g != saved_g.at(neighbors[i]->getID())) {
			continue;
		}

		double tempDist = dist(q, *(neighbors.at(i)));
		if (tempDist == 0.0) { // Minimum found
			min = *(neighbors.at(i));
			return tempDist;
		}
		if (tempDist < minDist || minDist < 0.0) {
			min = *(neighbors.at(i));
			minDist = tempDist;
		}
	}

	return minDist;
}


unsigned long long HashTable::getSize() const {
	unsigned long long total = 0;
	total += sizeof(k);
	total += sizeof(dimensions);


	total += sizeof(buckets);
	unsigned int bucketCount = buckets.bucket_count();
	// Size of one key and one vector for each bucket
	total += bucketCount * (sizeof(int) + sizeof(std::vector<DataPoint *>));

	// Get vector size in each bucket
	for (auto it : buckets) {
		if (it.second.size() > 0) {
			total += it.second.size() * sizeof(it.second[0]);
		}
	}


	total += sizeof(saved_g);
	bucketCount = saved_g.bucket_count();
	total += bucketCount * sizeof(std::vector<int>);

	// Get vector and key size for each bucket
	for (auto it : saved_g) {
		total += sizeof(it.first);
		total += it.first.size();
		if (it.second.size() > 0) {
			total += it.second.size() * sizeof(it.second[0]);
		}
	}

	total += sizeof(dist);

	return total;
}
