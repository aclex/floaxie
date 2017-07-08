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
	const char* test1 = "5e-577";
	const char* test2 = "0.000000000000000000000000000000001";
}

int main(int, char**)
{
	char* str_end;
	const auto ret1 = atof<float>(test1, &str_end);
	if (ret1.value != 0 || ret1.status != conversion_status::underflow)
	{
		return -1;
	}
	const auto ret2 = atof<float>(test2, &str_end);
	if (ret2.value != 0 || ret2.status != conversion_status::underflow)
	{
		return -2;
	}

	return 0;
}
