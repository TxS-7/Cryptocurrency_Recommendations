#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib> // atof
#include <cmath>
#include "data_point.h"
#include "metrics.h"
#include "util.h"

DataPoint::DataPoint(const std::vector<double>& p, std::string id2) : id(id2) {
	unsigned int i;
	for (i = 0; i < p.size(); i++) {
		x.push_back(p[i]);
	}
	norm = calculateNorm();
}


DataPoint::DataPoint(const DataPoint& p2) {
	id = p2.id;
	unsigned int i;
	for (i = 0; i < p2.x.size(); i++) {
		x.push_back(p2.x[i]);
	}
	norm = p2.norm;
}


DataPoint& DataPoint::operator=(const DataPoint& p2) {
	if (this == &p2) {
		return *this;
	}

	id = p2.id;
	// Delete previous vector if it exists
	if (x.size() > 0) {
		x.clear();
	}
	unsigned int i;
	for (i = 0; i < p2.x.size(); i++) {
		x.push_back(p2.x[i]);
	}
	norm = p2.norm;
	return *this;
}


double DataPoint::calculateNorm() const {
	unsigned int i;
	double total = 0.0;
	for (i = 0; i < x.size(); i++) {
		total += (((double) x[i]) * x[i]);
	}
	return sqrt(total);
}


/* Read tab-seperated coordinates for the input point */
bool DataPoint::readDataPoint(const std::string& line) {
	// Check if '\r' is found which is used in Windows OS
	if (line.find('\r') != std::string::npos) {
		std::cerr << "[-] Windows format found" << std::endl;
		return false;
	}
	// Check which delimiter is used
	bool tab_used = false;
	bool comma_used = false;
	if (line.find('\t') != std::string::npos) {
		tab_used = true;
	}
	if (line.find(',') != std::string::npos) {
		comma_used = true;
	}

	char delim;
	if ((tab_used && comma_used) || (!tab_used && !comma_used)) {
		return false;
	} else if (tab_used) { // TAB
		delim = '\t';
	} else { // COMMA
		delim = ',';
	}

	// Convert to string stream
	std::istringstream lineStream(line);

	// Remove leading whitespaces
	lineStream >> std::ws;

	// Read the ID first as a string
	if (!getline(lineStream, id, delim)) {
		return false;
	}

	std::string curr;
	while (getline(lineStream, curr, delim)) {
		if (!isNumber(curr)) {
			return false;
		}
		x.push_back(atof(curr.c_str()));
	}

	// Save point's norm
	norm = calculateNorm();
	return true;
}


void DataPoint::print() const {
	std::cout.precision(15);
	std::cout << std::fixed;
	std::cout << id << ": ";
	for (std::vector<double>::const_iterator it = x.begin(); it != x.end(); it++) {
		if (it == x.begin()) {
			std::cout << *it;
		} else {
			std::cout << ", " << *it;
		}
	}
	std::cout << std::endl;
}


/* Compute dot product of the data point with a given vector */
double DataPoint::dotProduct(const std::vector<double>& y) const {
	if (x.size() != y.size()) {
		return 0.0;
	}

	double total = 0;
	unsigned int i;
	for (i = 0; i < x.size(); i++) {
		total += x[i] * y[i];
	}

	return total;
}


/* Compute dot product of the data point with another data point */
double DataPoint::dotProduct(const DataPoint& p2) const {
	if (this->getDimensions() != p2.getDimensions()) {
		return 0.0;
	}

	double total = 0.0;
	unsigned int i;
	for (i = 0; i < x.size(); i++) {
		total += ((double) x[i]) * p2.x[i];
	}

	return total;
}


void DataPoint::add(const DataPoint& p) {
	if (this->getDimensions() != p.getDimensions()) {
		return;
	}

	unsigned int i;
	for (i = 0; i < x.size(); i++) {
		x[i] += p.x[i];
	}
	norm = calculateNorm();
}


void DataPoint::divide(double y) {
	if (y == 0) {
		return;
	}

	unsigned int i;
	for (i = 0; i < x.size(); i++) {
		x[i] /= y;
	}
	norm = calculateNorm();
}


/* Compute euclidean distance from a given point */
double DataPoint::distance(const DataPoint& p) const {
	if (this->getDimensions() != p.getDimensions()) {
		return 0.0;
	}

	double total = 0.0;
	unsigned int i;
	for (i = 0; i < x.size(); i++) {
		total += (((double) (x[i] - p.x[i])) * (x[i] - p.x[i]));
	}

	return sqrt(total);
}


/* Find the nearest point from a vector of points using brute force search */
int DataPoint::findNearest(const std::vector<DataPoint *>& searchPoints, double& minDist, DIST_PTR distFun) const {
	// No search points given
	if (searchPoints.size() == 0) {
		minDist = -1;
		return -1;
	}

	// Mismatching dimensions
	if (searchPoints[0]->getDimensions() != this->getDimensions()) {
		minDist = -1;
		return -1;
	}

	int minIndex = 0;
	minDist = distFun(*this, *searchPoints[0]);


	unsigned int i;
	for (i = 1; i < searchPoints.size(); i++) {
		// Check if dimensions match
		if (searchPoints[i]->getDimensions() != this->getDimensions()) {
			minDist = -1;
			return -1;
		}

		double dist = distFun(*this, *searchPoints[i]);
		if (dist < minDist) {
			minDist = dist;
			minIndex = i;
		}
	}

	return minIndex;
}
