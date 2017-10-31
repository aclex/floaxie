#include <array>
#include <utility>
#include <iostream>
#include <cmath>
#include <cstdint>

#include "floaxie/atof.h"
#include "floaxie/print.h"

using namespace std;
using namespace floaxie;

namespace
{
	const char* inf_str = "inf";
	const char* inf_str_w = "0inf";
	const char* pinf_str = "+inf";
	const char* ninf_str = "-inf";
	const char* uinf_str = "INf";
	const char* puinf_str = "+InfINitY";
	const char* muinf_str = "-inFIniTy";
	const char* uinf_str_w = "infini";
}

int main(int, char**)
{
	char* str_end;
	const auto ret1 = atof<double>(inf_str, &str_end);
	if (!std::isinf(ret1.value) || ret1.status != conversion_status::success || str_end - inf_str != 3)
	{
		return -1;
	}

	const auto ret1w = atof<double>(inf_str_w, &str_end);
	if (std::isinf(ret1w.value) || ret1w.status != conversion_status::success || str_end - inf_str_w != 1)
	{
		return -10;
	}

	const auto ret2 = atof<double>(pinf_str, &str_end);
	if (!std::isinf(ret2.value) || ret2.value < 0 || ret2.status != conversion_status::success || str_end - pinf_str != 4)
	{
		return -2;
	}

	const auto ret3 = atof<double>(ninf_str, &str_end);
	if (!std::isinf(ret3.value) || ret3.value >= 0 || ret3.status != conversion_status::success || str_end - ninf_str != 4)
	{
		return -3;
	}

	const auto ret4 = atof<double>(uinf_str, &str_end);
	if (!std::isinf(ret4.value) || ret4.status != conversion_status::success || str_end - uinf_str != 3)
	{
		return -4;
	}

	const auto ret5 = atof<double>(puinf_str, &str_end);
	if (!std::isinf(ret5.value) || ret5.value < 0 || ret5.status != conversion_status::success || str_end - puinf_str != 9)
	{
		return -5;
	}

	const auto ret6 = atof<double>(muinf_str, &str_end);
	if (!std::isinf(ret6.value) || ret6.value >= 0 || ret6.status != conversion_status::success || str_end - muinf_str != 9)
	{
		return -6;
	}

	const auto ret7w = atof<double>(uinf_str_w, &str_end);
	if (!std::isinf(ret7w.value) || ret7w.status != conversion_status::success || str_end - uinf_str_w != 3)
	{
		return -11;
	}

	return 0;
}
