#include "floaxie/crosh.h"

#include <iostream>

using namespace floaxie;
using namespace std;

constexpr std::size_t offset(2);

bool check_round_down()
{
	constexpr unsigned char tail1(0x0b); // == 0b01011
	bool ret = round_up(tail1, offset) == false;
	constexpr unsigned char tail2(0); // == 0b00000
	ret = ret && round_up(tail2, offset) == false;
	constexpr unsigned char tail3(0x04); // == 0b00100
	ret = ret && round_up(tail3, offset) == false;
	constexpr unsigned char tail4(0x10); // == 0b10000
	ret = ret && round_up(tail4, offset) == false;
	constexpr unsigned char tail5(0x14); // == 0b10100
	ret = ret && round_up(tail5, offset) == false;

	return ret;
}

bool check_round_up_by_fraction_bits()
{
	constexpr unsigned char tail1(0x05); // == 0b00101
	bool ret = round_up(tail1, offset) == true;
	constexpr unsigned char tail2(0x06); // == 0b00110
	ret = ret && round_up(tail2, offset) == true;
	constexpr unsigned char tail3(0x07); // == 0b00111
	ret = ret && round_up(tail3, offset) == true;

	return ret;
}

bool check_round_by_least_significant_bit()
{
	constexpr unsigned char tail1(0x0c); // == 0b01100
	bool ret = round_up(tail1, offset) == true;

	return ret;
}

int main(int, char**)
{
	if (!check_round_down()) return 2;
	if (!check_round_up_by_fraction_bits()) return 2;
	if (!check_round_by_least_significant_bit()) return 2;

	return 0;
}
