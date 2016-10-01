#include <iostream>

#include "floaxie/ftoa.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	double pi = 0.1;
	char buffer[128];

	ftoa(pi, buffer);
	cout << "pi: " << pi << ", buffer: " << buffer << endl;

	return 0;
}
