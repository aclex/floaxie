#include "floaxie/crosh.h"

#include <iostream>

using namespace floaxie;
using namespace std;

constexpr std::size_t round_to_power(3);

bool check_round_down()
{
	bool ret, accurate;

	constexpr unsigned char tail1(0x0b); // == 0b01011
	ret = round_up(tail1, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (ret || !accurate) // expected: ret == false, accurate == true
		return false;

	constexpr unsigned char tail2(0); // == 0b00000
	ret = round_up(tail2, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (ret || !accurate) // expected: ret == false, accurate == true
		return false;

	constexpr unsigned char tail3(0x04); // == 0b00100
	ret = round_up(tail3, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (ret || accurate) // expected: ret == false, accurate == false
		return false;

	constexpr unsigned char tail4(0x10); // == 0b10000
	ret = round_up(tail4, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (ret || !accurate) // expected: ret == false, accurate == true
		return false;

	constexpr unsigned char tail5(0x14); // == 0b10100
	ret = round_up(tail5, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (ret || accurate) // expected: ret == false, accurate == false
		return false;

	return true;
}

bool check_round_up_by_fraction_bits()
{
	bool ret, accurate;

	constexpr unsigned char tail1(0x05); // == 0b00101
	ret = round_up(tail1, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (!ret || !accurate) // expected: ret == true, accurate == true
		return false;

	constexpr unsigned char tail2(0x06); // == 0b00110
	ret = round_up(tail2, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (!ret || !accurate) // expected: ret == true, accurate == true
		return false;

	constexpr unsigned char tail3(0x07); // == 0b00111
	ret = round_up(tail3, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (!ret || !accurate) // expected: ret == true, accurate == true
		return false;

	return ret;
}

bool check_round_by_least_significant_bit()
{
	bool ret, accurate;

	constexpr unsigned char tail1(0x0c); // == 0b01100
	ret = round_up(tail1, round_to_power, &accurate);
	std::cout << "accurate:" << accurate << std::endl;
	if (!ret || accurate) // expected: ret == true, accurate == false
		return false;

	return ret;
}

int main(int, char**)
{
	if (!check_round_down()) return 2;
	if (!check_round_up_by_fraction_bits()) return 2;
	if (!check_round_by_least_significant_bit()) return 2;

	return 0;
}
