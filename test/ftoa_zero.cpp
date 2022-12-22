#include <iostream>
#include <cstring>

#include "floaxie/ftoa.h"

using namespace std;
using namespace floaxie;

namespace
{
	constexpr double test_val(0);
}

int main(int, char**)
{
	char buf[max_buffer_size<decltype(test_val)>()];
	const auto sz = ftoa(test_val, buf);
	if (std::strcmp(buf, "0"))
	{
		return -1;
	}

	if (std::strlen(buf) != sz)
	{
		return -2;
	}

	return 0;
}
