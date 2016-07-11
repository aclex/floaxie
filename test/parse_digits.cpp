#include <cstring>
#include <iostream>

#include "floaxie/crosh.h"

using namespace std;
using namespace floaxie;

bool check_integer()
{
	std::cout << "check_integer" << std::endl;

	bool ret = false;
	diy_fp::mantissa_storage_type value;
	const char* str_end;
	bool sign(false);
	int K(0);

	const char str1[] = "12345";
	value = parse_digits<decimal_q, true, false>(str1, &str_end, &sign, &K);
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;
	std::cout << "str_end: " << ptrdiff_t(str_end) << " (expected: " << ptrdiff_t(str1 + strlen(str1)) << ")" << std::endl;

	ret = str_end == (str1 + strlen(str1)) && (value == 12345) && sign && K == 0;

	const char str2[] = "-12345";
	value = parse_digits<decimal_q, true, false>(str2, &str_end, &sign, &K);
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;
	std::cout << "str_end: " << ptrdiff_t(str_end) << " (expected: " << ptrdiff_t(str2 + strlen(str2)) << ")" << std::endl;

	ret = ret && str_end == (str2 + strlen(str2)) && (value == 12345) && !sign && K == 0;

	return ret;
}

bool check_decimal_fraction()
{
	std::cout << "check_decimal_fraction" << std::endl;

	bool ret = false;
	diy_fp::mantissa_storage_type value;
	const char* str_end;
	bool sign(false);
	int K(0);

	const char str0[] = "0000123400.0000000000";
	const diy_fp::mantissa_storage_type chk0 = 1234;
	value = parse_digits<decimal_q, true, false>(str0, &str_end, &sign, &K);

	ret = str_end == (str0 + strlen(str0)) && (value == chk0) && sign && K == 2;
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;

	const char str1[] = "0000123400.05678900000";
	const diy_fp::mantissa_storage_type chk1 = 123400056789;
	value = parse_digits<decimal_q, true, false>(str1, &str_end, &sign, &K);

	ret = str_end == (str1 + strlen(str1)) && (value == chk1) && sign && K == -6;
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;

	const char str2[] = "-0.000123400000";
	const diy_fp::mantissa_storage_type chk2 = 1234;
	value = parse_digits<decimal_q, true, false>(str2, &str_end, &sign, &K);
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = ret && str_end == (str2 + strlen(str2)) && (value == chk2) && !sign && K == -7;

	const char str3[] = "123.00000";
	const diy_fp::mantissa_storage_type chk3 = 123;
	value = parse_digits<decimal_q, true, false>(str3, &str_end, &sign, &K);
	std::cout << "value: " << value << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = ret && str_end == (str3 + strlen(str3)) && (value == chk3) && sign && K == 0;

	return ret;
}

bool check_whole()
{
	std::cout << "check_whole" << std::endl;

	bool ret = false;
	const char* str_end;
	bool sign;
	int K;

	const char str1[] = "00001234.000567e000012";
	diy_fp chk_mant1(1234000567, 0);
	chk_mant1.normalize();
	const int chk_exp1 = 12;
	auto mant1 = parse_mantissa(str1, &str_end, &sign, &K);
	auto exp1 = parse_exponent(str_end, &str_end);

	ret = str_end == (str1 + strlen(str1)) && (mant1 == chk_mant1) && (exp1 == chk_exp1) && sign && (K + exp1 == 6);
	std::cout << "chk:  " << chk_mant1 << std::endl;
	std::cout << "mant: " << mant1 << std::endl;
	std::cout << "exp: " << exp1 << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K + exp1 << std::endl;

	const char str2[] = "-1234.000567e-000012";
	diy_fp chk_mant2(1234000567, 0);
	chk_mant2.normalize();
	const int chk_exp2 = -12;
	auto mant2 = parse_mantissa(str2, &str_end, &sign, &K);
	auto exp2 = parse_exponent(str_end, &str_end);

	ret = str_end == (str2 + strlen(str2)) && (mant2 == chk_mant2) && (exp2 == chk_exp2) && !sign && (K + exp2 == -18);
	std::cout << "chk:  " << chk_mant2 << std::endl;
	std::cout << "mant: " << mant2 << std::endl;
	std::cout << "exp: " << exp2 << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "K: " << K + exp2 << std::endl;

	return ret;
}

int main(int, char**)
{
	if (!check_integer()) return 2;
	if (!check_decimal_fraction()) return 3;
	if (!check_whole()) return 4;
	return 0;
}
