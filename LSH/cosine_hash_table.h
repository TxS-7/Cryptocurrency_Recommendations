#ifndef COSINE_HASH_TABLE_H
#define COSINE_HASH_TABLE_H

#include <vector>
#include "hash_table.h"
#include "../data_point.h"

class CosineHashTable: public HashTable {
private:
	std::vector< std::vector<double> > r; // r_i, each one for a hyperplane


	unsigned int gToBucket(const std::vector<int>&) const;
	int h(const DataPoint&, int) const; // h_i
public:
	CosineHashTable(int, int);
};

#endif // COSINE_HASH_TABLE_H
