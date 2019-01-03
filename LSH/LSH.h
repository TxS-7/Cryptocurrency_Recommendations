#ifndef LSH_H
#define LSH_H

#include <vector>
#include "hash_table.h"
#include "../data_point.h"

class LSH {
protected:
	int L; // Number of Hash Tables

	// L hash tables;
	std::vector<HashTable *> tables;
public:
	LSH(int, int, int, int);

	void insert(DataPoint&);
	int findAllNeighbors(const DataPoint&, std::vector<DataPoint *>&, std::vector<double>&) const;
	double findNearestNeighbor(const DataPoint&, DataPoint&) const;

	unsigned long long getSize() const;

	~LSH();
};

#endif // LSH_H
