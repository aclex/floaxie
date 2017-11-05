#include <iostream>
#include <cstring>

#include "floaxie/default_fallback.h"

using namespace std;
using namespace floaxie;

namespace
{
	constexpr char test_val[] = "0.5";
	constexpr wchar_t test_wval[] = L"0.5";
}

int main(int, char**)
{
	if (default_fallback<float, char>(test_val, nullptr) != std::strtof(test_val, nullptr))
		return -1;

	if (default_fallback<float, wchar_t>(test_wval, nullptr) != std::wcstof(test_wval, nullptr))
		return -2;

	if (default_fallback<double, char>(test_val, nullptr) != std::strtod(test_val, nullptr))
		return -3;

	if (default_fallback<double, wchar_t>(test_wval, nullptr) != std::wcstod(test_wval, nullptr))
		return -4;

	if (default_fallback<long double, char>(test_val, nullptr) != std::strtold(test_val, nullptr))
		return -5;

	if (default_fallback<long double, wchar_t>(test_wval, nullptr) != std::wcstold(test_wval, nullptr))
		return -6;

	// test check_errno
	const auto ch1 = check_errno(0.5f);
	if (ch1 != conversion_status::success)
		return -7;

	const char abnormal_val[] = "0.5e-588";
	const float test_abnormal_val = std::strtof(abnormal_val, nullptr);
	const auto ch2 = check_errno(test_abnormal_val);
	if (ch2 != conversion_status::underflow)
		return -8;

	return 0;
}
