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
	const auto ret1 = atof<double>(test1, &str_end);
	if (ret1.value != HUGE_VAL || ret1.status != conversion_status::overflow)
	{
		return -1;
	}

	return 0;
}
