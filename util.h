#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>

bool isNumber(const std::string&);
bool fileAccessible(const char *);
bool emptyFile(const char *);
unsigned int mod(long long, unsigned int);
unsigned int binToDec(const std::vector<int>&);
std::string toLower(const std::string&);
unsigned int min(unsigned int, unsigned int);

#endif // UTIL_H
