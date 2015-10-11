#include <iostream>

#include "floaxie/dtoa.h"

using namespace std;
using namespace floaxie;

int main(int argc, char* argv[])
{
	double pi = 0.1;
	char buffer[128];

	for (int i = 0; i < 128; ++i)
		buffer[i] = 0;

	dtoa(pi, buffer);
	std::cout << "pi: " << pi << ", buffer: " << buffer << std::endl;

	return 0;
}
