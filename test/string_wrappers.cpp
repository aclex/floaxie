#include <iostream>

#include "floaxie/atof.h"
#include "floaxie/ftoa.h"

using namespace std;
using namespace floaxie;

namespace
{
	const wstring str(L"0.25");
	const float v = 0.5;
}

int main(int, char**)
{
	const auto from_string_result = from_string<double>(str);

	if (from_string_result != 0.25)
	{
		return -1;
	}

	const auto to_string_result = floaxie::to_string(v);

	cout << "result: \"" << to_string_result << "\"" << endl;

	if (to_string_result != string("0.5"))
	{
		return -2;
	}

	return 0;
}
