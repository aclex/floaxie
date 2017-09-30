#include <cmath>
#include <bitset>
#include <iostream>
#include <limits>

#include "floaxie/diy_fp.h"
#include "floaxie/type_punning_cast.h"
#include "floaxie/print.h"

using namespace std;
using namespace floaxie;

bool check_double_conversion()
{
	const double pi(std::atan(1) * 4);
	cout << "original pi: " << print_binary(pi) << endl;

	diy_fp<double> w(pi);
	w.normalize();

	const auto& check_pi(w.downsample());
	cout << "check_pi: " << print_binary(check_pi.value) << endl;

	return check_pi.value == pi;
}

int main(int, char**)
{
	if (!check_double_conversion()) return 2;
	return 0;
}
