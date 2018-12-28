#include <vector>
#include <random>
#include <chrono>
#include <cmath>
#include "cosine_hash_table.h"
#include "../data_point.h"
#include "../metrics.h"
#include "../util.h"

CosineHashTable::CosineHashTable(int k, int dimensions)
	: HashTable(k, dimensions, &Metrics::cosineSimilarity) {
	// Initialize the seed for the random number generator
	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed);

	std::normal_distribution<double> distrib(0.0, 1.0); // N(0,1)

	// Create k random r_i vectors using normal distribution
	int i;
	for (i = 0; i < k; i++) {
		std::vector<double> new_r;
		int j;
		for (j = 0; j < dimensions; j++) {
			new_r.push_back(distrib(generator));
		}
		r.push_back(new_r);
	}
}


int CosineHashTable::h(const DataPoint& p, int i) const {
	double prod = p.dotProduct(r[i]);
	return (prod >= 0) ? 1 : 0;
}


/* Convert the binary sequence g to an integer used for the hash table indexes */
unsigned int CosineHashTable::gToBucket(const std::vector<int>& g) const {
	return binToDec(g);
}
