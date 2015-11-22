#include <cstring>
#include <iostream>

#include "floaxie/take_apart.h"

using namespace std;
using namespace floaxie;

bool check_integer()
{
	bool ret = false;
	char buffer[max_buffer_length];
	const char* str_end;
	bool sign;
	int len, K;

	const char str1[] = "12345";
	take_apart(str1, &str_end, buffer, &sign, &len, &K);
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = str_end == (str1 + strlen(str1)) && !strcmp(buffer, str1) && sign && len == strlen(str1) && K == 0;

	const char str2[] = "-12345";
	take_apart(str2, &str_end, buffer, &sign, &len, &K);
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = ret && str_end == (str2 + strlen(str2)) && !strcmp(buffer, str1) && !sign && len == strlen(str1) && K == 0;

	return ret;
}

bool check_decimal_fraction()
{
	bool ret = false;
	char buffer[max_buffer_length];
	const char* str_end;
	bool sign;
	int len, K;

	const char str1[] = "00001234.0005678900000";
	const char chk1[] = "123400056789";
	take_apart(str1, &str_end, buffer, &sign, &len, &K);

	ret = str_end == (str1 + strlen(str1)) && !strcmp(buffer, chk1) && sign && len == strlen(chk1) && K == -8;
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	const char str2[] = "-0.000123400000";
	const char chk2[] = "1234";
	take_apart(str2, &str_end, buffer, &sign, &len, &K);
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = ret && str_end == (str2 + strlen(str2)) && !strcmp(buffer, chk2) && !sign && len == strlen(chk2) && K == -7;

	const char str3[] = "123.00000";
	const char chk3[] = "123";
	take_apart(str3, &str_end, buffer, &sign, &len, &K);
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	ret = ret && str_end == (str3 + strlen(str3)) && !strcmp(buffer, chk3) && sign && len == strlen(chk3) && K == 0;

	return ret;
}

bool check_exponent()
{
	bool ret = false;
	char buffer[max_buffer_length];
	const char* str_end;
	bool sign;
	int len, K;

	const char str1[] = "00001234.000567e000012";
	const char chk1[] = "1234000567";
	take_apart(str1, &str_end, buffer, &sign, &len, &K);

	ret = str_end == (str1 + strlen(str1)) && !strcmp(buffer, chk1) && sign && len == strlen(chk1) && K == 6;
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	const char str2[] = "-1234.000567e-000012";
	const char chk2[] = "1234000567";
	take_apart(str2, &str_end, buffer, &sign, &len, &K);

	ret = ret && str_end == (str2 + strlen(str2)) && !strcmp(buffer, chk2) && !sign && len == strlen(chk2) && K == -18;
	std::cout << "buffer: " << buffer << std::endl;
	std::cout << "sign: " << sign << std::endl;
	std::cout << "len: " << len << std::endl;
	std::cout << "K: " << K << std::endl;

	return ret;
}

int main(int, char**)
{
	if (!check_integer()) return 2;
	if (!check_decimal_fraction()) return 2;
	if (!check_exponent()) return 2;
	return 0;
}
