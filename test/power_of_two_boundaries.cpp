/* Issue and test examples for this case have been kindly
 * provided by Alexander Bolz.
 */

#include <cmath>
#include <bitset>
#include <iostream>
#include <limits>

#include "floaxie/ftoa.h"
#include "floaxie/print.h"

using namespace std;
using namespace floaxie;

bool check_single_conversion()
{
	constexpr array<float, 20> input
	{{
		9.8607613e-32f,
		6.3108872e-30f,
		8.4703295e-22f,
		8.6736174e-19f,
		7.1054274e-15f,
		7.2057594e+16f,
		3.7778932e+22f,
		7.5557864e+22f,
		4.8357033e+24f,
		7.7371252e+25f,
		1.5474251e+26f,
		3.0948501e+26f,
		6.1897002e+26f,
		1.2379401e+27f,
		2.4758801e+27f,
		4.9517602e+27f,
		9.9035203e+27f,
		1.9807041e+28f,
		3.9614081e+28f,
		7.9228163e+28f
	}};

	typedef decltype(input)::value_type value_type;

	char buf[max_buffer_size<value_type>()];

	for (value_type v : input)
	{
		cout << "binary form: " << print_binary(v) << endl;
		ftoa(v, buf);
		if (strtof(buf, nullptr) != v)
		{
			cerr << "value " << v << " failed to be converted round-trip." << endl;
			return false;
		}
	}

	return true;
}

bool check_double_conversion()
{
	constexpr array<double, 22> input
	{{
		1.7800590868057611e-307,
		2.0522684006491881e-289,
		3.9696644133184383e-264,
		2.9290953396399042e-244,
		2.5160737381238802e-234,
		5.5329046628180653e-222,
		4.5965573598916705e-187,
		2.8451311993408992e-160,
		5.0052077379577523e-147,
		4.9569176510071274e-119,
		4.6816763546921983e-97,
		5.0978941156238473e-57,
		3.2311742677852644e-27,
		3.8685626227668134e+25,
		4.9039857307708443e+55,
		2.6074060497081422e+92,
		4.8098152095208105e+111,
		4.7634102635436893e+139,
		4.4989137945431964e+161,
		4.8988833106573424e+201,
		8.139666055761541e+236,
		1.3207363278391631e+269
		}};

	typedef decltype(input)::value_type value_type;

	char buf[max_buffer_size<value_type>()];

	for (value_type v : input)
	{
		cout << "binary form: " << print_binary(v) << endl;
		ftoa(v, buf);
		if (strtod(buf, nullptr) != v)
		{
			cerr << "value " << v << " failed to be converted round-trip." << endl;
			return false;
		}
	}

	return true;
}

int main(int, char**)
{
	if (!check_single_conversion()) return 1;
	if (!check_double_conversion()) return 2;
	return 0;
}
