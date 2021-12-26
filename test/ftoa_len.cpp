#include <iostream>
#include <random>
#include <limits>
#include <cstdlib>

#include "floaxie/ftoa.h"

#include "short_numbers.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	double pi = 0.1;
	char buffer[128];

	random_device rd;
	default_random_engine gen(rd());
	uniform_real_distribution<> dis(0, 2);

	for (size_t i = 0; i < short_numbers_length; ++i)
	{
		pi = short_numbers[i];
		const auto len { ftoa(pi, buffer) };
		const auto expected { strlen(buffer) };

		if (len != expected)
		{
			cerr << "not equal" << "pi: " << pi << ", len: " << len << ", expected: " << expected << endl;
			return -1;
		}
	}

	return 0;
}
