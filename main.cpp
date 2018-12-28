#include <iostream>
#include <vector>
#include <cstring> // strcmp, strncpy
#include <climits> // PATH_MAX
#include "data_point.h"
#include "file_io.h"
#include "util.h"

using namespace std;

static void usage(char *);

int main(int argc, char *argv[]) {
	// Parse command line arguments
	bool got_input_file = false;
	bool got_output_file = false;
	bool got_validate = false;

	char inputFile[PATH_MAX];
	char outputFile[PATH_MAX];

	if (argc > 6) {
		usage(argv[0]);
		return -1;
	}


	int i;
	for (i = 1; i < argc; i += 2) {
		if (strcmp(argv[i], "-d") == 0 && !got_input_file) {
			got_input_file = true;
			strncpy(inputFile, argv[i+1], PATH_MAX-1);
			inputFile[PATH_MAX-1] = '\0';

			if (!fileAccessible(inputFile)) {
				cerr << "[-] Input file: " << inputFile << " is not accessible" << endl;
				return -1;
			}
		} else if (strcmp(argv[i], "-o") == 0 && !got_output_file) {
			got_output_file = true;
			strncpy(outputFile, argv[i+1], PATH_MAX-1);
			outputFile[PATH_MAX-1] = '\0';
		} else if (strcmp(argv[i], "-validate") == 0 && !got_validate) {
			got_validate = true;
		} else {
			usage(argv[0]);
			return -1;
		}
	}


	if (!got_input_file || !got_output_file) {
		usage(argv[0]);
		return -1;
	}


	// Vector of input data points
	vector<DataPoint> dataPoints;

	// Read input file
	int neighbors = 20;
	int res;
	if ((res = readInputFile(inputFile, dataPoints, neighbors)) == 0) {
		cerr << "[-] Error while reading input file: " << inputFile << endl;
		return -1;
	} else if (res < 0) {
		cerr << "[-] Input file tweet IDs are not unique" << endl;
		return -1;
	}
	cout << "[+] Successfully read " << dataPoints.size() << " lines\n" << endl;


	// Remove previous contents of the output file
	if (emptyFile(outputFile) == false) {
		cerr << "[-] Couln't write to the output file: " << outputFile << endl;
		return -1;
	}


	//createResults();


	cout << "[!] Exiting the program..." << endl;
	return 0;
}



void usage(char *name) {
	cout << "Usage: " << name << " -d <input file> -o <output file> [-validate]" << endl;
}
