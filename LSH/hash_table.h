#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <vector>
#include <string>
#include <unordered_map>
#include "../data_point.h"
#include "../metrics.h"

class HashTable {
protected:
	int k; // Number of hash functions h_i
	int dimensions; // Dimensions of data points to be stored

	std::unordered_map<int, std::vector<DataPoint *> > buckets;

	// Store g function for every data point using the point's ID
	// (Match data point to g function)
	std::unordered_map<std::string, std::vector<int> > saved_g;

	// Pointer to dist function to be used
	DIST_PTR dist;


	virtual unsigned int gToBucket(const std::vector<int>&) const = 0; // Convert g to an index for a bucket
	virtual int h(const DataPoint&, int) const = 0; // h_i
public:
	HashTable(int k2, int d, DIST_PTR metric) : k(k2), dimensions(d), dist(metric) {}

	void insert(DataPoint&);
	int findNeighbors(const DataPoint&, double, std::vector<DataPoint *>&, std::vector<double>&) const;
	double findNearest(const DataPoint&, DataPoint&) const;

	virtual unsigned long long getSize() const;

	virtual ~HashTable() {}
};

#endif // HASH_TABLE_H
