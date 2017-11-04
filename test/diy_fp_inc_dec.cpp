#include <iostream>
#include <cstring>

#include "floaxie/diy_fp.h"

using namespace std;
using namespace floaxie;

namespace
{

}

int main(int, char**)
{
	typedef diy_fp<float> my_diy_fp;

	my_diy_fp tv1(1, 0);

	// normal cases
	++tv1;

	if (tv1.mantissa() != 2 || tv1.exponent() != 0)
		return -1;

	--tv1;

	if (tv1.mantissa() != 1 || tv1.exponent() != 0)
		return -2;

	// corner cases
	my_diy_fp tv2(numeric_limits<typename my_diy_fp::mantissa_storage_type>::max(), 2);

	++tv2;
	constexpr typename my_diy_fp::mantissa_storage_type ret2 = (numeric_limits<typename my_diy_fp::mantissa_storage_type>::max() >> 1) + 1;

	if (tv2.mantissa() != ret2 || tv2.exponent() != 3)
		return -3;

	my_diy_fp tv3(1, 2);

	--tv3;
	constexpr typename my_diy_fp::mantissa_storage_type ret3 = 1;

	if (tv3.mantissa() != ret3 || tv3.exponent() != 1)
		return -4;

	return 0;
}
