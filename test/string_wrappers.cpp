#include <iostream>

#include "floaxie/atof.h"
#include "floaxie/ftoa.h"

using namespace std;
using namespace floaxie;

namespace
{
	const wstring test_str(L"0.25");
	const float test_val = 0.5;
}

int main(int, char**)
{
	const auto from_string_result = from_string<double>(test_str);

	if (from_string_result != 0.25)
	{
		return -1;
	}

	const auto to_string_result = floaxie::to_string(test_val);

	cout << "result: \"" << to_string_result << "\"" << endl;

	if (to_string_result != string("0.5"))
	{
		return -2;
	}

	return 0;
}
