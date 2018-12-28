#ifndef FILE_IO_H
#define FILE_IO_H

#include <vector>
#include <string>
#include "tweet.h"

int readInputFile(const char *, std::vector<Tweet>&, unsigned int&);
//bool writeOutputFile();

#endif // FILE_IO_H
