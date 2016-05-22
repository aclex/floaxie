#include <cmath>
#include <bitset>
#include <iostream>
#include <limits>

#include "floaxie/diy_fp.h"
#include "floaxie/type_punning_cast.h"

using namespace std;
using namespace floaxie;

bool check_double_conversion()
{
	const double pi(std::atan(1) * 4);
	cout << "original pi: " << std::bitset<64>(type_punning_cast<std::uint64_t>(pi)) << endl;

	diy_fp w(pi);
	w.normalize<std::numeric_limits<double>::digits>();

	bool accurate;
	const double check_pi(w.downsample<double>(&accurate));
	cout << "check_pi: " << std::bitset<64>(type_punning_cast<std::uint64_t>(check_pi)) << endl;

	return check_pi == pi;
}

int main(int, char**)
{
	if (!check_double_conversion()) return 2;
	return 0;
}
