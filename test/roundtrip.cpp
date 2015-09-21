#include <iostream>
#include <random>
#include <limits>
#include <cstdlib>

#include "grisu.h"
#include "prettify.h"
#include "short_numbers.h"

using namespace std;

int main(int argc, char* argv[])
{
	double pi = 0.1;
	char buffer[128];

	int len = 0, K = 0;

	random_device rd;
	default_random_engine gen(rd());
	uniform_real_distribution<> dis(0, 2);

	for (size_t i = 0; i < short_numbers_length; ++i)
	{
		pi = short_numbers[i];
		grisu(pi, buffer, &len, &K);

		prettify_string(buffer, 0, len, K);
		double test = strtod(buffer, nullptr);

		if (test != pi)
		{
			cerr << "not equal" << "pi: " << pi << ", buffer: " << buffer << endl;
			return -1;
		}
	}

	return 0;
}
