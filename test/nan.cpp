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
	const char* nan_str = "nan";
	const char* nan_str_w = "0nan";
	const char* pnan_str = "+nan";
	const char* nnan_str = "-nan";
	const char* unan_str = "NaN";
	const char* punan_str = "+Nan";
	const char* munan_str = "-naN";
	const char* seqnan_str = "naN(12356)";
	const char* seqnan_str_w = "naN(12356";
}

int main(int, char**)
{
	char* str_end;
	const auto ret1 = atof<double>(nan_str, &str_end);
	if (!std::isnan(ret1.value) || ret1.status != conversion_status::success || str_end - nan_str != 3)
	{
		return -1;
	}

	const auto ret1w = atof<double>(nan_str_w, &str_end);
	if (std::isnan(ret1w.value) || ret1w.status != conversion_status::success || str_end - nan_str_w != 1)
	{
		return -10;
	}

	const auto ret2 = atof<double>(pnan_str, &str_end);
	if (!std::isnan(ret2.value) || ret2.status != conversion_status::success || str_end - pnan_str != 4)
	{
		return -2;
	}

	const auto ret3 = atof<double>(nnan_str, &str_end);
	if (!std::isnan(ret3.value) || ret3.status != conversion_status::success || str_end - nnan_str != 4)
	{
		return -3;
	}

	const auto ret4 = atof<double>(unan_str, &str_end);
	if (!std::isnan(ret4.value) || ret4.status != conversion_status::success || str_end - unan_str != 3)
	{
		return -4;
	}

	const auto ret5 = atof<double>(punan_str, &str_end);
	if (!std::isnan(ret5.value) || ret5.status != conversion_status::success || str_end - punan_str != 4)
	{
		return -5;
	}

	const auto ret6 = atof<double>(munan_str, &str_end);
	if (!std::isnan(ret6.value) || ret6.status != conversion_status::success || str_end - munan_str != 4)
	{
		return -6;
	}

	const auto ret7 = atof<double>(seqnan_str, &str_end);
	if (!std::isnan(ret7.value) || ret7.status != conversion_status::success || str_end - seqnan_str != 10)
	{
		return -7;
	}

	const auto ret7w = atof<double>(seqnan_str_w, &str_end);
	if (!std::isnan(ret7w.value) || ret7w.status != conversion_status::success || str_end - seqnan_str_w != 3)
	{
		return -11;
	}

	return 0;
}
