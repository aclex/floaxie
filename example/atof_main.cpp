#include <iostream>

#include "floaxie/atof.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	char str[] = "0.1";
	char* str_end;
	double pi = 0;

	pi = atof<double>(str, &str_end);
	cout << "pi: " << pi << ", str: " << str << ", str_end: " << str_end << endl;

	return 0;
}
