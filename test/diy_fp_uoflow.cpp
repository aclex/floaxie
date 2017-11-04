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

	my_diy_fp tv1(numeric_limits<typename my_diy_fp::mantissa_storage_type>::max(), numeric_limits<float>::max_exponent + 1);

	const auto ret1 = tv1.downsample();

	if (ret1.status != conversion_status::overflow)
		return -1;

	diy_fp<float> tv2(numeric_limits<typename my_diy_fp::mantissa_storage_type>::max(), numeric_limits<float>::min_exponent - 256);

	const auto ret2 = tv2.downsample();

	if (ret2.value != 0 || ret2.status != conversion_status::underflow)
		return -2;

	return 0;
}
