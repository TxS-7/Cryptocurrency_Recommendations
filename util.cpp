#include <iostream>
#include <fstream>
#include <limits> // numeric_limits<streamsize>::max()
#include <cctype> // tolower
#include "util.h"

bool isNumber(const std::string& str) {
	if (str.empty()) {
		return false;
	}

	// Check first character for minus sign or a digit
	if (str[0] != '-' && !isdigit(str[0])) {
		return false;
	}

	bool floatFound = false;

	// Start checking from second character (first character checked above)
	std::string::const_iterator it = str.begin() + 1;
	while (it != str.end()) {
		if (*it == '.') {
			// Floating point already found
			if (floatFound) {
				return false;
			}
			floatFound = true;
		} else if (!isdigit(*it)) {
			return false;
		}
		it++;
	}

	return true;
}


bool fileAccessible(const char *filename) {
	std::ifstream f(filename);
	return f.good();
}


/* Clear the contents of a file */
bool emptyFile(const char *filename) {
	std::ofstream file;
	file.open(filename, std::fstream::out | std::fstream::trunc);
	if (!file) { // Failed to truncate file
		return false;
	}
	file.close();
	return true;
}


/* Modulo operator returning non-negative numbers */
unsigned int mod(long long x, unsigned int n) {
	if (x == 0) {
		return 0;
	} else if (x > 0) { // Same as remainder
		return x % n;
	} else {
		return (x % n + n) % n;
	}
}


unsigned int binToDec(const std::vector<int>& bits) {
	unsigned int total = 0;
	unsigned int i;
	for (i = 0; i < bits.size(); i++) {
		if (bits.at(i) == 1) {
			total += (1 << i);
		}
	}
	return total;
}


std::string toLower(const std::string& str) {
	std::string lower;
	for (unsigned int i = 0; i < str.size(); i++) {
		lower += tolower(str[i]);
	}
	return lower;
}
