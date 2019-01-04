#ifndef DATA_POINT_H
#define DATA_POINT_H

#include <vector>
#include <string>

class DataPoint {
private:
	std::string id;
	std::vector<double> x;
	double norm; // Save the point's norm to reduce calculations


	double calculateNorm() const;
public:
	DataPoint() : id(""), norm(-1.0) {}
	DataPoint(const std::vector<double>&, std::string);
	// Copy constructor
	DataPoint(const DataPoint&);
	// Assignment operator
	DataPoint& operator=(const DataPoint&);
	// Access to a vector position
	double at(unsigned int index) const { return x[index]; }
	double& operator[] (unsigned int index) { return x[index]; }

	bool readDataPoint(const std::string&);
	std::vector<double> getVector() const { return x; }
	unsigned int getDimensions() const { return x.size(); }
	std::string getID() const { return id; }
	double getNorm() const { return norm; }

	void print() const;

	double dotProduct(const std::vector<double>&) const;
	double dotProduct(const DataPoint&) const;
	void add(const DataPoint&);
	void divide(double);
	double distance(const DataPoint&) const;
	bool equal(const DataPoint& p) const { return x == p.x; }
	int findNearest(const std::vector<DataPoint *>&, double&, double (*distFun)(const DataPoint&, const DataPoint&)) const;
};

#endif // DATA_POINT_H
