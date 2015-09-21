#include <iostream>

#include "grisu.h"
#include "prettify.h"

using namespace std;

int main(int argc, char* argv[])
{
	double pi = 0.1;
	char buffer[128];

	for (int i = 0; i < 128; ++i)
		buffer[i] = 0;

	int len = 0, K = 0;
	grisu(pi, buffer, &len, &K);
	cout << "pi: " << pi << ", buffer: " << buffer << ", len: " << len << ", K: " << K << std::endl;

	prettify_string(buffer, 0, len, K);
	cout << "pi: " << pi << ", buffer: " << buffer << std::endl;

	return 0;
}
