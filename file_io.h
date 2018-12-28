#ifndef FILE_IO_H
#define FILE_IO_H

#include <vector>
#include <string>
#include <unordered_map>
#include "tweet.h"

#define IO_GENERAL_ERROR   0
#define IO_NOT_UNIQUE     -1
#define IO_NOT_INCREASING -2

int readInputFile(const char *, std::vector<Tweet>&, unsigned int&);
bool readSentimentLexicon(const char *, std::unordered_map<std::string, double>&);
bool readCoins(const char *, std::vector< std::vector<std::string> >&);
//bool writeOutputFile();

#endif // FILE_IO_H
