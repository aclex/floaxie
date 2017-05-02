#include <cstring>
#include <iostream>

#include "floaxie/krosh.h"

using namespace std;
using namespace floaxie;

bool check_integer()
{
	std::cout << "check_integer" << std::endl;

	bool ret = false;

	digit_parse_result<double, char> dp;

	const char str1[] = "12345";
	dp = parse_digits<double>(str1);
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;
	std::cout << "str_end: " << ptrdiff_t(dp.str_end) << " (expected: " << ptrdiff_t(str1 + strlen(str1)) << ")" << std::endl;

	ret = dp.str_end == (str1 + strlen(str1)) && (dp.value == 12345) && dp.sign && dp.K == 0;

	const char str2[] = "-12345";
	dp = parse_digits<double>(str2);
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;
	std::cout << "str_end: " << ptrdiff_t(dp.str_end) << " (expected: " << ptrdiff_t(str2 + strlen(str2)) << ")" << std::endl;

	ret = ret && dp.str_end == (str2 + strlen(str2)) && (dp.value == 12345) && !dp.sign && dp.K == 0;

	return ret;
}

bool check_decimal_fraction()
{
	std::cout << "check_decimal_fraction" << std::endl;

	bool ret = false;

	digit_parse_result<double, char> dp;

	const char str0[] = "0000123400.0000000000";
	const diy_fp<double>::mantissa_storage_type chk0 = 1234;
	dp = parse_digits<double>(str0);

	ret = dp.str_end == (str0 + strlen(str0)) && (dp.value == chk0) && dp.sign && dp.K == 2;
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;

	const char str1[] = "0000123400.05678900000";
	const diy_fp<double>::mantissa_storage_type chk1 = 123400056789;
	dp = parse_digits<double>(str1);

	ret = dp.str_end == (str1 + strlen(str1)) && (dp.value == chk1) && dp.sign && dp.K == -6;
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;

	const char str2[] = "-0.000123400000";
	const diy_fp<double>::mantissa_storage_type chk2 = 1234;
	dp = parse_digits<double>(str2);
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;

	ret = ret && dp.str_end == (str2 + strlen(str2)) && (dp.value == chk2) && !dp.sign && dp.K == -7;

	const char str3[] = "123.00000";
	const diy_fp<double>::mantissa_storage_type chk3 = 123;
	dp = parse_digits<double>(str3);
	std::cout << "value: " << dp.value << std::endl;
	std::cout << "sign: " << dp.sign << std::endl;
	std::cout << "K: " << dp.K << std::endl;

	ret = ret && dp.str_end == (str3 + strlen(str3)) && (dp.value == chk3) && dp.sign && dp.K == 0;

	return ret;
}

bool check_whole()
{
	std::cout << "check_whole" << std::endl;

	bool ret = false;

	const char str1[] = "00001234.000567e000012";
	diy_fp<double> chk_mant1(1234000567, 0);
	chk_mant1.normalize();
	const int chk_exp1 = 12;
	auto mant1 = parse_mantissa<double>(str1);
	auto exp1 = parse_exponent(mant1.str_end);

	ret = exp1.str_end == (str1 + strlen(str1)) && (mant1.value == chk_mant1) && (exp1.value == chk_exp1) && mant1.sign && (mant1.K + exp1.value == 6);
	std::cout << "chk:  " << chk_mant1 << std::endl;
	std::cout << "mant: " << mant1.value << std::endl;
	std::cout << "exp: " << exp1.value << std::endl;
	std::cout << "sign: " << mant1.sign << std::endl;
	std::cout << "K: " << mant1.K + exp1.value << std::endl;

	const char str2[] = "-1234.000567e-000012";
	diy_fp<double> chk_mant2(1234000567, 0);
	chk_mant2.normalize();
	const int chk_exp2 = -12;
	auto mant2 = parse_mantissa<double>(str2);
	auto exp2 = parse_exponent(mant2.str_end);

	ret = exp2.str_end == (str2 + strlen(str2)) && (mant2.value == chk_mant2) && (exp2.value == chk_exp2) && !mant2.sign && (mant2.K + exp2.value == -18);
	std::cout << "chk:  " << chk_mant2 << std::endl;
	std::cout << "mant: " << mant2.value << std::endl;
	std::cout << "exp: " << exp2.value << std::endl;
	std::cout << "sign: " << mant2.sign << std::endl;
	std::cout << "K: " << mant2.K + exp2.value << std::endl;

	return ret;
}

int main(int, char**)
{
	if (!check_integer()) return 2;
	if (!check_decimal_fraction()) return 3;
	if (!check_whole()) return 4;
	return 0;
}
