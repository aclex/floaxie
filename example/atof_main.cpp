#include <iostream>

#include "floaxie/atof.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	char str[] = "0.1";
// 	char str[] = "0.500000000000000166533453693773481063544750213623046875";
	char* str_end;
	double pi = 0;

	pi = atof<double>(str, &str_end);
	std::cout << "pi: " << pi << ", str: " << str << ", str_end: " << str_end << std::endl;

	return 0;
}
