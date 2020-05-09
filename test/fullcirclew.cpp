#include <iostream>
#include <random>
#include <limits>
#include <cstdlib>

#include "floaxie/ftoa.h"
#include "floaxie/atof.h"

#include "short_numbers.h"

using namespace std;
using namespace floaxie;

int main(int, char**)
{
	wchar_t buffer[128];
	wchar_t* str_end;

	random_device rd;
	default_random_engine gen(rd());
	uniform_real_distribution<> dis(0, 2);

	size_t fault_number = 0;
	size_t fallback_count = 0;
	auto fallback_lambda = [&fallback_count](const wchar_t* str, wchar_t** str_end)
	{
		++fallback_count;
		return default_fallback<double>(str, str_end);
	};

	for (size_t i = 0; i < short_numbers_length; ++i)
	{
		double pi = short_numbers[i];
		ftoa(pi, buffer);
		double ret = atof<double>(buffer, &str_end, fallback_lambda);
		double ref_value = wcstod(buffer, nullptr);

		if (ref_value != pi || ref_value != ret)
		{
			cerr << "not equal [i = " << i << "] "
			<< "pi: " << pi
			<< ", buffer: "
			<< buffer << ", ref_value: "
			<< ref_value << ", ret: "
			<< ret << endl;
			++fault_number;
			return -1;
		}
	}
	cout << "krosh failed " << fault_number << " times out of " << short_numbers_length << endl;
	cout << "krosh triggered fallback conversion " << fallback_count << " times out of " << short_numbers_length << endl;

	return 0;
}
