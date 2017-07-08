#include <array>
#include <utility>
#include <iostream>
#include <cmath>
#include <cstdint>

#include "floaxie/atof.h"
#include "floaxie/print.h"
#include "floaxie/type_punning_cast.h"

using namespace std;
using namespace floaxie;

namespace
{
	const char* test1 = "5e577";
}

int main(int, char**)
{
	char* str_end;
	double ret1 = atof<double>(test1, &str_end);
	if (ret1 != HUGE_VAL)
	{
		return -1;
	}

	return 0;
}
